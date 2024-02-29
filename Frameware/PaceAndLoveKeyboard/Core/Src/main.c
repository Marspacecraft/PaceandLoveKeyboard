/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "keyconfig.h"
#include "tracelog.h"
#include "usbd_agent.h"
#include "scheduler.h"
#include "keyboard.h"
#include "memery.h"
#include "flash.h"
#include "fatfs.h"
#include "EPD_2in7_V2.h"
#include "timertask.h"
#include "timeradc.h"
#include "joystick.h"
#include "fonts.h"
#include "st7302.h"
#include "rgb.h"
#include "ws2812b.h"
#include "oled.h"
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


void KB_Delay_us(uint32_t us)
{
	uint32_t i = us * 18;//nop,i--,judge,jump 72/4=18

	for(;i>0;i--)
	{
		__NOP();
	}
}

bool KB_ATOMIC_CAS(volatile uint8_t *pdst,uint8_t osrc,uint8_t nsrc)
{
	bool ret = false;
	
	__disable_irq();
	
	if(*pdst == osrc)
	{
		*pdst = nsrc;
		ret = true;
	}
	__DMB();
	__enable_irq();
	return ret;
}

bool KB_ATOMIC_CAS_ADDR(volatile void **pdst,void * osrc,void * nsrc)
{
	bool ret = false;
	
	__disable_irq();
	
	if(*pdst == osrc)
	{
		*pdst = nsrc;
		ret = true;
	}
	__DMB();
	__enable_irq();
	return ret;
}

void usb_dc_low_level_init(void)
{
    /* Peripheral clock enable */
    __HAL_RCC_USB_CLK_ENABLE();
    /* USB interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

}

void testcase_led_toggle(uint8_t* data)
{
	HAL_GPIO_TogglePin(POWER_LED_GPIO_Port,POWER_LED_Pin);	
}

void Throw_PowerLedTask()
{
	Start_CycleTimer(TIMER_PERIOD_500MS,testcase_led_toggle,NULL);
}

void USB_Init()
{
#ifndef TEST_USB_MSC
	if(MEM_FlagBit_IsEnabled(MEM_FLAG_USBMSC))
	{
		usbd_start_msc_mode();
		MEM_FlagBit_Reset(MEM_FLAG_USBMSC);
		MEM_FlagBit_Set(MEM_FLAG_BOOTLOADER);//enable bootloader to update firmware
	}
	else
	{
		usbd_start_hid_vcp_mode();
	}
#endif

}

void Main()
{
	RGB_CloseAll();
	RGB_Stop();
	CycleTimer_Init();
	Flash_Init();
	Fonts_Init();
	
	USB_Init();
	
	#ifdef SPI_EPAPER
		EPD_StartShow();
	#endif

	#ifdef I2C_OLED
		OLED_Init();
	#ifdef TEST_OLED
		testcase_oled();
	#endif
	#endif
	
	KEY_Init();
	Joystick_Init();

	#ifdef SPI_SHOW_LCD
	LCD_Init();
	#endif
	
	#ifdef USE_TESTCASE
		Throw_PowerLedTask();
	#endif
	
	#ifdef TEST_USB
		HAL_Delay(10000);
		testcase_usb();
	#endif

	#ifdef TEST_74HC165
		extern void testcase_74hc165(void);
 		testcase_74hc165();
	#endif

	#ifdef TEST_SCHEDULER
		testcase_scheduler();
	#endif

	#ifdef TEST_KEYTASKSCHEDULER
		testcase_keytaskscheduler();
	#endif

	#ifdef TEST_RGB
		void testcase_rgb(void);
		testcase_rgb();
	#endif

	#ifdef TEST_INTFLASH
		testcase_intflash();
	#endif
	
	#ifdef TEST_NORFLASH
		testcase_norflash();
	#endif

	#ifdef TEST_FATFS
		extern void testcase_fatfs(void);
		testcase_fatfs();
	#endif

	#ifdef TEST_USB_MSC
		testcase_usbmsc();
	#endif

	#ifdef TEST_EPAPER
		extern void testcase_ePaper(void);
		testcase_ePaper();
	#endif

	#ifdef TEST_TIMERTASK
 		testcase_timertask();
	#endif

	#ifdef TEST_ADC
		testcase_adc();
	#endif

	#ifdef TEST_JOYSTICK
		testcase_joystick();
	#else
		//ADC_Start();
		//Joystick_Start();
	#endif

	#ifdef TEST_FNKEY
		testcase_Fnkey();
	#endif

	#ifdef TEST_LCD
		extern void testcase_lcd(void);
		testcase_lcd();
	#endif

	#ifdef TEST_FONT
		extern void testcase_font(void);
 		testcase_font();
	#endif

	
	#ifdef TEST_REGMODE
		extern void testcase_testrgbmode(void);
		testcase_testrgbmode();	
	#endif

	#ifdef TEST_GT20
		extern void testcase_gt20(void);
		testcase_gt20();
	#endif

	ADC_Start();
	Joystick_Start();
	RGB_Start(RGB_Init_KeyCross);

	extern void ginisobeautiful();
	ginisobeautiful();
	
	HAL_Delay(1000);

	KEY_Scan_Start();
	
	while (1)
  	{
		do_schedule();

  	}
}

void sys_nvic_set_vector_table()
{
    SCB->VTOR = FLASH_BASE | (APP_OFFSET & (uint32_t)0xFFFFFE00);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//#ifndef USE_TESTCASE
		sys_nvic_set_vector_table();
	//#endif
	IntFlash_Init();
	init_scheduler();
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
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_TIM8_Init();
  MX_ADC1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
	TRACELOG_INFO("KeyBoard start!");
  TRACELOG_INFO(PALK_VERSION);
	Main();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
  TRACELOG_ERROR(unknow_error_Handler);
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
