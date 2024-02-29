
#ifndef _FLASH_MEMORY_H_
#define _FLASH_MEMORY_H_

#include "main.h"

#ifndef bool
typedef uint8_t bool;
#endif
#ifndef false
#define false (0)
#endif
#ifndef true
#define true (1)
#endif
/********************************************************************************

	|<------STM32_FLASH_START 0x08000000
	|			
	|		SIZE 256KB	code area. 
	|
	|<------INTFLASH_ADDR_START INTFLASH_MEM_ADDR_START
	|
	|		SIZE 128KB	mem area. config region.
	|
	|<------INTFLASH_MEM_ADDR_END INTFLASH_MSC_ADDR_START
	|
	|		SIZE 128KB 	msc area. bootloader.
	|
	|<------INTFLASH_MSC_ADDR_END INTFLASH_ADDR_END

********************************************************************************/
#define APP_OFFSET 	0x4000//16KB 4KB整数倍
#define APP_ADDR	(FLASH_BASE + APP_OFFSET)


#define STM32_FLASH_SIZE       	 0x80000         /* 512KB */
#define FRAME_SIZE       		 0x10000         	/* 64KB */
#define STM32_FLASH_START        FLASH_BASE         /* 256KB */

#if STM32_FLASH_SIZE < (256 * 1024)
#define STM32_SECTOR_SIZE   1024             
#else
#define STM32_SECTOR_SIZE   2048              
#endif

#if FRAME_SIZE < (STM32_FLASH_SIZE/2)
#define INTFLASH_ADDR_START		(STM32_FLASH_START + (STM32_FLASH_SIZE/2))
#else
#define INTFLASH_ADDR_START		(STM32_FLASH_START + (((FRAME_SIZE)>>1)<<1) + 2)
#endif
#define INTFLASH_ADDR_END		(STM32_FLASH_START + STM32_FLASH_SIZE)

#define INTFLASH_SIZE			(INTFLASH_ADDR_END - INTFLASH_ADDR_START)

#define INTFLASH_MSC_SIZE 		(INTFLASH_SIZE/2)

#define INTFLASH_MEM_SIZE 		(INTFLASH_SIZE - INTFLASH_MSC_SIZE)


#define INTFLASH_MEM_ADDR_START 	INTFLASH_ADDR_START
#define INTFLASH_MEM_ADDR_END 		(INTFLASH_MEM_ADDR_START + INTFLASH_MEM_SIZE)


#define INTFLASH_MSC_ADDR_START 	INTFLASH_MEM_ADDR_END
#define INTFLASH_MSC_ADDR_END 		(INTFLASH_MSC_ADDR_START + INTFLASH_MSC_SIZE)



#define OLD_FRAMEWARE_ADDR			(STM32_FLASH_START+(STM32_FLASH_SIZE/4))//地址4k的倍数
#define FRAMEWARE_SIZE_MAX			(100*1024)


void IntFlash_Init(void);


/************************************************

	IntFlash config 配置区，总大小128KB

************************************************/

#define CONFIG_ADDR_ALIGN4(ADDR)		((((ADDR)>>2)<<2)+4)

#define CONFIG_FLAGBIT_ADDR		0
#define CONFIG_FLAGBIT_SIZE		4
	#define MEM_FLAG_SYSOFF		(0x00000001<<0)	//if systemoff,disable watchdog after restart
	#define MEM_FLAG_JOYSTICK	(0x00000001<<1)	//start with usb msc mode
	#define MEM_FLAG_USBMSC		(0x00000001<<2)	//start with usb msc mode
	#define MEM_FLAG_USBMSCEND	(0x00000001<<3)	//start with usb msc mode
	#define MEM_FLAG_BOOTLOADER	(0x00000001<<4)	//start with usb msc mode

#define CONFIG_JOYSTICK_ADDR		CONFIG_ADDR_ALIGN4(CONFIG_FLAGBIT_ADDR+CONFIG_FLAGBIT_SIZE)
#define CONFIG_JOYSTICK_SIZE		12

#define CONFIG_KEYLAYER_ADDR		CONFIG_ADDR_ALIGN4(CONFIG_JOYSTICK_ADDR+CONFIG_JOYSTICK_SIZE)
#define CONFIG_KEYLAYER_SIZE		4096

#define CONFIG_FONTMSG_ADDR			CONFIG_ADDR_ALIGN4(CONFIG_KEYLAYER_ADDR+CONFIG_KEYLAYER_SIZE)
#define CONFIG_FONTMSG_ADDR_SIZE	256

#define CONFIG_FONTS_ADDR			CONFIG_ADDR_ALIGN4(CONFIG_FONTMSG_ADDR+CONFIG_FONTMSG_ADDR_SIZE)
#define CONFIG_FONTS_ADDR_SIZE		16384




uint32_t flash_read_word(uint32_t raddr);
void flash_write_word(uint32_t waddr, uint32_t word);
void flash_erase(uint32_t eaddr);

bool IntFlash_MSC_Write(uint32_t addr, uint16_t size, uint8_t *data);
bool IntFlash_MSC_Read(uint32_t addr, uint16_t size, uint8_t *data);

bool MEM_FlagBit_IsEnabled(uint32_t flagbit);
void MEM_FlagBit_Reset(uint32_t flagbit);



#endif







