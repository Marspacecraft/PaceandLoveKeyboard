/**
  ******************************************************************************
  * @file    IAP_Main/Src/menu.c 
  * @author  MCD Application Team
  * @brief   This file provides the software which contains the main menu routine.
  *          The main menu gives the options of:
  *             - downloading a new binary file, 
  *             - uploading internal flash memory,
  *             - executing the binary file already loaded 
  *             - configuring the write protection of the Flash sectors where the 
  *               user loads his binary file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/** @addtogroup STM32F1xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "common.h"
#include "menu.h"
#include "ymodem.h"
#include "memery.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction JumpToApplication;
uint32_t JumpAddress;
uint32_t FlashProtection = 0;
uint8_t aFileName[FILE_NAME_LENGTH];


/* Private function prototypes -----------------------------------------------*/
COM_StatusTypeDef SerialDownload(uint32_t flashaddr,uint32_t *psize);
void LoadOldFirmware(void);
void SaveCurrentFirmware(void);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Download a file via serial port
  * @param  None
  * @retval None
  */
COM_StatusTypeDef SerialDownload(uint32_t flashaddr,uint32_t *psize)
{
  uint8_t number[11] = {0};
  uint32_t size = 0;
  COM_StatusTypeDef result;
  //Serial_PutString((uint8_t *)"\r\n== First transmit to program flash.Second transmit to check the flash! ==\r\n");

  Serial_PutString((uint8_t *)"Waiting for the file to be sent ... (press 'a' to abort)\n\r");
  result = Ymodem_Receive(flashaddr ,&size ,0);
  if (result == COM_OK)
  {
  	//Serial_PutString((uint8_t *)"\n\n\r Retransmit to check the result!\n\r\r\n");
	//result = Ymodem_Receive(flashaddr ,&size ,1);
    Serial_PutString((uint8_t *)"\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
    Serial_PutString(aFileName);
    Int2Str(number, size);
    Serial_PutString((uint8_t *)"\n\r Size: ");
    Serial_PutString(number);
    Serial_PutString((uint8_t *)" Bytes\r\n");
    Serial_PutString((uint8_t *)"-------------------\n");
  }
  else if (result == COM_LIMIT)
  {
    Serial_PutString((uint8_t *)"\n\n\rThe image size is higher than the allowed space memory!\n\r");
  }
  else if (result == COM_DATA)
  {
    Serial_PutString((uint8_t *)"\n\n\rVerification failed!\n\r");
  }
  else if (result == COM_ABORT)
  {
    Serial_PutString((uint8_t *)"\r\n\nAborted by user.\n\r");
  }
  else
  {
    Serial_PutString((uint8_t *)"\n\rFailed to receive the file!\n\r");
  }
  *psize = size;
  return result;
}



void Download_Firmware()
{
	uint32_t size = 0;
	
	if(COM_OK != SerialDownload(APP_ADDR,&size))
	{
		Serial_PutString((uint8_t *)"\r\nDownload error!\r\n");
	}
	else
	{
		Serial_PutString((uint8_t *)"\r\nDownload success!\r\n");
	}

}

extern void Print_Error(void);

/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(void)
{
	uint8_t key = 0;

	Serial_PutString((uint8_t *)"\r\n======================================================================");
	Serial_PutString((uint8_t *)"\r\n=              Pace and Love Keyboarder Bootloader                   =");
	Serial_PutString((uint8_t *)"\r\n=                                                                    =");
	Serial_PutString((uint8_t *)"\r\n=                                          Bootloader(Version 1.0.0) =");
	Serial_PutString((uint8_t *)"\r\n=                                                                    =");
	Serial_PutString((uint8_t *)"\r\n=                                          By Marspacecraft          =");
	Serial_PutString((uint8_t *)"\r\n======================================================================");
	Serial_PutString((uint8_t *)"\r\n\r\n");

	while (1)
	{

		Serial_PutString((uint8_t *)"\r\n============================ Main Menu ============================\r\n\n");
		Serial_PutString((uint8_t *)"  Download firmware ------------------------------------------ 1\r\n\n");
		Serial_PutString((uint8_t *)"  Save current firmware -------------------------------------- 2\r\n\n");
		Serial_PutString((uint8_t *)"  Load old firmware ------------------------------------------ 3\r\n\n");
		Serial_PutString((uint8_t *)"  Exit ------------------------------------------------------- 4\r\n\n");

		/* Clean the input path */
		__HAL_UART_FLUSH_DRREGISTER(&UartHandle);
			
		/* Receive key */
		HAL_UART_Receive(&UartHandle, &key, 1, RX_TIMEOUT);

		switch (key)
		{
		    case '1' :
		      	Download_Firmware();
		      	break;
			case '2' :
				Serial_PutString((uint8_t *)"\r\nSaving...\r\n");
		      	SaveCurrentFirmware();
				Serial_PutString((uint8_t *)"\r\nSave current firmware success!\r\n");
		      	break;
		    case '3' :
				Serial_PutString((uint8_t *)"\r\nLoading...\r\n");
				LoadOldFirmware();
				Serial_PutString((uint8_t *)"\r\nLoad old firmware success!\r\n");
		      break;
			case '4' :
				Serial_PutString((uint8_t *)"\r\n=============================Exit=====================================");
				return;
			default:
				Serial_PutString((uint8_t *)"Invalid Number ! ==> The number should be either 1, 2, 3, 4\r");
				break;
		}
	}
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
