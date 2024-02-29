#ifndef _BSP_SPI_FLASH_H_
#define _BSP_SPI_FLASH_H_

#include "stm32f1xx.h"
#include "stm32f1xx_hal_spi.h"

#include "stm32f1xx_hal.h"



#define W25Q128	0XEF17

#define  sFLASH_ID 0XEF16
extern SPI_HandleTypeDef hspi3;
extern uint16_t W25QXX_TYPE;			
#define W25QXX_CS_1		  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET)
#define W25QXX_CS_0	      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_RESET)
//////////////////////////////////////////////////////////////////////////////////

#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg1		0x05
#define W25X_ReadStatusReg2		0x35
#define W25X_ReadStatusReg3		0x15
#define W25X_WriteStatusReg1    0x01
#define W25X_WriteStatusReg2    0x31
#define W25X_WriteStatusReg3    0x11
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown	0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID	0x90
#define W25X_JedecDeviceID		0x9F
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9

void W25QXX_Init(void);
uint16_t  W25QXX_ReadID(void);  	
void W25QXX_WAKEUP(void);				
void W25QXX_Erase_Chip(void);
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
#endif /* __SPI_FLASH_H */

