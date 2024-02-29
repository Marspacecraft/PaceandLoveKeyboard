/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "tracelog.h"
#include "memery.h"


/* 为每个设备定义一个物理编号 */
#define ATA			    0     // 预留SD卡使用
#define INT_FLASH		1     // 外部SPI Flash

#define SMC_STORAGE_BLK_NBR              	(INTFLASH_MSC_SIZE/STM32_SECTOR_SIZE)*4
#define SMC_STORAGE_BLK_SIZ           		STM32_SECTOR_SIZE/4


/*-----------------------------------------------------------------------*/
/* 获取设备状态                                                          */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* 物理编号 */
)
{

	DSTATUS status = STA_NOINIT;
	
	switch (pdrv) {
		case ATA:	/* SD CARD */
			break;
		case INT_FLASH:
			status &= ~STA_NOINIT;
			break;
		default:
			status = STA_NOINIT;
	}
	return status;
}




/*-----------------------------------------------------------------------*/
/* 设备初始化                                                            */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* 物理编号 */
)
{
	DSTATUS status = STA_NOINIT;	
	switch (pdrv) {
		case ATA:	         /* SD CARD */
			break;
		case INT_FLASH: 
      		status=disk_status(pdrv);
			break;
		default:
			status = STA_NOINIT;
	}
	return status;
}



/*-----------------------------------------------------------------------*/
/* 读扇区：读取扇区内容到指定存储区                                              */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* 设备物理编号(0..) */
	BYTE *buff,		/* 数据缓存区 */
	DWORD sector,	/* 扇区首地址 */
	UINT count		/* 扇区个数(1..128) */
)
{
	DRESULT status = RES_PARERR;
	switch (pdrv) 
	{
		case ATA:	/* SD CARD */
			break;
		case INT_FLASH:
			if(IntFlash_MSC_Read(sector*SMC_STORAGE_BLK_SIZ, count*SMC_STORAGE_BLK_SIZ, buff))
			{
				status = RES_OK;
			}
			break;
		default:
			status = RES_PARERR;
	}
	return status;
}



/*-----------------------------------------------------------------------*/
/* 写扇区：见数据写入指定扇区空间上                                      */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			  /* 设备物理编号(0..) */
	const BYTE *buff,	/* 欲写入数据的缓存区 */
	DWORD sector,		  /* 扇区首地址 */
	UINT count			  /* 扇区个数(1..128) */
)
{
	DRESULT status = RES_PARERR;
	if (!count) 
	{
		return RES_PARERR;		/* Check parameter */
	}

	switch (pdrv) 
	{
		case ATA:	/* SD CARD */      
			break;
		case INT_FLASH:
			if(IntFlash_MSC_Write(sector*SMC_STORAGE_BLK_SIZ, count*SMC_STORAGE_BLK_SIZ,(uint8_t *) buff))
			{
				status = RES_OK;
			}
			break;
		default:
			status = RES_PARERR;
	}
	return status;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 */
	void *buff		/* 写入或者读取数据地址指针 */
)
{
	DRESULT status = RES_PARERR;
	switch (pdrv) 
	{
		case ATA:	/* SD CARD */
			break;
    	case INT_FLASH:
			switch (cmd) 
			{
		        case GET_SECTOR_COUNT:
		          *(DWORD * )buff = SMC_STORAGE_BLK_NBR;	//128KB
		        break;
		        /* 扇区大小  */
		        case GET_SECTOR_SIZE :
		          *(WORD * )buff = SMC_STORAGE_BLK_SIZ;
		        break;
		        /* 同时擦除扇区个数 */
		        case GET_BLOCK_SIZE :
		          *(DWORD * )buff = 1;
		        break;        
      		}
      		status = RES_OK;
		
			break;
		default:
			status = RES_PARERR;
	}
	return status;
}

DWORD get_fattime (void)
{
  /* 在这里添加实际的获取时间的方法... */
  
  
  /* 对应的要修改返回值 */
  return ((2021-1980)<<25) | ((1)<<21) | ((1)<<16) | ((1)<<11) | ((1)<<5) | ((1)<<0);
}



