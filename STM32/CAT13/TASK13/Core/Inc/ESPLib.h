#ifndef ESPLIB_H
#define ESPLIB_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef enum
{
    StationMode = 1U
} ESP8266_Mode;

typedef enum
{
    QoS0 = 0U
} MQTT_QoS_t;

#define CleanSession 0x02U

typedef struct
{
    uint16_t keepAlive;
    uint8_t connectFlag;
    const char *clientID;
} MQTT_InitTypeDef_t;

void ESP8266_Init(ESP8266_Mode mode, UART_HandleTypeDef uart);
void ESP8266_wifiConnect(const char *ssid, const char *password);
void ESP8266_portConnect(const char *protocol, const char *host, const char *port);
void ESP8266_RxCallback(uint8_t receivedByte);
void MQTT_connectBroker(MQTT_InitTypeDef_t *setting);
void MQTT_publishTopic(const char *topic, const char *payload, MQTT_QoS_t qos);
void MQTT_subscribeTopic(const char *topic, uint8_t qos);
void MQTTDataHandler(char *topic, char *data);
void MQTTTimer(void);

#endif
