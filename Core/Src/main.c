/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "quadspi.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "flash.h"
#include "lsm303c.h"
#include <math.h>
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
int16_t rawData[3];
float axisg[3];
float W;
int zdarzenia[6] = { 0, 0, 0, 0, 0, 0 };
uint8_t bp;  //bit przychodzący
int k = 0;
int newData,Start;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart2, (uint8_t*) ptr, len, 50);
	return len;
}

void memoryInfo() {
	uint16_t dataCount = getDataCount();
	printf("---------- \r\n");
	printf("Data stored: %d\r\n", dataCount);
	if (dataCount) {
		printf("Memory left: %d\r\n", memLeft());
		printf("Best data: \r\n");
		infoStruct(&bestStruct);
	}
	printf("---------- \r\n\n");
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	switch (atoi(&bp)) {
	case 1:
		printf("\033[2J");

		//clearMemory();
		printf("Wyczyszczono pamiec \r\n");
		printf("x: %3f  y: %3f z: %3f  Wektor: %3f  \r\n", axisg[0], axisg[1],
					axisg[2], W);
			printf(
					"Liczba zdarzen stopnia: 1: %3d 2: %3d 3:  %3d 4: %3d 5: %3d 6: %3d",
					zdarzenia[0], zdarzenia[1], zdarzenia[2], zdarzenia[3],
					zdarzenia[4], zdarzenia[5]);
		memoryInfo();
		menu();
		actualTime();
		break;

	case 2:
		printf("\033[2J");
		printf("Najwyzszy pomiar to: \r\n");
		infoStruct(&bestStruct);
		menu();
		break;

	case 3:
		printf("\033[2J");
		if(Start==0){Start=1;printf("Wznowienie pomiarow  \r\n");}
		else{Start=0;printf("Zatrzymanie pomiarow\r\n");}
		menu();
		break;

	case 4:
		printf("\033[2J");
		getLastStruct();
		menu();
		break;

	case 5:
		printf("\033[2J");
		printf("Wyswietlanie zapamietanych pomiarow: \r\n");
		listAllData();
		menu();
		break;

	}
	HAL_UART_Receive_IT(&huart2, &bp, 1);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim6) {
		newData=1;

	}
}
void acceler()
{
	LSM303C_AccReadXYZ(rawData);
			axisg[0] = (float) rawData[0] / 1600;
			axisg[0] = axisg[0] - 1.3;
			axisg[1] = (float) rawData[1] / 1600;
			axisg[1] = axisg[1] ;
			axisg[2] = (float) rawData[2] / 1600;
			axisg[2] = axisg[2] - 9.8;
			W = sqrtf(pow(axisg[0], 2) + pow(axisg[1], 2) + pow(axisg[2], 2));
			nextMeasurement(421);
			if (W > 0.25 && W < 0.11) {
				zdarzenia[0] += 1;
			}
			if (W > 0.35 && W < 0.44) {
				zdarzenia[1] += 1;
			}
			if (W > 0.44 && W < 1.66) {
				zdarzenia[2] += 1;
			}
			if (W > 1.66 && W < 8.2) {
				zdarzenia[3] += 1;
			}
			if (W > 8.2 && W < 32.8) {
				zdarzenia[4] += 1;
			}
			if (W > 32.8 && W < 328) {
				zdarzenia[5] += 1;
			}

}
void menu() {
	printf("#### SZWA-Sejsmograf z wykorzystaniem akcelerometru #### \r\n");
	printf("		1 -> Usun wszystkie pomiary \r\n");
	printf("		2 -> Wyswietl najwyzszy pomiar \r\n");
	printf("		3 -> Zatrzymaj/wznow pomiary \r\n");
	printf("		4 -> Wyswietl ostatni pomiar \r\n");
	printf("		5 -> Wyswietl wszystkie pomiary \r\n");

}
void actualTime() {
	RTC_TimeTypeDef rtcTime;
	RTC_DateTypeDef rtcData;
	HAL_RTC_GetTime(&hrtc, &rtcTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &rtcData, RTC_FORMAT_BIN);
	printf("Date: %02d.%02d.20%02d", rtcData.Date, rtcData.Month, rtcData.Year);
	printf("	Time: %02d:%02d:%02d:%03ld\n\r", rtcTime.Hours, rtcTime.Minutes,
			rtcTime.Seconds, rtcTime.SubSeconds);

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
  MX_QUADSPI_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
	HAL_RTC_Init(&hrtc);
	BSP_QSPI_Init();
	LSM303C_AccInit(0x37);
	HAL_UART_Receive_IT(&huart2, &bp, 1);
	HAL_TIM_Base_Start_IT(&htim6);
//	clearMemory();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1) {


if(Start==1){

	if(newData==1){ acceler(); newData=0;}

}






		/*actualTime(); // print actual time
		nextMeasurement(421);

		memoryInfo();
		//		printf("x: %3d  y: %3d z: %3d  Wektor: %3d \r\n", dane[0], dane[1], dane[2], W);
		//		printf("Liczba zdarzen stopnia:\r\n");
		//		for(int i = 0; i < 6; i++) printf("		%d: %d\r\n", i+1, zdarzenia[i]);
		printf("x: %3f  y: %3f z: %3f  Wektor: %3f  \r\n", axisg[0], axisg[1],
				axisg[2], W);
		printf(
				"Liczba zdarzen stopnia: 1: %3d 2: %3d 3:  %3d 4: %3d 5: %3d 6: %3d",
				zdarzenia[0], zdarzenia[1], zdarzenia[2], zdarzenia[3],
				zdarzenia[4], zdarzenia[5]);
		printf("\r\n\n");
		listAllData();*/

		//HAL_Delay(2000);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
