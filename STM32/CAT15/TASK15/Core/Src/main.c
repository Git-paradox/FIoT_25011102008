/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bme280.h"
#include "ESPLib.h"
#include "app_config.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
struct bme280_dev bme280;
struct bme280_data sensor_data;
struct bme280_settings settings;
MQTT_InitTypeDef_t mqttConfig;

char mqttMessage[100];
char mqttTopic[] = "college/weather/bme280";

uint8_t bme280_address;
int8_t rslt;

char msg[150];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static void BME280_Init(void);
static void BME280_ReadData(void);
void Terminal_Print(char *message);
static BME280_INTF_RET_TYPE BME280_I2C_Read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
);

static BME280_INTF_RET_TYPE BME280_I2C_Write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
);

static void BME280_Delay(uint32_t period, void *intf_ptr);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
		static BME280_INTF_RET_TYPE BME280_I2C_Read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;

    if (HAL_I2C_Mem_Read(
        &hi2c1,
        address << 1,
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        reg_data,
        (uint16_t)len,
        100) == HAL_OK)
    {
        return BME280_INTF_RET_SUCCESS;
    }

    return BME280_E_COMM_FAIL;
}


static BME280_INTF_RET_TYPE BME280_I2C_Write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;

    if (HAL_I2C_Mem_Write(
        &hi2c1,
        address << 1,
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *)reg_data,
        (uint16_t)len,
        100) == HAL_OK)
    {
        return BME280_INTF_RET_SUCCESS;
    }

    return BME280_E_COMM_FAIL;
}


static void BME280_Delay(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;

    HAL_Delay((period + 999U) / 1000U);
}

static void BME280_Init(void)
{
    memset(&bme280, 0, sizeof(bme280));

    /* Configure I2C interface */

    bme280.intf_ptr = &bme280_address;
    bme280.intf = BME280_I2C_INTF;

    bme280.read = BME280_I2C_Read;
    bme280.write = BME280_I2C_Write;
    bme280.delay_us = BME280_Delay;

    /* Initialize sensor */

    rslt = bme280_init(&bme280);

    if (rslt != BME280_OK)
    {
        Terminal_Print("BME280 initialization failed!\r\n");
        Error_Handler();
    }

    Terminal_Print("BME280 initialized successfully!\r\n");

    /* Configure sensor settings */

    settings.osr_t = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_1X;
    settings.osr_h = BME280_OVERSAMPLING_1X;

    settings.filter = BME280_FILTER_COEFF_OFF;
    settings.standby_time = BME280_STANDBY_TIME_1000_MS;

    rslt = bme280_set_sensor_settings(
        BME280_SEL_OSR_PRESS |
        BME280_SEL_OSR_TEMP |
        BME280_SEL_OSR_HUM |
        BME280_SEL_FILTER |
        BME280_SEL_STANDBY,
        &settings,
        &bme280
    );

    if (rslt != BME280_OK)
    {
        Terminal_Print("Sensor settings error!\r\n");
        Error_Handler();
    }

    /* Set normal operating mode */

    rslt = bme280_set_sensor_mode(
        BME280_POWERMODE_NORMAL,
        &bme280
    );

    if (rslt != BME280_OK)
    {
        Terminal_Print("Sensor mode error!\r\n");
        Error_Handler();
    }

    Terminal_Print("BME280 ready to measure!\r\n");
}

static void BME280_ReadData(void)
{
    rslt = bme280_get_sensor_data(
        BME280_ALL,
        &sensor_data,
        &bme280
    );

    if (rslt == BME280_OK)
    {
        sprintf(
            msg,
            "\r\n"
            "-----------------------------\r\n"
            "Temperature : %.2f C\r\n"
            "Pressure    : %.2f hPa\r\n"
            "Humidity    : %.2f %%RH\r\n"
            "-----------------------------\r\n",
            sensor_data.temperature,
            sensor_data.pressure / 100.0,
            sensor_data.humidity
        );

        Terminal_Print(msg);
    }
    else
    {
        Terminal_Print("Sensor reading failed!\r\n");
    }
}
void Terminal_Print(char *message)
{
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)message,
                      strlen(message),
                      HAL_MAX_DELAY);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

char msg[100];
uint8_t found = 0;

Terminal_Print("\r\n");
Terminal_Print("==============================\r\n");
Terminal_Print("   STM32 BME280 MONITOR\r\n");
Terminal_Print("==============================\r\n");

/* Detect BME280 address */

if (HAL_I2C_IsDeviceReady(
    &hi2c1,
    0x76 << 1,
    3,
    100) == HAL_OK)
{
    bme280_address = 0x76;
}
else if (HAL_I2C_IsDeviceReady(
    &hi2c1,
    0x77 << 1,
    3,
    100) == HAL_OK)
{
    bme280_address = 0x77;
}
else
{
    Terminal_Print("BME280 NOT DETECTED!\r\n");
    Error_Handler();
}

/* Initialize sensor */

BME280_Init();

HAL_Delay(1100);
/* Initialize ESP8266 using USART1 */

funcState_t espStatus;

espStatus = ESP8266_Init(StationMode, huart1);

if (espStatus == funcOk)
{
    Terminal_Print("ESP8266 initialized successfully!\r\n");
}
else
{
    Terminal_Print("ESP8266 initialization failed!\r\n");
}
/* Connect ESP8266 to Wi-Fi */

if (ESP8266_wifiConnect(WIFI_SSID, WIFI_PASSWORD) == funcOk)
{
    Terminal_Print("Wi-Fi connected successfully!\r\n");
}
else
{
    Terminal_Print("Wi-Fi connection failed!\r\n");
}
/* Connect to MQTT broker */

if (ESP8266_portConnect("TCP", "broker.emqx.io", "1883") == funcOk)
{
    Terminal_Print("MQTT broker TCP connection successful!\r\n");
}
else
{
    Terminal_Print("MQTT broker connection failed!\r\n");
}

/* Configure MQTT client */

MQTT_InitTypeDef_t mqttConfig;

mqttConfig.clientID = "STM32_BME280_Device01";
mqttConfig.username = NULL;
mqttConfig.password = NULL;
mqttConfig.keepAlive = 60;
mqttConfig.connectFlag = CleanSession;

/* Send MQTT CONNECT packet */

MQTT_connectBroker(&mqttConfig);

HAL_Delay(1000);

Terminal_Print("MQTT CONNECT packet sent!\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 while (1)
{
    BME280_ReadData();

    sprintf(mqttMessage,
            "{\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.2f}",
            sensor_data.temperature,
            sensor_data.pressure / 100.0,
            sensor_data.humidity);

    MQTT_publishTopic(mqttTopic, mqttMessage, QoS0);

    Terminal_Print("MQTT publish function called!\r\n");

    HAL_Delay(5000);
}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
