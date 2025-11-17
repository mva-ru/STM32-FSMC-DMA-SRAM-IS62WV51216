/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "EventRecorder.h"
#include "hal_is62wv51216.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
u32 _eventRecordAll;

//******************* Примеры использования IS62WV51216 *****************************
// ссылку на объект управления драйвером микросхемы SRAM - IS62WV51216
// на случай, если надо отслеживать счетчики ошибок или доп. инфу.
IS62WV51216_s_obj* p_sram; 
//------------------------------------------------------------------------------------
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
  EventRecorderInitialize(_eventRecordAll, 1);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_SPI1_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  //******************* Примеры использования IS62WV51216 *****************************
  IS62WV51216_Init(IS62WV51216_NUM_DRV_1, &hsram1, IS62WV51216_DMA_ON);
  p_sram = IS62WV51216_Get_Obj_Link();  
  HAL_TIM_Base_Start_IT(&htim5);
  //------------------------------------------------------------------------------------
  /* USER CODE END 2 */
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    //******************* Примеры использования IS62WV51216 *****************************
//    cu8 size = 100;
//    u16 bf_tx[size];
//    u16 bf_rx[size];
    //------------------------------------------------------------------------------------
//    for(u16 i = 0, j = 0; i < size; i++, j += 500)
//      bf_tx[i] = j;
//    
//    IS62WV51216_Rw_Data(0, 0, (u16*)&bf_tx, size, IS62WV51216_WRITE, IS62WV51216_SET_DMA_OFF);
//    IS62WV51216_Rw_Data(0, 0, (u16*)&bf_rx, size, IS62WV51216_READ, IS62WV51216_SET_DMA_OFF);   
    //------------------------------------------------------------------------------------
//    for(u16 i = 0, j = 0; i < size; i++, j += 101)
//      bf_tx[i] = j;
//    
//    IS62WV51216_Rw_Data(0, 0, (u16*)&bf_tx, size, IS62WV51216_WRITE, IS62WV51216_DMA_ON);
//    IS62WV51216_Rw_Data(0, 0, (u16*)&bf_rx, size, IS62WV51216_READ, IS62WV51216_DMA_ON);
    //------------------------------------------------------------------------------------
//    u8 hi_byte = 0, lo_byte = 0;
//    // чтение страшего и младшего байта
//    IS62WV51216_Rw_Byte(0, 99, &hi_byte, IS62WV51216_READ, IS62WV51216_HI_BYTE);  // 0xC1   
//    IS62WV51216_Rw_Byte(0, 99, &lo_byte, IS62WV51216_READ, IS62WV51216_LO_BYTE);  // 0x5C 
//    // запись страшего байта
//    hi_byte = 0x66, lo_byte = 0xDD;
//    IS62WV51216_Rw_Byte(0, 99, &hi_byte, IS62WV51216_WRITE, IS62WV51216_HI_BYTE);  
//    hi_byte = 0;
//    IS62WV51216_Rw_Byte(0, 99, &hi_byte, IS62WV51216_READ, IS62WV51216_HI_BYTE);  // 0x66 
//    // запись младшего байта
//    IS62WV51216_Rw_Byte(0, 99, &lo_byte, IS62WV51216_WRITE, IS62WV51216_LO_BYTE);  
//    lo_byte = 0;
//    IS62WV51216_Rw_Byte(0, 99, &lo_byte, IS62WV51216_READ, IS62WV51216_LO_BYTE);  // 0xDD 
    //------------------------------------------------------------------------------------    
    IS62WV51216_Handler();
    //------------------------------------------------------------------------------------  
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
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

#ifdef  USE_FULL_ASSERT
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
