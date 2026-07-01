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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "smartwatch_ui.h"
#include "mpu6050.h"
#include "bluetooth.h"
#include "pedometer.h"
#include "soft_rtc.h"
#include "i2c_drv.h"
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

/* USER CODE BEGIN PV */
SmartWatchData_t watch_data;
PedometerState pedo_state;
UIPage_t current_page;
uint32_t last_ui_tick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();

  /* 先做 I2C 总线恢复，防止上电死锁 */
  I2C1_RecoverBus();
  I2C2_RecoverBus();

  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  UI_InitData(&watch_data);
  Pedometer_Init(&pedo_state);
  current_page = PAGE_WATCH_FACE;

  /* Initialize OLED (I2C1) */
  OLED_Init();

  /* Initialize MPU6050 (I2C2) */
  watch_data.imu_status = (MPU6050_Init() == 0) ? 1 : 0;

  /* Initialize Bluetooth (USART2 DMA) */
  BT_Init();
  watch_data.bt_connected = 0;

  /* Start TIM2 for 1Hz RTC tick */
  HAL_TIM_Base_Start_IT(&htim2);

  /* Show initial page */
  UI_DrawPage(current_page, &watch_data);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t now = HAL_GetTick();

    /* 1) 从软 RTC 同步时间 */
    watch_data.hour = rtc_time.hour;
    watch_data.minute = rtc_time.min;
    watch_data.second = rtc_time.sec;

    /* 2) MPU6050 传感器读取 + 计步（每 200ms 一次） */
    {
        static uint32_t last_mpu_tick = 0;
        static uint32_t last_retry_tick = 0;
        static uint32_t last_bt_tick = 0;

        if (watch_data.imu_status) {
            if (now - last_mpu_tick >= 200) {
                int acc_ok = MPU6050_ReadAccel(&watch_data.accel);
                int gyr_ok = MPU6050_ReadGyro(&watch_data.gyro);
                float temp = MPU6050_ReadTemp();

                if (acc_ok != 0 || gyr_ok != 0 || temp < -100.0f) {
                    watch_data.imu_status = 0;
                } else {
                    MPU6050_CalcAngle(&watch_data.accel, &watch_data.angle);
                    watch_data.temp_celsius = (int8_t)temp;

                    /* 更新计步器（需要原始 int16_t 数据） */
                    int16_t raw_acc[3];
                    if (MPU6050_ReadRawAccel(raw_acc) == 0) {
                        Pedometer_Update(&pedo_state, raw_acc, now);
                        watch_data.step_count = Pedometer_GetStepCount(&pedo_state);
                        watch_data.distance_m = Pedometer_GetDistanceMeters(&pedo_state);
                        watch_data.calories = Pedometer_GetCalories(&pedo_state);
                    }
                }

                /* Send sensor data via Bluetooth every 1s */
                if (now - last_bt_tick >= 1000) {
                    BT_SendSensorData(&watch_data);
                    last_bt_tick = now;
                }

                last_mpu_tick = now;
            }
        } else {
            if (now - last_retry_tick >= 3000) {
                if (MPU6050_Init() == 0) {
                    watch_data.imu_status = 1;
                }
                last_retry_tick = now;
            }
        }
    }

    /* 3) 处理蓝牙命令（时间同步等） */
    if (BT_Process(&watch_data)) {
        /* 如果收到时间同步，把时间写回软 RTC */
        rtc_time.hour = watch_data.hour;
        rtc_time.min = watch_data.minute;
        rtc_time.sec = watch_data.second;
    }

    /* 4) 读取 HC-05 STATE 引脚 */
    watch_data.bt_connected = BT_IsConnected();

    /* 5) 页面自动切换（每 3 秒换一页，或者按键触发） */
    if (now - last_ui_tick >= 3000) {
        current_page = (UIPage_t)((current_page + 1) % PAGE_MAX);
        last_ui_tick = now;
    }

    /* 6) UI 刷新（每 500ms） */
    {
        static uint32_t last_refresh = 0;
        if (now - last_refresh >= 500) {
            UI_DrawPage(current_page, &watch_data);
            last_refresh = now;
        }
    }
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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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