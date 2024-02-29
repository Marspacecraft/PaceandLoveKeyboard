/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
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
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#define EXT_SPI_HANDLER 	hspi2
#define EXT_SPIX			SPI2
#define FLASH_SPI_HANDLER 	hspi3
#define FLASH_SPIX			SPI3


//typedef void (*EXT_SPI_DMA_CALLBACK)(void);
typedef void (*EXT_CSFUNC)(bool);

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi1;

extern SPI_HandleTypeDef hspi2;

extern SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_SPI1_Init(void);
void MX_SPI2_Init(void);
void MX_SPI3_Init(void);

/* USER CODE BEGIN Prototypes */

// csfunc used to enable or disable the extent spi device. csfunc(true) is enale device.
//Synchronous communacation,return with data
bool EXT_SPI_OccupyBus(EXT_CSFUNC csfunc);
void EXT_SPI_FreeBus(EXT_CSFUNC csfunc);


HAL_StatusTypeDef EXT_SPI_read_write(uint8_t *pRxData, uint8_t *pTxData,uint16_t Size,EXT_CSFUNC csfunc);
HAL_StatusTypeDef EXT_SPI_read(uint8_t *pRxData,uint16_t Size,EXT_CSFUNC csfunc);
HAL_StatusTypeDef EXT_SPI_write(uint8_t *pTxData,uint16_t Size,EXT_CSFUNC csfunc);

HAL_StatusTypeDef EXT_SPI_read_write_byte(uint8_t *pRxData, uint8_t TxData,EXT_CSFUNC csfunc);
HAL_StatusTypeDef EXT_SPI_read_byte(uint8_t *pRxData,EXT_CSFUNC csfunc);
HAL_StatusTypeDef EXT_SPI_write_byte(uint8_t TxData,EXT_CSFUNC csfunc);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

