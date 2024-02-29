#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f1xx.h"
#include <stdio.h>
#include "spi.h"
#include "bsp_spi_flash.h"


#define FLASH_SIZE      	16*1024//KB
#define FLASH_SECTOR_SIZE  	4//KB
#define FLASH_NOFS_SIZE   	6*1024//KB
#define FLASH_FS_SIZE      (FLASH_SIZE - FLASH_NOFS_SIZE)//KB

#define FS_STARTADD_OFFSET (FLASH_NOFS_SIZE/FLASH_SECTOR_SIZE)
#define FS_SECTOR_NUM  (FLASH_FS_SIZE/FLASH_SECTOR_SIZE)


void Flash_Init(void);
uint8_t Flash_SPI_Valid(void);

#define Flash_EraseChip 					W25QXX_Erase_Chip
#define Flash_EraseSecotor(sector)			W25QXX_Erase_Sector((sector))

#define Flash_Read(addr,size,data) 			W25QXX_Read((data),(addr),(size))
#define Flash_Write(addr,size,data)  		W25QXX_Write_NoCheck((data),(addr),(size))
#define Flash_Write_Erz(addr,size,data) 	W25QXX_Write((data),(addr),(size))

#define Flash_PowerDown						W25QXX_PowerDown
#define Flash_WakeUp 						W25QXX_WAKEUP	

#define Flash_WriteEnable					W25QXX_Write_Enable
#define Flash_WriteDisable					W25QXX_Write_Disable


#ifdef TEST_NORFLASH
	void testcase_norflash(void);
#endif


#endif /* __SPI_FLASH_H */

