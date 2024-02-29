/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#ifndef true
	#define true (0==0)
#endif

#ifndef false
	#define false (1==0)
#endif

#ifndef NULL
	#define NULL ((void *)0)
#endif

#ifndef bool
	typedef uint8_t bool;
#endif
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define POWER_LED_Pin GPIO_PIN_13
#define POWER_LED_GPIO_Port GPIOC
#define JOYSTICK_ADC0_Pin GPIO_PIN_0
#define JOYSTICK_ADC0_GPIO_Port GPIOC
#define JOYSTICK_ADC1_Pin GPIO_PIN_1
#define JOYSTICK_ADC1_GPIO_Port GPIOC
#define JOYSTICK_KEY_Pin GPIO_PIN_2
#define JOYSTICK_KEY_GPIO_Port GPIOC
#define PWR_ADC_Pin GPIO_PIN_3
#define PWR_ADC_GPIO_Port GPIOC
#define FN_KEY_WAKEUP_Pin GPIO_PIN_0
#define FN_KEY_WAKEUP_GPIO_Port GPIOA
#define FN_KEY_WAKEUP_EXTI_IRQn EXTI0_IRQn
#define PWR_STDBY_Pin GPIO_PIN_1
#define PWR_STDBY_GPIO_Port GPIOA
#define PWR_CHRG_Pin GPIO_PIN_4
#define PWR_CHRG_GPIO_Port GPIOA
#define KEY_SPI1_SCK_Pin GPIO_PIN_5
#define KEY_SPI1_SCK_GPIO_Port GPIOA
#define KEY_SPI1_MISO_Pin GPIO_PIN_6
#define KEY_SPI1_MISO_GPIO_Port GPIOA
#define KEY_PL_Pin GPIO_PIN_7
#define KEY_PL_GPIO_Port GPIOA
#define FFT_STROBE_Pin GPIO_PIN_4
#define FFT_STROBE_GPIO_Port GPIOC
#define FFT_RESET_Pin GPIO_PIN_5
#define FFT_RESET_GPIO_Port GPIOC
#define FFT_ADC_R_Pin GPIO_PIN_0
#define FFT_ADC_R_GPIO_Port GPIOB
#define FFT_ADC_L_Pin GPIO_PIN_1
#define FFT_ADC_L_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define OLED_I2C2_SCL_Pin GPIO_PIN_10
#define OLED_I2C2_SCL_GPIO_Port GPIOB
#define OLED_I2C2_SDA_Pin GPIO_PIN_11
#define OLED_I2C2_SDA_GPIO_Port GPIOB
#define EPD_LED_Pin GPIO_PIN_12
#define EPD_LED_GPIO_Port GPIOB
#define EXT_SPI2_CLK_Pin GPIO_PIN_13
#define EXT_SPI2_CLK_GPIO_Port GPIOB
#define EXT_SPI2_MISO_Pin GPIO_PIN_14
#define EXT_SPI2_MISO_GPIO_Port GPIOB
#define EXT_SPI2_MOSI_Pin GPIO_PIN_15
#define EXT_SPI2_MOSI_GPIO_Port GPIOB
#define RGB_PWM_Pin GPIO_PIN_6
#define RGB_PWM_GPIO_Port GPIOC
#define LED_PWM_Pin GPIO_PIN_7
#define LED_PWM_GPIO_Port GPIOC
#define EPD_CS_Pin GPIO_PIN_8
#define EPD_CS_GPIO_Port GPIOC
#define EPD_DC_Pin GPIO_PIN_9
#define EPD_DC_GPIO_Port GPIOC
#define EPD_RST_Pin GPIO_PIN_8
#define EPD_RST_GPIO_Port GPIOA
#define EPD_BUSY_Pin GPIO_PIN_9
#define EPD_BUSY_GPIO_Port GPIOA
#define EPD_PWR_Pin GPIO_PIN_10
#define EPD_PWR_GPIO_Port GPIOA
#define BLE_CMD3_Pin GPIO_PIN_15
#define BLE_CMD3_GPIO_Port GPIOA
#define BLE_CMD0_Pin GPIO_PIN_10
#define BLE_CMD0_GPIO_Port GPIOC
#define FLASH_CS_Pin GPIO_PIN_11
#define FLASH_CS_GPIO_Port GPIOC
#define FLASH_IO3_Pin GPIO_PIN_12
#define FLASH_IO3_GPIO_Port GPIOC
#define FLASH_IO2_Pin GPIO_PIN_2
#define FLASH_IO2_GPIO_Port GPIOD
#define FLASH_SPI3_CLK_Pin GPIO_PIN_3
#define FLASH_SPI3_CLK_GPIO_Port GPIOB
#define FLASH_SPI3_MISO_Pin GPIO_PIN_4
#define FLASH_SPI3_MISO_GPIO_Port GPIOB
#define FLASH_SPI3_MOSI_Pin GPIO_PIN_5
#define FLASH_SPI3_MOSI_GPIO_Port GPIOB
#define EXT_UART1_TX_Pin GPIO_PIN_6
#define EXT_UART1_TX_GPIO_Port GPIOB
#define EXT_UART1_RX_Pin GPIO_PIN_7
#define EXT_UART1_RX_GPIO_Port GPIOB
#define BLE_CMD1_Pin GPIO_PIN_8
#define BLE_CMD1_GPIO_Port GPIOB
#define BLE_CMD2_Pin GPIO_PIN_9
#define BLE_CMD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
bool KB_ATOMIC_CAS(volatile uint8_t *pdst,uint8_t osrc,uint8_t nsrc);
bool KB_ATOMIC_CAS_ADDR(volatile void **pdst,void * osrc,void * nsrc);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
