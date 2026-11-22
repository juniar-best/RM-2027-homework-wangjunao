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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "OLED.h"
#include "Servo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SEQ_LEN    5
#define STEP_MS    1000
#define KEY_STEP   15
#define DEBOUNCE   20
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static const uint8_t  seq[SEQ_LEN] = {0, 45, 90, 135, 180};

static uint16_t curAngle = 0;
static uint8_t  idx      = 0;
static uint8_t  autoRun  = 1;
static uint32_t lastTick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static uint8_t  Key_Scan(void);
static void    ShowStatus(void);
static void    SetAngle(uint16_t angle);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int fd, char *ptr, int len)
{
  uint32_t tickstart;

  if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)ptr, (uint16_t)len) != HAL_OK)
  {
    return 0;
  }

  tickstart = HAL_GetTick();
  while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX)
  {
    if ((HAL_GetTick() - tickstart) > 100) break;
  }

  return len;
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
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  setvbuf(stdout, NULL, _IONBF, 0);

  OLED_Init();
  Servo_Init();
  SetAngle(seq[0]);
  lastTick = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint8_t key = Key_Scan();

    if (key == 1)
    {
      autoRun = 0;
      curAngle += KEY_STEP;
      if (curAngle > 180) curAngle = 180;
      SetAngle(curAngle);
    }
    else if (key == 2)
    {
      autoRun = 0;
      if (curAngle >= KEY_STEP) curAngle -= KEY_STEP;
      else curAngle = 0;
      SetAngle(curAngle);
    }

    if (autoRun && (HAL_GetTick() - lastTick >= STEP_MS))
    {
      lastTick = HAL_GetTick();
      idx      = (uint8_t)((idx + 1) % SEQ_LEN);
      SetAngle(seq[idx]);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

/* USER CODE BEGIN 4 */

static uint8_t Key_Scan(void)
{
  static uint8_t released = 1;

  if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_SET &&
      HAL_GPIO_ReadPin(KEY_DN_GPIO_Port, KEY_DN_Pin) == GPIO_PIN_SET)
  {
    released = 1;
  }

  if (released == 0) return 0;

  if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET)
  {
    HAL_Delay(DEBOUNCE);
    if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET)
    {
      released = 0;
      return 1;
    }
  }

  if (HAL_GPIO_ReadPin(KEY_DN_GPIO_Port, KEY_DN_Pin) == GPIO_PIN_RESET)
  {
    HAL_Delay(DEBOUNCE);
    if (HAL_GPIO_ReadPin(KEY_DN_GPIO_Port, KEY_DN_Pin) == GPIO_PIN_RESET)
    {
      released = 0;
      return 2;
    }
  }

  return 0;
}

/* 刷新 OLED 三行并打印到串口 */
static void ShowStatus(void)
{
  uint16_t ccr = Servo_GetCCR();

  OLED_ShowString(1, 1,  "Target:");
  OLED_ShowNum   (1, 9,  curAngle, 3);
  OLED_ShowString(1, 12, "deg");

  OLED_ShowString(2, 1,  "CCR   :");
  OLED_ShowNum   (2, 9,  ccr, 4);

  OLED_ShowString(3, 1,  "Pulse :");
  OLED_ShowNum   (3, 9,  ccr / 1000, 1);
  OLED_ShowString(3, 10, ".");
  OLED_ShowNum   (3, 11, ccr % 1000, 3);
  OLED_ShowString(3, 14, "ms");

  printf("Target=%3d  CCR=%4d  Pulse=%d.%03dms\r\n",
         curAngle, ccr, ccr / 1000, ccr % 1000);
}

static void SetAngle(uint16_t angle)
{
  curAngle = angle;
  Servo_SetAngle((float)angle);
  ShowStatus();
}
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
