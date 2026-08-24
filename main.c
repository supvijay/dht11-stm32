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
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "string.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define port GPIOC
#define pin GPIO_PIN_13
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
uint8_t Response_status = 5; // To know if initialized by watchdog
uint16_t before = 0;
uint16_t after = 0;
uint8_t prepare_low = 0;
uint8_t prepare_high = 0;
uint8_t	data[40]={0};
uint8_t Checksum = 0;
float Temperature = 0;
float Humidity = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



void Delay_us(uint16_t delay)
 { __HAL_TIM_SET_COUNTER(&htim1,0); // set counter to 0
	while (__HAL_TIM_GET_COUNTER(&htim1) < delay)  // wait for the counter to reach the delay input in the parameter
	   {
	       };
 }

void Test_Delay_us(void)
{
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);

for (int i=0; i<20; i++)
{
	  Delay_us(50000);
}
HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
before = HAL_GetTick();
for (int i=0; i<40; i++)
{
	  Delay_us(50000);
}
after = HAL_GetTick();

}

void Set_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = GPIO_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

   }

void Set_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = GPIO_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void DHT_Startup(void)
{ Set_Output(port, pin);
  HAL_Delay (2000);
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
  Delay_us(18000);
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
  Delay_us(40);
	}

uint8_t DHT_Response(void)
{   uint8_t response = 0;
	Set_Input(port, pin);
	prepare_low = HAL_GPIO_ReadPin(port, pin);  // value = 0
    if  (!(HAL_GPIO_ReadPin(port, pin)))  // if low
	{ Delay_us(90);
	prepare_high = HAL_GPIO_ReadPin(port, pin); // value = 1

	  if  (HAL_GPIO_ReadPin(port, pin))  // if high
	      {
		   response = 1;
	      }
	  else
	      {
		   response = -1;
	      }}
	  while (HAL_GPIO_ReadPin(port, pin)) // wait for the pin to go low
		  {

		  }

	  return response ;

	 }

void DHT_Data(void)
{
 HAL_IWDG_Refresh(&hiwdg);
for (uint8_t i=40; i>0; i--)
{
	// Wait for the pin to go HIGH (start of bit)
	        while (!HAL_GPIO_ReadPin(port, pin)){}

	        // Wait 40us then read the bit
	        Delay_us(40);

	        if (HAL_GPIO_ReadPin(port, pin))
	            data[i-1] = 1;
	        else
	            data[i-1] = 0;

	        // Wait for the pin to go LOW again before next bit
	        while (HAL_GPIO_ReadPin(port, pin)){}
}
}

uint8_t Binary_conversion(uint8_t data[],uint8_t start)
{ uint8_t binary = 0;
    for (uint8_t i = 0 ;i<8;i++)
    {
    	binary = binary | (data[start+i] << i) ;
    }
	return binary;
}

void Check_sum (void)
{
	uint8_t sum = 0 ;
	sum = Binary_conversion(data,8) + Binary_conversion(data,16)
            + Binary_conversion(data,24) + Binary_conversion(data,32);
	if
	(Binary_conversion(data,0) == sum & 0xFF )
	{
		Checksum = 1;
	}
	else
	{
		Checksum = 0;
	}
}

void Calculate_temperature_humidity(void)
{
	Temperature = Binary_conversion(data,16) +  (Binary_conversion(data,8)/100.00) ;
	Humidity = Binary_conversion(data,32) +  (Binary_conversion(data,24)/100.00) ;


}

void Send_temperature_humidity(void)
{
    char buffer[64];
    sprintf(buffer, "Temperature: %.2f°C  Humidity: %.2f%%\r\n", Temperature, Humidity);
    CDC_Transmit_FS((uint8_t *)buffer, strlen(buffer));
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
  MX_TIM1_Init();
  MX_IWDG_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
__HAL_RCC_GPIOC_CLK_ENABLE();
  DHT_Startup();
  Response_status = DHT_Response();

if (Response_status)
 {
  DHT_Data();
  Check_sum();
  if (Checksum)
	{
	  Calculate_temperature_humidity();
	  Send_temperature_humidity();
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 5;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 3999;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

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
  htim1.Init.Prescaler = 59;
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
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

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
