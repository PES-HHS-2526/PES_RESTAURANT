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
 * @brief STM32 Encoder-module binnen het PES restaurantproject.
 *
 * Deze STM32 combineert meerdere functies binnen het embedded systeem:
 * CANBUS-communicatie met Raspberry Pi-B, RGB-LED aansturing voor de
 * CO2/eCO2-ventilatiestatus, aansturing van een MAX7219 LED-matrix via SPI,
 * ultrasoonsensorfunctionaliteit via trigger/echo voor detectie bij de
 * restaurantingang en knopdetectie met CAN-terugmelding.
 *
 * @author Arbër Deda
 * @author Haki Abdulovski
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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/*!
 * @brief Huidige knopstatus voor de knopdetectie.
 */
uint8_t knop_state = 0;

/*!
 * @brief Vorige knopstatus voor flankdetectie.
 *
 * Deze variabele wordt gebruikt om te bepalen of de knop net is ingedrukt
 * of net is losgelaten.
 */
uint8_t last_knop = 0;

/*!
 * @brief Tijdstip van de laatste detectie.
 *
 * Deze variabele kan worden gebruikt om detecties of metingen in tijd te
 * begrenzen.
 */
uint32_t laatsteDetectie = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>

/*!
 * @brief Maakt een korte vertraging in microseconden met TIM1.
 *
 * De timer-counter wordt eerst op nul gezet. Daarna wacht de functie totdat
 * de teller de opgegeven delaywaarde heeft bereikt.
 *
 * Deze functie wordt gebruikt voor de triggerpuls van de ultrasoonsensor.
 *
 * @param delay Gewenste vertraging in microseconden.
 */
void microDelay(uint16_t delay)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    while(__HAL_TIM_GET_COUNTER(&htim1) < delay);
}

/*!
 * @brief Zet alle RGB-LED kanalen uit.
 *
 * Deze functie reset de gebruikte GPIO-pinnen voor rood, groen en blauw zodat
 * de RGB-LED volledig uit staat voordat een nieuwe kleur wordt ingeschakeld.
 */
void RGB_Uit()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
}

/*!
 * @brief Zet de RGB-LED op rood.
 *
 * Binnen het systeem wordt rood gebruikt als waarschuwingstoestand voor de
 * CO2/eCO2-ventilatiestatus.
 */
void RGB_Rood()
{
    RGB_Uit();

    // CO2:0 = ventilatie uit
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
}

/*!
 * @brief Zet de RGB-LED op groen.
 *
 * Binnen het systeem wordt groen gebruikt als veilige of normale toestand voor
 * de CO2/eCO2-ventilatiestatus.
 */
void RGB_Groen()
{
    RGB_Uit();

    // CO2:1 = ventilatie aan
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
}

// ===== LICHTKRANT (MAX7219) =====

/*!
 * @brief Buffer voor het versturen van MAX7219 SPI-data.
 *
 * De eerste byte bevat het registeradres en de tweede byte bevat de waarde die
 * naar de MAX7219 LED-driver gestuurd wordt.
 */
uint8_t spiData[2];

/*!
 * @brief Verstuurt één registerwaarde naar de MAX7219 LED-driver.
 *
 * Deze functie gebruikt SPI om twee bytes naar de MAX7219 te sturen: het
 * registeradres en de bijbehorende waarde. De chip-select pin wordt laag gemaakt
 * tijdens het verzenden en daarna weer hoog gezet.
 *
 * @param address Registeradres van de MAX7219.
 * @param value Waarde die naar het register gestuurd wordt.
 */
void max7219Send(uint8_t address, uint8_t value)
{
    spiData[0] = address;
    spiData[1] = value;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, spiData, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

/*!
 * @brief Initialiseert de MAX7219 LED-matrix.
 *
 * Deze functie configureert de MAX7219-driver door onder andere testmodus uit
 * te zetten, normal operation aan te zetten, decode mode uit te schakelen,
 * alle acht rijen te gebruiken en de helderheid in te stellen.
 */
void max7219Init()
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay(100);
    max7219Send(0x0F, 0x00);
    max7219Send(0x0C, 0x01);
    max7219Send(0x09, 0x00);
    max7219Send(0x0B, 0x07);
    max7219Send(0x0A, 0x08);
}

/*!
 * @brief Fonttabel voor letters, cijfers en enkele symbolen.
 *
 * Elke rij bevat een 8-byte patroon voor één karakter. Deze patronen worden
 * gebruikt om tekst op de MAX7219 LED-matrix te tonen.
 */
const uint8_t font[44][8] =
{
    {0x7E,0x09,0x09,0x09,0x7E,0x00,0x00,0x00}, // A
    {0x7F,0x49,0x49,0x49,0x36,0x00,0x00,0x00}, // B
    {0x3E,0x41,0x41,0x41,0x22,0x00,0x00,0x00}, // C
    {0x7F,0x41,0x41,0x22,0x1C,0x00,0x00,0x00}, // D
    {0x7F,0x49,0x49,0x49,0x41,0x00,0x00,0x00}, // E
    {0x7F,0x09,0x09,0x09,0x01,0x00,0x00,0x00}, // F
    {0x3E,0x41,0x49,0x49,0x7A,0x00,0x00,0x00}, // G
    {0x7F,0x08,0x08,0x08,0x7F,0x00,0x00,0x00}, // H
    {0x41,0x41,0x7F,0x41,0x41,0x00,0x00,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01,0x00,0x00,0x00}, // J
    {0x7F,0x08,0x14,0x22,0x41,0x00,0x00,0x00}, // K
    {0x7F,0x40,0x40,0x40,0x40,0x00,0x00,0x00}, // L
    {0x7F,0x02,0x04,0x02,0x7F,0x00,0x00,0x00}, // M
    {0x7F,0x04,0x08,0x10,0x7F,0x00,0x00,0x00}, // N
    {0x3E,0x41,0x41,0x41,0x3E,0x00,0x00,0x00}, // O
    {0x7F,0x09,0x09,0x09,0x06,0x00,0x00,0x00}, // P
    {0x3E,0x41,0x51,0x21,0x5E,0x00,0x00,0x00}, // Q
    {0x7F,0x09,0x19,0x29,0x46,0x00,0x00,0x00}, // R
    {0x46,0x49,0x49,0x49,0x31,0x00,0x00,0x00}, // S
    {0x01,0x01,0x7F,0x01,0x01,0x00,0x00,0x00}, // T
    {0x3F,0x40,0x40,0x40,0x3F,0x00,0x00,0x00}, // U
    {0x1F,0x20,0x40,0x20,0x1F,0x00,0x00,0x00}, // V
    {0x7F,0x20,0x18,0x20,0x7F,0x00,0x00,0x00}, // W
    {0x63,0x14,0x08,0x14,0x63,0x00,0x00,0x00}, // X
    {0x03,0x04,0x78,0x04,0x03,0x00,0x00,0x00}, // Y
    {0x61,0x51,0x49,0x45,0x43,0x00,0x00,0x00}, // Z
    {0x7C,0xA2,0x92,0x8A,0x7C,0x00,0x00,0x00}, // 0
    {0x00,0x04,0x02,0x7F,0x00,0x00,0x00,0x00}, // 1
    {0x62,0x92,0x92,0x92,0x8C,0x00,0x00,0x00}, // 2
    {0x44,0x82,0x92,0x92,0x6C,0x00,0x00,0x00}, // 3
    {0x18,0x14,0x12,0x7F,0x10,0x00,0x00,0x00}, // 4
    {0x4E,0x8A,0x8A,0x8A,0x72,0x00,0x00,0x00}, // 5
    {0x7C,0x92,0x92,0x92,0x64,0x00,0x00,0x00}, // 6
    {0x02,0xE2,0x12,0x0A,0x06,0x00,0x00,0x00}, // 7
    {0x6C,0x92,0x92,0x92,0x6C,0x00,0x00,0x00}, // 8
    {0x4C,0x92,0x92,0x92,0x7C,0x00,0x00,0x00}, // 9
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // (spatie)
    {0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00}, // !
    {0x02,0x01,0x51,0x09,0x06,0x00,0x00,0x00}, // ?
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, // :
    {0x08,0x08,0x08,0x08,0x08,0x00,0x00,0x00}, // -
    {0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00}, // .
    {0x00,0x04,0x02,0x01,0x00,0x00,0x00,0x00}, // /
    {0x08,0x08,0x3E,0x08,0x08,0x00,0x00,0x00}  // +
};

/*!
 * @brief Bepaalt de index van een karakter in de fonttabel.
 *
 * Deze functie vertaalt letters, cijfers en enkele symbolen naar de juiste
 * index in de font-array. Onbekende karakters worden als spatie behandeld.
 *
 * @param c Karakter dat opgezocht moet worden.
 * @return Index van het karakter in de fonttabel.
 */
int getFontIndex(char c)
{
    if(c >= 'A' && c <= 'Z') return c - 'A';
    if(c >= '0' && c <= '9') return 26 + (c - '0');
    if(c == ' ') return 36;
    if(c == '!') return 37;
    if(c == '?') return 38;
    if(c == ':') return 39;
    if(c == '-') return 40;
    if(c == '.') return 41;
    if(c == '/') return 42;
    if(c == '+') return 43;
    return 36;
}

/*!
 * @brief Voert één scrollstap uit voor tekst op de LED-matrix.
 *
 * Deze functie bouwt een displaybuffer op basis van het meegegeven bericht.
 * Als het bericht veranderd is, wordt de buffer opnieuw opgebouwd. Daarna wordt
 * elke 150 ms een nieuw deel van de buffer naar de MAX7219 gestuurd, waardoor
 * het bericht scrollend wordt weergegeven.
 *
 * @param message Pointer naar de tekst die scrollend weergegeven moet worden.
 */
void scrollStep(char *message)
{
    static uint8_t  displayBuffer[256];
    static int      bufferLen    = 0;
    static int      offset       = 0;
    static uint32_t lastScroll   = 0;
    static char    *lastMessage  = NULL;

    // Herbouw buffer als bericht veranderd is
    if (message != lastMessage)
    {
        lastMessage  = message;
        bufferLen    = 0;
        offset       = 0;

        for (int i = 0; message[i] != '\0'; i++)
        {
            int index = getFontIndex(message[i]);
            for (int row = 0; row < 8; row++)
                displayBuffer[bufferLen++] = font[index][row];
            displayBuffer[bufferLen++] = 0x00;
        }
    }

    // Alleen scrollen als 150ms verstreken is
    if (HAL_GetTick() - lastScroll >= 150)
    {
        lastScroll = HAL_GetTick();

        for (int row = 0; row < 8; row++)
            max7219Send(row + 1, displayBuffer[offset + row]);

        offset++;
        if (offset > bufferLen - 8)
            offset = 0;  // begin opnieuw
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
	char buffer[64];

	CAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[1];
	uint32_t TxMailbox;

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
  MX_I2C1_Init();
  MX_TIM1_Init();
  HAL_TIM_Base_Start(&htim1);
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /*!
   * CAN-filterconfiguratie.
   *
   * Het filter staat op mask mode met alle mask bits op nul. Hierdoor worden
   * alle CAN-berichten op FIFO0 geaccepteerd. De daadwerkelijke filtering op
   * CAN-ID gebeurt later in de receive callback.
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
   * Activeert CAN-interrupts voor ontvangen berichten op FIFO0.
   *
   * Wanneer een CAN-frame binnenkomt, wordt HAL_CAN_RxFifo0MsgPendingCallback()
   * aangeroepen.
   */
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  /*!
   * Initialiseert de MAX7219 LED-matrix en zet de RGB-LED in de startstatus.
   */
  max7219Init();

  RGB_Groen();

  HAL_Delay(500);

  // scan (debug)
  for (uint8_t i = 0; i < 128; i++)
  {
      if (HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, 100) == HAL_OK)
      {
          sprintf(buffer, "Found device at 0x%X\r\n", i);
          HAL_UART_Transmit(&huart2,(uint8_t*)buffer,strlen(buffer),1000);
      }
  }

  HAL_Delay(2000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
    {
	  uint8_t current = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);

	  char msg[50];

	   /*!
	    * Detecteert of de knop net is ingedrukt.
	    *
	    * Bij een overgang van HIGH naar LOW wordt CAN-ID 0x300 met data 0x01
	    * verstuurd. Dit geeft aan dat de knop is ingedrukt.
	    */
	   if (current == GPIO_PIN_RESET && last_knop == GPIO_PIN_SET)
	  {

	      TxHeader.StdId = 0x300;
	      TxHeader.IDE = CAN_ID_STD;
	      TxHeader.RTR = CAN_RTR_DATA;
	      TxHeader.DLC = 1;

		  TxData[0] = 1;   // knop gedrukt = TRUE

	      if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
	      {
	          HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	      }

	      HAL_UART_Transmit(&huart2, (uint8_t*)"KNOP GEDRUKT\r\n", 15, 100);
	  }

	   /*!
	    * Detecteert of de knop net is losgelaten.
	    *
	    * Bij een overgang van LOW naar HIGH wordt CAN-ID 0x300 met data 0x00
	    * verstuurd. Dit geeft aan dat de knop is losgelaten of gereset.
	    */
	   if (current == GPIO_PIN_SET && last_knop == GPIO_PIN_RESET)
	   {
	       TxHeader.StdId = 0x300;
	       TxHeader.IDE = CAN_ID_STD;
	       TxHeader.RTR = CAN_RTR_DATA;
	       TxHeader.DLC = 1;

	       TxData[0] = 0;
	       if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
	       {
	           HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	       }

	       HAL_UART_Transmit(&huart2, (uint8_t*)"UIT\r\n", 5, 100);
	   }

	  last_knop = current;
	  static uint32_t lastSensor = 0;

	  /*!
	   * Laat de lichtkrant blijven scrollen terwijl de ultrasoonsensor maar
	   * elke 500 ms opnieuw wordt gemeten.
	   */
	  if (HAL_GetTick() - lastSensor < 500)
	  {
	      scrollStep("1+4GRATIS");
	      continue;
	  }
	  lastSensor = HAL_GetTick();
	  TxHeader.StdId = 0x500;
	  TxHeader.IDE = CAN_ID_STD;
	  TxHeader.RTR = CAN_RTR_DATA;
	  TxHeader.DLC = 1;
	  // TRIGGER laag
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	  microDelay(2);

	  // 10 us triggerpuls
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	  microDelay(10);

	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

	  uint32_t startTick = 0;
	  uint32_t endTick = 0;

	  // wacht op echo HIGH
	  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET);

	  startTick = __HAL_TIM_GET_COUNTER(&htim1);

	  // wacht op echo LOW
	  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET);

	  endTick = __HAL_TIM_GET_COUNTER(&htim1);

	  uint32_t duration = endTick - startTick;

	  // afstand berekenen
	  float distance = duration * 0.034 / 2;
	  if(distance > 81.0)
	  {
	      distance = 81.0;
	  }

	  static uint8_t lastDetectie = 0;

	  uint8_t detectie;

	  /*!
	   * Stuurt een CAN-bericht wanneer de gemeten afstand kleiner is dan 5 cm.
	   *
     * CAN-ID 0x500 met data 0x02 wordt gebruikt als detectiebericht voor de
     * automatische ingangdeur van het restaurant. Wanneer een persoon/object dicht
     * bij de ultrasoonsensor komt, wordt dit via CANBUS naar Raspberry Pi-B gestuurd.
	   */
	  if(distance < 5.0)
	  {
	      TxHeader.StdId = 0x500;
	      TxHeader.IDE = CAN_ID_STD;
	      TxHeader.RTR = CAN_RTR_DATA;
	      TxHeader.DLC = 1;

	      TxData[0] = 2;

	      if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
	      {
	          HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	      }
	  }

	  //HAL_Delay(500);

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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 79;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, TRIGGER_Pin|GPIO_PIN_3|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pins : TRIGGER_Pin PA3 PA8 PA9
                           PA10 */
  GPIO_InitStruct.Pin = TRIGGER_Pin|GPIO_PIN_3|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ECHO_Pin */
  GPIO_InitStruct.Pin = ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*!
 * @brief Callback voor ontvangen CANBUS-berichten.
 *
 * Deze functie wordt automatisch aangeroepen wanneer er een CAN-frame binnenkomt
 * op FIFO0. In deze STM32-module wordt CAN-ID 0x101 gebruikt voor de
 * CO2/eCO2-ventilatiestatus.
 *
 * Bij data 0x01 wordt de RGB-LED rood gezet. Bij data 0x00 wordt de RGB-LED
 * groen gezet.
 *
 * @param hcan Pointer naar de CAN-handle waarop het bericht is ontvangen.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);

    if(RxHeader.StdId == 0x101 && RxHeader.DLC > 0)
    {
        if(RxData[0] == 0x01)
        {
            RGB_Rood();
        }
        else if(RxData[0] == 0x00)
        {
            RGB_Groen();
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