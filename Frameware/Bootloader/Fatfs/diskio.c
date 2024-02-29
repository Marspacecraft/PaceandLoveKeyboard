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


/* Ϊÿ���豸����һ�������� */
#define ATA			    0     // Ԥ��SD��ʹ��
#define INT_FLASH		1     // �ⲿSPI Flash

#define SMC_STORAGE_BLK_NBR              	(INTFLASH_MSC_SIZE/STM32_SECTOR_SIZE)*4
#define SMC_STORAGE_BLK_SIZ           		STM32_SECTOR_SIZE/4


/*-----------------------------------------------------------------------*/
/* ��ȡ�豸״̬                                                          */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* ������ */
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
/* �豸��ʼ��                                                            */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* ������ */
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
/* ����������ȡ�������ݵ�ָ���洢��                                              */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* �豸������(0..) */
	BYTE *buff,		/* ���ݻ����� */
	DWORD sector,	/* �����׵�ַ */
	UINT count		/* ��������(1..128) */
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
/* д������������д��ָ�������ռ���                                      */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			  /* �豸������(0..) */
	const BYTE *buff,	/* ��д�����ݵĻ����� */
	DWORD sector,		  /* �����׵�ַ */
	UINT count			  /* ��������(1..128) */
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
	BYTE pdrv,		/* ������ */
	BYTE cmd,		  /* ����ָ�� */
	void *buff		/* д����߶�ȡ���ݵ�ַָ�� */
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
		        /* ������С  */
		        case GET_SECTOR_SIZE :
		          *(WORD * )buff = SMC_STORAGE_BLK_SIZ;
		        break;
		        /* ͬʱ������������ */
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
  /* ���������ʵ�ʵĻ�ȡʱ��ķ���... */
  
  
  /* ��Ӧ��Ҫ�޸ķ���ֵ */
  return ((2021-1980)<<25) | ((1)<<21) | ((1)<<16) | ((1)<<11) | ((1)<<5) | ((1)<<0);
}



