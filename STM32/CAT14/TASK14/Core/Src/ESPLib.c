#include "ESPLib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ESP_RX_BUFFER_SIZE 512U
#define MQTT_PACKET_SIZE   256U

static UART_HandleTypeDef espUart;
static volatile uint8_t rxBuffer[ESP_RX_BUFFER_SIZE];
static volatile uint16_t rxHead = 0U;
static volatile uint16_t rxTail = 0U;
static uint16_t mqttKeepAlive = 60U;
static volatile uint32_t mqttMilliseconds = 0U;
static uint32_t lastPingTime = 0U;
static uint16_t packetIdentifier = 1U;

static uint8_t ESP_ReadByte(uint8_t *value)
{
    if (rxTail == rxHead)
        return 0U;

    *value = rxBuffer[rxTail];
    rxTail = (uint16_t)((rxTail + 1U) % ESP_RX_BUFFER_SIZE);
    return 1U;
}

void ESP8266_RxCallback(uint8_t receivedByte)
{
    uint16_t next = (uint16_t)((rxHead + 1U) % ESP_RX_BUFFER_SIZE);

    if (next != rxTail)
    {
        rxBuffer[rxHead] = receivedByte;
        rxHead = next;
    }
}

static uint8_t ESP_WaitForText(const char *expected, uint32_t timeout)
{
    char response[256];
    uint16_t length = 0U;
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    response[0] = '\0';

    while ((HAL_GetTick() - start) < timeout)
    {
        if (ESP_ReadByte(&ch) != 0U)
        {
            if (length < sizeof(response) - 1U)
            {
                response[length++] = (char)ch;
                response[length] = '\0';
            }

            if (strstr(response, expected) != NULL)
                return 1U;

            if (strstr(response, "ERROR") != NULL ||
                strstr(response, "FAIL") != NULL)
                return 0U;
        }
        else
        {
            HAL_Delay(1U);
        }
    }

    return 0U;
}

static void ESP_SendCommand(const char *command, const char *expected,
                            uint32_t timeout)
{
    HAL_UART_Transmit(&espUart, (uint8_t *)command,
                      (uint16_t)strlen(command), 2000U);
    HAL_UART_Transmit(&espUart, (uint8_t *)"\r\n", 2U, 1000U);

    if (expected != NULL)
        (void)ESP_WaitForText(expected, timeout);
}

static uint8_t ESP_SendMqttPacket(const uint8_t *packet, uint16_t length)
{
    char command[40];

    (void)snprintf(command, sizeof(command), "AT+CIPSEND=%u",
                   (unsigned int)length);

    HAL_UART_Transmit(&espUart, (uint8_t *)command,
                      (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&espUart, (uint8_t *)"\r\n", 2U, 1000U);

    if (ESP_WaitForText(">", 5000U) == 0U)
        return 0U;

    HAL_UART_Transmit(&espUart, (uint8_t *)packet, length, 3000U);
    return ESP_WaitForText("SEND OK", 6000U);
}

static uint8_t MQTT_WriteRemainingLength(uint8_t *buffer, uint32_t value)
{
    uint8_t count = 0U;

    do
    {
        uint8_t encoded = (uint8_t)(value % 128U);
        value /= 128U;

        if (value > 0U)
            encoded |= 0x80U;

        buffer[count++] = encoded;
    }
    while (value > 0U && count < 4U);

    return count;
}

void ESP8266_Init(ESP8266_Mode mode, UART_HandleTypeDef uart)
{
    espUart = uart;
    ESP_SendCommand("AT", "OK", 3000U);

    if (mode == StationMode)
        ESP_SendCommand("AT+CWMODE=1", "OK", 3000U);
}

void ESP8266_wifiConnect(const char *ssid, const char *password)
{
    char command[180];

    (void)snprintf(command, sizeof(command),
                   "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    ESP_SendCommand(command, "WIFI GOT IP", 30000U);
}

void ESP8266_portConnect(const char *protocol, const char *host, const char *port)
{
    char command[160];

    ESP_SendCommand("AT+CIPMUX=0", "OK", 3000U);
    (void)snprintf(command, sizeof(command),
                   "AT+CIPSTART=\"%s\",\"%s\",%s",
                   protocol, host, port);
    ESP_SendCommand(command, "CONNECT", 15000U);
}

void MQTT_connectBroker(MQTT_InitTypeDef_t *setting)
{
    uint8_t packet[MQTT_PACKET_SIZE];
    uint16_t index = 0U;
    uint16_t clientLength;
    uint32_t remainingLength;
    uint8_t encodedLength[4];
    uint8_t encodedCount;
    const char *clientID;

    if (setting == NULL)
        return;

    clientID = (setting->clientID != NULL) ? setting->clientID : "STM32Client";
    clientLength = (uint16_t)strlen(clientID);
    mqttKeepAlive = (setting->keepAlive != 0U) ? setting->keepAlive : 60U;

    remainingLength = 10U + 2U + clientLength;
    encodedCount = MQTT_WriteRemainingLength(encodedLength, remainingLength);

    packet[index++] = 0x10U;
    memcpy(&packet[index], encodedLength, encodedCount);
    index = (uint16_t)(index + encodedCount);

    packet[index++] = 0x00U;
    packet[index++] = 0x04U;
    packet[index++] = 'M';
    packet[index++] = 'Q';
    packet[index++] = 'T';
    packet[index++] = 'T';
    packet[index++] = 0x04U;
    packet[index++] = (setting->connectFlag != 0U) ?
                      setting->connectFlag : CleanSession;
    packet[index++] = (uint8_t)(mqttKeepAlive >> 8);
    packet[index++] = (uint8_t)(mqttKeepAlive & 0xFFU);

    packet[index++] = (uint8_t)(clientLength >> 8);
    packet[index++] = (uint8_t)(clientLength & 0xFFU);
    memcpy(&packet[index], clientID, clientLength);
    index = (uint16_t)(index + clientLength);

    if (ESP_SendMqttPacket(packet, index) != 0U)
        lastPingTime = mqttMilliseconds;
}

void MQTT_publishTopic(const char *topic, const char *payload, MQTT_QoS_t qos)
{
    uint8_t packet[MQTT_PACKET_SIZE];
    uint16_t topicLength;
    uint16_t payloadLength;
    uint16_t index = 0U;
    uint32_t remainingLength;
    uint8_t encodedLength[4];
    uint8_t encodedCount;

    if (topic == NULL || payload == NULL || qos != QoS0)
        return;

    topicLength = (uint16_t)strlen(topic);
    payloadLength = (uint16_t)strlen(payload);
    remainingLength = 2U + topicLength + payloadLength;
    if (remainingLength + 5U > sizeof(packet))
        return;

    encodedCount = MQTT_WriteRemainingLength(encodedLength, remainingLength);
    packet[index++] = 0x30U;
    memcpy(&packet[index], encodedLength, encodedCount);
    index = (uint16_t)(index + encodedCount);

    packet[index++] = (uint8_t)(topicLength >> 8);
    packet[index++] = (uint8_t)(topicLength & 0xFFU);
    memcpy(&packet[index], topic, topicLength);
    index = (uint16_t)(index + topicLength);
    memcpy(&packet[index], payload, payloadLength);
    index = (uint16_t)(index + payloadLength);

    (void)ESP_SendMqttPacket(packet, index);
}

void MQTT_subscribeTopic(const char *topic, uint8_t qos)
{
    uint8_t packet[MQTT_PACKET_SIZE];
    uint16_t topicLength;
    uint16_t index = 0U;
    uint32_t remainingLength;
    uint8_t encodedLength[4];
    uint8_t encodedCount;

    if (topic == NULL)
        return;

    topicLength = (uint16_t)strlen(topic);
    remainingLength = 2U + 2U + topicLength + 1U;
    encodedCount = MQTT_WriteRemainingLength(encodedLength, remainingLength);

    packet[index++] = 0x82U;
    memcpy(&packet[index], encodedLength, encodedCount);
    index = (uint16_t)(index + encodedCount);

    packet[index++] = (uint8_t)(packetIdentifier >> 8);
    packet[index++] = (uint8_t)(packetIdentifier & 0xFFU);
    packetIdentifier++;

    packet[index++] = (uint8_t)(topicLength >> 8);
    packet[index++] = (uint8_t)(topicLength & 0xFFU);
    memcpy(&packet[index], topic, topicLength);
    index = (uint16_t)(index + topicLength);
    packet[index++] = (qos > 0U) ? 1U : 0U;

    (void)ESP_SendMqttPacket(packet, index);
}

static void MQTT_ParsePublish(const uint8_t *packet, uint16_t length,
                              char *topic, char *data)
{
    uint32_t remaining = 0U;
    uint8_t multiplier = 1U;
    uint16_t position = 1U;
    uint8_t encoded;

    if (length < 4U || (packet[0] >> 4) != 3U)
        return;

    do
    {
        if (position >= length)
            return;
        encoded = packet[position++];
        remaining += (uint32_t)(encoded & 0x7FU) * multiplier;
        multiplier *= 128U;
    }
    while ((encoded & 0x80U) != 0U && multiplier <= 128U * 128U * 128U);

    if (position + 2U > length)
        return;

    {
        uint16_t topicLength =
            (uint16_t)(((uint16_t)packet[position] << 8) | packet[position + 1U]);
        uint16_t payloadStart;

        position = (uint16_t)(position + 2U);
        if (position + topicLength > length)
            return;

        if (topicLength > 63U)
            topicLength = 63U;

        memcpy(topic, &packet[position], topicLength);
        topic[topicLength] = '\0';
        payloadStart = (uint16_t)(position + topicLength);

        if ((packet[0] & 0x06U) != 0U)
            payloadStart = (uint16_t)(payloadStart + 2U);

        if (payloadStart > length)
            return;

        {
            uint16_t payloadLength = (uint16_t)(length - payloadStart);
            if (payloadLength > 127U)
                payloadLength = 127U;
            memcpy(data, &packet[payloadStart], payloadLength);
            data[payloadLength] = '\0';
        }
    }

    (void)remaining;
}

void MQTTDataHandler(char *topic, char *data)
{
    static uint8_t state = 0U;
    static char ipdHeader[24];
    static uint8_t headerLength = 0U;
    static uint16_t packetLength = 0U;
    static uint16_t packetIndex = 0U;
    static uint8_t packet[MQTT_PACKET_SIZE];
    uint8_t ch;

    if (topic == NULL || data == NULL)
        return;

    while (ESP_ReadByte(&ch) != 0U)
    {
        if (state == 0U)
        {
            if (ch == '+')
            {
                ipdHeader[0] = '+';
                headerLength = 1U;
                state = 1U;
            }
        }
        else if (state == 1U)
        {
            if (headerLength < sizeof(ipdHeader) - 1U)
                ipdHeader[headerLength++] = (char)ch;

            if (ch == ':')
            {
                ipdHeader[headerLength - 1U] = '\0';
                if (strncmp(ipdHeader, "+IPD,", 5U) == 0)
                {
                    char *lastComma = strrchr(ipdHeader, ',');
                    packetLength = (uint16_t)atoi(lastComma != NULL ?
                                                   lastComma + 1 : ipdHeader + 5);
                    if (packetLength > sizeof(packet))
                        packetLength = sizeof(packet);
                    packetIndex = 0U;
                    state = (packetLength > 0U) ? 2U : 0U;
                }
                else
                    state = 0U;
            }
            else if (headerLength >= sizeof(ipdHeader) - 1U)
                state = 0U;
        }
        else if (state == 2U)
        {
            packet[packetIndex++] = ch;
            if (packetIndex >= packetLength)
            {
                MQTT_ParsePublish(packet, packetLength, topic, data);
                state = 0U;
            }
        }
    }

    if ((mqttMilliseconds - lastPingTime) >= ((uint32_t)mqttKeepAlive * 1000U))
    {
        static const uint8_t pingRequest[2] = {0xC0U, 0x00U};
        (void)ESP_SendMqttPacket(pingRequest, sizeof(pingRequest));
        lastPingTime = mqttMilliseconds;
    }
}

void MQTTTimer(void)
{
    mqttMilliseconds++;
}
