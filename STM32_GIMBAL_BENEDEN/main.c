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
 * @brief STM32 GIMBAL_BENEDEN-module voor automatische deursturing.
 *
 * Deze STM32 stuurt twee servomotoren aan binnen het PES restaurantproject.
 * De onderste servo wordt gebruikt voor de automatische restaurantdeur en wordt
 * aangestuurd door afstandsdetectie vanaf de Encoder-STM32. De bovenste servo
 * wordt gebruikt voor de personeelsdeur en wordt aangestuurd wanneer de
 * RFID-module toegang verleent.
 *
 * De module ontvangt CANBUS-berichten vanaf Raspberry Pi-B en zet deze berichten
 * om naar PWM-aansturing voor de twee servomotoren.
 *
 * @author Arbër Deda
 * @date 2026
 */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/*!
 * @brief PWM-waarde voor de rechter servo-positie.
 */
#define SERVO_RECHTS 500

/*!
 * @brief PWM-waarde voor de middenpositie van de servo.
 */
#define SERVO_MIDDEN 800

/*!
 * @brief PWM-waarde voor de linker servo-positie.
 */
#define SERVO_LINKS 1000


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/*!
 * @brief Headerstructuur voor ontvangen CANBUS-berichten.
 *
 * Deze variabele bevat onder andere de CAN-ID en de datalengte van het ontvangen
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
 * @brief Tijdstip waarop de onderste servo/deur voor het laatst is geopend.
 *
 * Deze waarde wordt gebruikt om de automatische restaurantdeur na 5 seconden
 * weer te sluiten.
 */
uint32_t laatsteOpenTijd = 0;

/*!
 * @brief Tijdstip van de laatste scanbeweging.
 *
 * Deze variabele is beschikbaar voor scanlogica van de servo.
 */
uint32_t laatsteScan = 0;

/*!
 * @brief Huidige scanpositie van de servo.
 */
int scanPos = SERVO_LINKS;

/*!
 * @brief Richting waarin de scanpositie verandert.
 */
int richting = -1;

/*!
 * @brief Statusvariabele voor de bovenste servo.
 *
 * Als deze waarde 1 is, wordt de bovenste servo naar de actieve positie gestuurd.
 * Deze servo hoort bij de personeelsdeur die reageert op RFID-toegang.
 */
uint8_t bovenServoLinks = 0;

/*!
 * @brief Statusvariabele voor de onderste servo/deur.
 *
 * Als deze waarde 1 is, wordt de automatische restaurantdeur geopend. Deze
 * deur wordt aangestuurd na detectie vanaf de Encoder-STM32.
 */
uint8_t deurOpen = 0;

/*!
 * @brief Tijdstip waarop de bovenste servo voor het laatst is geactiveerd.
 *
 * Deze waarde wordt gebruikt om de bovenste servo na 5 seconden terug te zetten
 * naar de middenpositie.
 */
uint32_t laatsteDetectieBoven = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM16_Init(void);
static void MX_CAN1_Init(void);
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
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM16_Init();
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */

  /*!
   * Start de PWM-kanalen voor de twee servomotoren.
   *
   * TIM2 kanaal 2 wordt gebruikt voor de bovenste servo van de personeelsdeur.
   * TIM16 kanaal 1 wordt gebruikt voor de onderste servo van de automatische
   * restaurantdeur.
   */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);

  /*!
   * Zet beide servomotoren bij het opstarten in de middenpositie.
   */
  __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, SERVO_MIDDEN);

  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, SERVO_MIDDEN);

  /*!
   * CAN-filterconfiguratie.
   *
   * Het filter staat op mask mode met alle mask bits op nul. Hierdoor worden
   * alle CAN-berichten op FIFO0 geaccepteerd. De daadwerkelijke controle op
   * CAN-ID gebeurt in HAL_CAN_RxFifo0MsgPendingCallback().
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

  HAL_CAN_Start(&hcan1);

  /*!
   * Activeert CAN-interrupts voor ontvangen berichten.
   *
   * Wanneer een CAN-frame binnenkomt op FIFO0, wordt de callbackfunctie
   * HAL_CAN_RxFifo0MsgPendingCallback() uitgevoerd.
   */
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // Bovenste servo
      /*!
       * Stuurt de bovenste servo voor de personeelsdeur aan.
       *
       * Wanneer bovenServoLinks gelijk is aan 1, beweegt de bovenste servo naar
       * de actieve positie. Anders blijft de servo in de middenpositie.
       */
      if(bovenServoLinks == 1)
      {
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, SERVO_RECHTS);
      }
      else
      {
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, SERVO_MIDDEN);
      }

      // Onderste servo
      /*!
       * Stuurt de onderste servo voor de automatische restaurantdeur aan.
       *
       * Wanneer deurOpen gelijk is aan 1, opent de onderste servo de deur.
       * Wanneer deurOpen gelijk is aan 0, gaat de servo terug naar de
       * middenpositie.
       */
      if(deurOpen == 1)
      {
          __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, SERVO_RECHTS);
      }
      else
      {
          __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, SERVO_MIDDEN);
      }

      // Na 5 seconden onderste servo/deur sluiten
      /*!
       * Sluit de automatische restaurantdeur na 5 seconden.
       *
       * De timer wordt gestart zodra deurOpen op 1 wordt gezet door een
       * CAN-bericht van de Encoder-STM32.
       */
      if(deurOpen == 1 && HAL_GetTick() - laatsteOpenTijd > 5000)
      {
          deurOpen = 0;
      }

      // Na 5 seconden bovenste servo terug naar midden
      /*!
       * Zet de bovenste servo na 5 seconden terug naar de middenpositie.
       *
       * Deze logica zorgt ervoor dat de personeelsdeur na RFID-toegang niet
       * permanent open blijft staan.
       */
      if(bovenServoLinks == 1 && HAL_GetTick() - laatsteDetectieBoven > 5000)
      {
          bovenServoLinks = 0;
      }
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 79;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 79;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 19999;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */
  HAL_TIM_MspPostInit(&htim16);

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
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

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
 * op FIFO0. De callback verwerkt twee soorten berichten:
 *
 * - CAN-ID 0x500 met data 0x02 opent de onderste servo voor de automatische
 *   restaurantdeur. Dit bericht komt vanuit de afstandsdetectie van de
 *   Encoder-STM32.
 * - CAN-ID 0x501 met data 0x01 activeert de bovenste servo voor de
 *   personeelsdeur. Dit bericht komt vanuit de RFID-functionaliteit.
 *
 * Wanneer CAN-ID 0x501 met data 0x00 wordt ontvangen, wordt de bovenste servo
 * direct teruggezet naar de middenpositie.
 *
 * @param hcan Pointer naar de CAN-handle waarop het bericht is ontvangen.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);

    // Onderste servo openen
    if(RxHeader.StdId == 0x500)
    {
        if(RxData[0] == 2)
        {
            deurOpen = 1;
            laatsteOpenTijd = HAL_GetTick();
        }
    }

    // Bovenste servo naar links
    if(RxHeader.StdId == 0x501)
    {
        if(RxData[0] == 1)
        {
            bovenServoLinks = 1;
            laatsteDetectieBoven = HAL_GetTick();
        }

        if(RxData[0] == 0)
        {
            bovenServoLinks = 0;
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