/**
  ******************************************************************************
  * @file    IAP_Main/Src/ymodem.c 
  * @author  MCD Application Team
  * @brief   This file provides all the software functions related to the ymodem 
  *          protocol.
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
#include "common.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"
#include "menu.h"
#include "memery.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* activate the CRC16 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit alligned */
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];

/* Private function prototypes -----------------------------------------------*/                                                                       
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout)
{
  uint32_t crc;
  uint32_t packet_size = 0;
  HAL_StatusTypeDef status;
  uint8_t char1;

  *p_length = 0;
  status = HAL_UART_Receive(&UartHandle, &char1, 1, timeout);

  if (status == HAL_OK)
  {
    switch (char1)
    {
      case SOH:
        packet_size = PACKET_SIZE;
        break;
      case STX:
        packet_size = PACKET_1K_SIZE;
        break;
      case EOT:
        break;
      case CA:
        if ((HAL_UART_Receive(&UartHandle, &char1, 1, timeout) == HAL_OK) && (char1 == CA))
        {
          packet_size = 2;
        }
        else
        {
          status = HAL_ERROR;
        }
        break;
      case ABORT1:
      case ABORT2:
        status = HAL_BUSY;
        break;
      default:
        status = HAL_ERROR;
        break;
    }
    *p_data = char1;

    if (packet_size >= PACKET_SIZE )
    {
      status = HAL_UART_Receive(&UartHandle, &p_data[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, timeout);

      /* Simple packet sanity check */
      if (status == HAL_OK )
      {
        if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
        {
          packet_size = 0;
          status = HAL_ERROR;
        }
        else
        {
          /* Check packet CRC */
          crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
          crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
          if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc )
          {
            packet_size = 0;
            status = HAL_ERROR;
          }
        }
      }
      else
      {
        packet_size = 0;
      }
    }
  }
  *p_length = packet_size;
  return status;
}
/**
  * @brief  Update CRC16 for input byte
  * @param  crc_in input value 
  * @param  input byte
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;

  do
  {
    crc <<= 1;
    in <<= 1;
    if(in & 0x100)
      ++crc;
    if(crc & 0x10000)
      crc ^= 0x1021;
  }
  
  while(!(in & 0x10000));

  return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t* dataEnd = p_data+size;

  while(p_data < dataEnd)
    crc = UpdateCRC16(crc, *p_data++);
 
  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);

  return crc&0xffffu;
}


static void flash_write_erase(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
    uint16_t i;

	HAL_FLASH_Unlock(); 
    for (i = 0; i < length; i++)
    {
    	if(0 == (waddr%STM32_SECTOR_SIZE))
		{
			HAL_FLASH_Lock(); /* ��??? */
			flash_erase(waddr);
			HAL_FLASH_Unlock(); 
		}
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, waddr, pbuf[i]);
        waddr += 2; /* ???��??��???��?��? */
    }
	HAL_FLASH_Lock(); /* ��??? */

}

static void Flash_Write_Erase(uint32_t waddr, uint8_t *pbuf, uint16_t length)
{
	uint16_t value;
	
	flash_write_erase(waddr,(uint16_t*)pbuf,(length)>>1);
	if(length & 0x0001)
	{
		value = *((volatile uint16_t *)(waddr + length-1));		
		value = (value&~0xff00)|((uint16_t)pbuf[length - 1]);
		flash_write_erase(waddr + length-1 , &value,1);
	}

}

static uint8_t Flash_CheckResult(uint32_t waddr, uint8_t *pbuf, uint16_t length)
{
	uint16_t i;

	for(i=0;i<length;i++)
	{
		uint8_t value = *((volatile uint8_t *)(waddr + i));
		if(value != pbuf[i])
		{
			return false;
		}
	}
	return true;

}


/**
  * @brief  Calculate Check sum for YModem Packet
  * @param  p_data Pointer to input data
  * @param  size length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *p_data_end = p_data + size;

  while (p_data < p_data_end )
  {
    sum += *p_data++;
  }

  return (sum & 0xffu);
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @retval COM_StatusTypeDef result of reception/programming
  */
COM_StatusTypeDef Ymodem_Receive (uint32_t flashaddr,uint32_t *p_size,uint8_t checkresult)
{
	uint32_t i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0;
	uint32_t flashdestination, filesize;
	uint8_t *file_ptr;
	uint8_t file_size[FILE_SIZE_LENGTH], tmp, packets_received;
	COM_StatusTypeDef result = COM_OK;

	/* Initialize flashdestination variable */
	flashdestination = flashaddr;

	while ((session_done == 0) && (result == COM_OK))
	{
		packets_received = 0;
		file_done = 0;
		while ((file_done == 0) && (result == COM_OK))
		{
			switch (ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT))
			{
				case HAL_OK:
				  	errors = 0;
				  	switch (packet_length)
					{
						case 2:
							/* Abort by sender */
							Serial_PutByte(ACK);
							result = COM_ABORT;
							break;
						case 0:
							/* End of transmission */
							Serial_PutByte(ACK);
							file_done = 1;
							break;
						default:
							/* Normal packet */
							if (aPacketData[PACKET_NUMBER_INDEX] != packets_received)
							{
								Serial_PutByte(NAK);
							}
						  	else
							{
								if (packets_received == 0)
								{
									/* File name packet */
									if (aPacketData[PACKET_DATA_INDEX] != 0)
									{
										/* File name extraction */
										i = 0;
										file_ptr = aPacketData + PACKET_DATA_INDEX;
										while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH))
										{
										  aFileName[i++] = *file_ptr++;
										}

										/* File size extraction */
										aFileName[i++] = '\0';
										i = 0;
										file_ptr ++;
										while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH))
										{
										  	file_size[i++] = *file_ptr++;
										}
										file_size[i++] = '\0';
										Str2Int(file_size, &filesize);

										/* Test the size of the image to be sent */
										/* Image size is greater than Flash size */
										if (*p_size > (USER_FLASH_SIZE + 1))
										{
											/* End session */
											tmp = CA;
											HAL_UART_Transmit(&UartHandle, &tmp, 1, NAK_TIMEOUT);
											HAL_UART_Transmit(&UartHandle, &tmp, 1, NAK_TIMEOUT);
											result = COM_LIMIT;
										}
										
										*p_size = filesize;

										Serial_PutByte(ACK);
										Serial_PutByte(CRC16);
									}
									/* File header packet is empty, end session */
									else
									{
										Serial_PutByte(ACK);
										file_done = 1;
										session_done = 1;
										break;
									}
								}
								else /* Data packet */
								{
									/* Write received data in Flash */
									//if (FLASH_If_Write(flashdestination, (uint32_t*) ramsource, packet_length/4) == FLASHIF_OK) 
									//if (Flash_Write(flashdestination, packet_length, &aPacketData[PACKET_DATA_INDEX])) 
									if(checkresult)
									{
										if(false == Flash_CheckResult(flashdestination, &aPacketData[PACKET_DATA_INDEX],packet_length))
										{
											/* End session */
											Serial_PutByte(CA);
											Serial_PutByte(CA);
											return COM_DATA;
										}
									}
									else
									{
										Flash_Write_Erase(flashdestination, &aPacketData[PACKET_DATA_INDEX],packet_length);

									}
									//if()
									{
										flashdestination += packet_length;
										Serial_PutByte(ACK);	
									}
									//else /* An error occurred while writing to Flash memory */
									{
										/* End session */
										//Serial_PutByte(CA);
										//Serial_PutByte(CA);
										//result = COM_DATA;
										//XMODE_ERR_PLUS(XMODE_DOWN_ERR_WRITE_ERR);
									}
									//Flash_Read(flashdestination, packet_length, &aPacketData[PACKET_DATA_INDEX]);
								}
								packets_received ++;
								session_begin = 1;
							}
							break;
					}
				  	break;
				case HAL_BUSY: /* Abort actually */
					Serial_PutByte(CA);
					Serial_PutByte(CA);
					result = COM_ABORT;
					break;
				default:		
					if (session_begin > 0)
					{
						errors ++;
					}
					if (errors > MAX_ERRORS)
					{
						/* Abort communication */
						Serial_PutByte(CA);
						Serial_PutByte(CA);
					}
					else
					{
						Serial_PutByte(CRC16); /* Ask for a packet */
					}
					break;
			}
		}
	}
  return result;
}


/*******************(C)COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
