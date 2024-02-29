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
void SerialUpload(uint32_t flashaddr, const uint8_t *p_file_name, uint32_t file_size );

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

  Serial_PutString((uint8_t *)"Waiting for the file to be sent ... (press 'a' to abort)\n\r");
  result = Ymodem_Receive(flashaddr ,&size );
  if (result == COM_OK)
  {
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

/**
  * @brief  Upload a file via serial port.
  * @param  None
  * @retval None
  */
void SerialUpload(uint32_t flashaddr, const uint8_t *p_file_name, uint32_t file_size )
{
  uint8_t status = 0;

  Serial_PutString((uint8_t *)"\n\n\rSelect Receive File\n\r");

  HAL_UART_Receive(&UartHandle, &status, 1, RX_TIMEOUT);
  if ( status == CRC16)
  {
    /* Transmit the flash image through ymodem protocol */
    status = Ymodem_Transmit(flashaddr, p_file_name ,file_size);

    if (status != 0)
    {
      Serial_PutString((uint8_t *)"\n\rError Occurred while Transmitting File\n\r");
    }
    else
    {
      Serial_PutString((uint8_t *)"\n\rFile uploaded successfully \n\r");
    }
  }
}


void Download_Fonts()
{
	uint32_t size = 0;
	SerialDownload(0,&size);


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
  Serial_PutString((uint8_t *)"\r\n=              (C) COPYRIGHT 2016 STMicroelectronics                 =");
  Serial_PutString((uint8_t *)"\r\n=                                                                    =");
  Serial_PutString((uint8_t *)"\r\n=  STM32F1xx In-Application Programming Application  (Version 1.0.0) =");
  Serial_PutString((uint8_t *)"\r\n=                                                                    =");
  Serial_PutString((uint8_t *)"\r\n=                                   By MCD Application Team          =");
  Serial_PutString((uint8_t *)"\r\n======================================================================");
  Serial_PutString((uint8_t *)"\r\n\r\n");

  while (1)
  {

    Serial_PutString((uint8_t *)"\r\n=================== Main Menu ============================\r\n\n");
    Serial_PutString((uint8_t *)"  Download font to the  NorFlash ----------------------------- 1\r\n\n");
    Serial_PutString((uint8_t *)"  Check font result ------------------------------------------ 2\r\n\n");
	Serial_PutString((uint8_t *)"  Print 8 to exit -------------------------------------------- 8\r\n\n");
    Serial_PutString((uint8_t *)"  Print error statistics ------------------------------------- 9\r\n\n");

    /* Clean the input path */
    __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
		
    /* Receive key */
    HAL_UART_Receive(&UartHandle, &key, 1, RX_TIMEOUT);

    switch (key)
    {
    case '1' :
      /* Download user application in the Flash */
      Download_Fonts();
      break;
    case '2' :
      /* Upload user application from the Flash */

      break;
	case '8' :
      /* Upload user application from the Flash */
		Serial_PutString((uint8_t *)"\r\n=============================Exit=====================================");
		return;
	 case '9' :
      /* Upload user application from the Flash */
		Print_Error();
      break;
	default:
	Serial_PutString((uint8_t *)"Invalid Number ! ==> The number should be either 1, 2\r");
	break;
    }
  }
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
