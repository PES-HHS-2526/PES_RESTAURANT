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

/*!
 * @file main.c
 * @brief STM32 GIMBAL_BOVEN-module voor klantoproepen in de keuken.
 *
 * Deze STM32 wordt gebruikt als keuken-/serveersterunit voor tafel 1 en tafel 2
 * binnen het PES restaurantproject. Wanneer een klant op de Wemos-knop van
 * tafel 1 of tafel 2 drukt, ontvangt deze STM32 via CANBUS een bericht en gaat
 * de bijbehorende keuken-LED aan.
 *
 * Wanneer de medewerker of serveerster op de resetknop van de juiste tafel drukt,
 * gaat de keuken-LED uit en wordt via CANBUS een resetbericht teruggestuurd.
 * Dit resetbericht wordt via Raspberry Pi-B en Raspberry Pi-A doorgestuurd naar
 * de Wemos bij de klanttafel, zodat ook de LED bij de klanttafel uitgaat.
 *
 * @author Arbër Deda
 * @date 2026
 */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
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
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/*!
 * @brief Headerstructuur voor te verzenden CANBUS-berichten.
 *
 * Deze variabele bevat onder andere de CAN-ID, het type frame en de datalengte
 * van het CAN-bericht dat naar Raspberry Pi-B wordt verzonden.
 */
CAN_TxHeaderTypeDef TxHeader;

/*!
 * @brief Databuffer voor te verzenden CANBUS-berichten.
 *
 * Deze buffer bevat de data-byte die wordt meegestuurd met een resetbericht.
 */
uint8_t TxData[1];

/*!
 * @brief Mailbox waarin het verzonden CANBUS-bericht wordt geplaatst.
 */
uint32_t TxMailbox;

/*!
 * @brief Headerstructuur voor ontvangen CANBUS-berichten.
 *
 * Deze variabele bevat onder andere de CAN-ID en de datalengte van een ontvangen
 * CAN-frame.
 */
CAN_RxHeaderTypeDef RxHeader;

/*!
 * @brief Databuffer voor ontvangen CANBUS-berichten.
 *
 * Deze buffer bevat de data-bytes van het ontvangen CAN-frame.
 */
uint8_t RxData[8];

/*!
 * @brief Vorige status van knop 0.
 *
 * Knop 0 hoort bij tafel 1. Deze variabele wordt gebruikt voor flankdetectie,
 * zodat er alleen een resetbericht wordt verstuurd wanneer de knop net wordt
 * ingedrukt.
 */
uint8_t lastKnop0 = GPIO_PIN_SET;

/*!
 * @brief Vorige status van knop 1.
 *
 * Knop 1 hoort bij tafel 2. Deze variabele wordt gebruikt voor flankdetectie,
 * zodat er alleen een resetbericht wordt verstuurd wanneer de knop net wordt
 * ingedrukt.
 */
uint8_t lastKnop1 = GPIO_PIN_SET;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*!
 * @brief Stuurt tekst via UART naar de seriële monitor.
 *
 * Deze functie wordt gebruikt voor debugmeldingen tijdens het testen van de
 * knoppen, CANBUS-communicatie en ontvangen berichten.
 *
 * @param text Tekst die via UART verzonden moet worden.
 */
void uartPrint(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)text, strlen(text), 100);
}

/*!
 * @brief Verstuurt een CANBUS-bericht met één data-byte.
 *
 * Deze functie stelt de CAN-header in, vult de data-byte en verstuurt het
 * bericht via CAN1. De functie controleert eerst of er een vrije CAN-mailbox
 * beschikbaar is. Als er geen mailbox vrij is, worden de mailboxes geaborteerd
 * en wordt het bericht niet verzonden.
 *
 * Binnen deze STM32 wordt deze functie gebruikt om resetberichten van tafel 1
 * en tafel 2 terug te sturen naar Raspberry Pi-B.
 *
 * @param id CAN-ID van het bericht.
 * @param value Data-byte die meegestuurd wordt.
 */
void CAN_Send(uint32_t id, uint8_t value)
{
    char buf[100];

    TxHeader.StdId = id;
    TxHeader.ExtId = 0;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 1;
    TxHeader.TransmitGlobalTime = DISABLE;

    TxData[0] = value;

    uint32_t free = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);

    int len = snprintf(buf, sizeof(buf), "Voor send: free mailboxes=%lu err=0x%08lX\r\n", free, HAL_CAN_GetError(&hcan1));
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);

    if(free == 0)
    {
        uartPrint("GEEN vrije CAN mailbox NIET verzonden\r\n");
        HAL_CAN_AbortTxRequest(&hcan1, CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
        uartPrint("Mailboxes geaborteerd, druk opnieuw op de knop\r\n");
        return;
    }

    HAL_StatusTypeDef ret = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);

    if(ret == HAL_OK)
    {
        len = snprintf(buf, sizeof(buf), "CAN ECHT verstuurd: ID=0x%03lX DATA=%02X mailbox=0x%08lX\r\n", id, value, TxMailbox);
        HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);
    }
    else
    {
        len = snprintf(buf, sizeof(buf), "CAN_SEND ERROR ret=%d err=0x%08lX\r\n", ret, HAL_CAN_GetError(&hcan1));
        HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);
    }
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
  MX_USART2_UART_Init();
  MX_CAN1_Init();
  //MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /*!
   * Startmelding voor de seriële monitor.
   *
   * Hiermee is tijdens het testen zichtbaar dat het programma is gestart.
   */
  uartPrint("Knop + CAN test gestart\r\n");

  /*!
   * Zet de keuken-LED's van tafel 1 en tafel 2 uit bij het opstarten.
   *
   * PA8 hoort bij tafel 1 en PA10 hoort bij tafel 2.
   */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8 | GPIO_PIN_10, GPIO_PIN_RESET);

  /*!
   * CAN-filterconfiguratie.
   *
   * Het filter staat op mask mode met alle mask bits op nul. Hierdoor worden
   * alle CAN-berichten op FIFO0 geaccepteerd. In de CAN receive callback wordt
   * daarna gecontroleerd of het bericht bij tafel 1 of tafel 2 hoort.
   */
  CAN_FilterTypeDef sFilterConfig;

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

  /*!
   * Start CAN1 en geeft via UART aan of het starten gelukt is.
   */
  if(HAL_CAN_Start(&hcan1) == HAL_OK) uartPrint("CAN gestart OK\r\n");
  else uartPrint("CAN start ERROR\r\n");

  /*!
   * Activeert CAN-interrupts voor ontvangen berichten.
   *
   * Wanneer een CAN-frame binnenkomt, wordt HAL_CAN_RxFifo0MsgPendingCallback()
   * aangeroepen.
   */
  if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) == HAL_OK) uartPrint("CAN interrupt OK\r\n");
  else uartPrint("CAN interrupt ERROR\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint8_t knop0 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
	      uint8_t knop1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9);

	      // Knop 0 ingedrukt: stuur 0x300 data 00 en zet LED0 uit
	      /*!
	       * Verwerkt de resetknop voor tafel 1.
	       *
	       * Wanneer knop 0 net wordt ingedrukt, wordt LED0 in de keuken
	       * uitgezet en wordt CAN-ID 0x300 met data 0x00 verzonden.
	       *
	       * Dit resetbericht wordt door Raspberry Pi-B en Raspberry Pi-A
	       * doorgestuurd naar de Wemos van tafel 1, zodat ook de LED bij de
	       * klanttafel uitgaat.
	       */
	      if(knop0 == GPIO_PIN_RESET && lastKnop0 == GPIO_PIN_SET)
	      {
	          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

	          CAN_Send(0x300, 0x00);

	          uartPrint("Knop 0 ingedrukt -> CAN 0x300 data 00, LED0 uit\r\n");

	          HAL_Delay(200);
	      }

	      // Knop 1 ingedrukt: stuur 0x301 data 00 en zet LED1 uit
	      /*!
	       * Verwerkt de resetknop voor tafel 2.
	       *
	       * Wanneer knop 1 net wordt ingedrukt, wordt LED1 in de keuken
	       * uitgezet en wordt CAN-ID 0x301 met data 0x00 verzonden.
	       *
	       * Dit resetbericht wordt door Raspberry Pi-B en Raspberry Pi-A
	       * doorgestuurd naar de Wemos van tafel 2, zodat ook de LED bij de
	       * klanttafel uitgaat.
	       */
	      if(knop1 == GPIO_PIN_RESET && lastKnop1 == GPIO_PIN_SET)
	      {
	          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

	          CAN_Send(0x301, 0x00);

	          uartPrint("Knop 1 ingedrukt -> CAN 0x301 data 00, LED1 uit\r\n");

	          HAL_Delay(200);
	      }

	      lastKnop0 = knop0;
	      lastKnop1 = knop1;
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

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
  RCC_OscInitStruct.PLL.PLLN = 16;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

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
  hi2c1.Init.Timing = 0x00B07CB4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD3_Pin */
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*!
 * @brief Callback voor ontvangen CANBUS-berichten.
 *
 * Deze functie wordt automatisch aangeroepen wanneer er een CAN-frame binnenkomt
 * op FIFO0. De callback verwerkt klantoproepen van tafel 1 en tafel 2.
 *
 * CAN-ID 0x300 hoort bij tafel 1:
 * - data 0x01 zet LED0 in de keuken aan;
 * - data 0x00 zet LED0 in de keuken uit.
 *
 * CAN-ID 0x301 hoort bij tafel 2:
 * - data 0x01 zet LED1 in de keuken aan;
 * - data 0x00 zet LED1 in de keuken uit.
 *
 * @param hcan Pointer naar de CAN-handle waarop het bericht is ontvangen.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    uartPrint("CALLBACK!\r\n");

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);

    char buf[80];
    int len = snprintf(buf, sizeof(buf), "ID=0x%03lX DATA=%02X DLC=%lu\r\n", RxHeader.StdId, RxData[0], RxHeader.DLC);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);

    if(RxHeader.StdId == 0x300 && RxHeader.DLC > 0)
    {
        if(RxData[0] == 0x01)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
            uartPrint("CAN 0x300 data 01 ontvangen -> LED0 aan\r\n");
        }
        else if(RxData[0] == 0x00)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
            uartPrint("CAN 0x300 data 00 ontvangen -> LED0 uit\r\n");
        }
    }

    else if(RxHeader.StdId == 0x301 && RxHeader.DLC > 0)
    {
        if(RxData[0] == 0x01)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
            uartPrint("CAN 0x301 data 01 ontvangen -> LED1 aan\r\n");
        }
        else if(RxData[0] == 0x00)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
            uartPrint("CAN 0x301 data 00 ontvangen -> LED1 uit\r\n");
        }
    }
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