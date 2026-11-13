#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Replace these placeholders before using Wi-Fi and TCP transmission. */
#define WIFI_SSID               "YOUR_WIFI_SSID"
#define WIFI_PASSWORD           "YOUR_WIFI_PASSWORD"
#define TCP_SERVER_IP           "YOUR_SERVER_IP"
#define TCP_SERVER_PORT         5000U

#define WEMOS_UART_BAUDRATE     115200U
#define PC_UART_BAUDRATE        9600U
#define WIFI_CONNECT_TIMEOUT   30000U
#define TCP_CONNECT_TIMEOUT    15000U
#define SENSOR_REPORT_INTERVAL 5000U

#endif
