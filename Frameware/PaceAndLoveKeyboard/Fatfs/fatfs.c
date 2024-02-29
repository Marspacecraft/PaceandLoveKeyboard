#include "ff.h"
#include "fatfs.h"
#include "diskio.h"
#include "tracelog.h"

#define FLASH_ROOT_DIR "1:"
#define INTFLASH_ROOT_DIR "2:"

/*******************************************************************

        fatfs interface

*******************************************************************/
static FATFS fs_flash;												
static FRESULT res_flash = FR_INT_ERR;               
static BYTE work_flash[FF_MAX_SS];

void Flash_Mount()
{
	res_flash = f_mount(&fs_flash,FLASH_ROOT_DIR,1);
	if(res_flash!=FR_OK)
	{
		TRACELOG_DEBUG_PRINTF("mount flash error,ret[%d]",res_flash);
		if(res_flash == FR_NO_FILESYSTEM)
		{
			TRACELOG_ERROR(mount_flash_mkfs);
			res_flash=f_mkfs(FLASH_ROOT_DIR, 0, work_flash, sizeof(work_flash));						
		
			if(res_flash == FR_OK)
			{
				TRACELOG_DEBUG_PRINTF("mkfs success");
				res_flash = f_mount(NULL,FLASH_ROOT_DIR,1);					
				res_flash = f_mount(&fs_flash,FLASH_ROOT_DIR,1);
			}
			else
			{
				TRACELOG_ERROR(mount_flash_mkfs_error);
				return;
			}
		}
		else
		{
			TRACELOG_ERROR(mount_flash_error);
		}
	}
	else
	{
		TRACELOG_DEBUG_PRINTF("mount flash ok");	
	}

	//f_mount(NULL,"1:",1);

}



void Flash_Unmount()
{
	if(FR_OK == res_flash)
	{
		f_mount(NULL,FLASH_ROOT_DIR,1);
		res_flash = FR_INT_ERR;
	}
}

static FATFS fs_intflash;												
static FRESULT res_intflash = FR_INT_ERR;               
static BYTE work_intflash[FF_MAX_SS];

void IntFlash_Mount()
{
	if(FR_OK == res_intflash)
	{
		return;
	}
	res_intflash = f_mount(&fs_intflash,INTFLASH_ROOT_DIR,1);
	if(res_intflash!=FR_OK)
	{
		TRACELOG_DEBUG_PRINTF("mount intflash error,ret[%d]",res_intflash);
		if(res_intflash == FR_NO_FILESYSTEM)
		{
			TRACELOG_ERROR(mount_flash_mkfs);
			res_intflash=f_mkfs(INTFLASH_ROOT_DIR, 0, work_intflash, sizeof(work_intflash));						
		
			if(res_intflash == FR_OK)
			{
				TRACELOG_DEBUG_PRINTF("mkfs success");
				res_intflash = f_mount(NULL,INTFLASH_ROOT_DIR,1);					
				res_intflash = f_mount(&fs_intflash,INTFLASH_ROOT_DIR,1);
			}
			else
			{
				TRACELOG_ERROR(mount_flash_mkfs_error);
				return;
			}
		}
		else
		{
			TRACELOG_ERROR(mount_flash_error);
		}
	}
	else
	{
		TRACELOG_DEBUG_PRINTF("mount intflash ok");	
	}

	//f_mount(NULL,"1:",1);

}



void IntFlash_Unmount()
{
	if(FR_OK == res_intflash)
	{
		f_mount(NULL,INTFLASH_ROOT_DIR,1);
		res_intflash = FR_INT_ERR;
	}
}


uint8_t IntFlash_Open(FIL*fp,uint8_t*file)
{
	FRESULT f_res;

	IntFlash_Mount();
	if(FR_OK != res_intflash)
	{
		return false;
	}
	
	f_res = f_open(fp,(const TCHAR*)file,FA_OPEN_EXISTING|FA_READ);
	if(FR_OK != f_res)
	{
		TRACELOG_ERROR(intflash_open_error);
		return false;
	}
	return true;
}

void IntFlash_Close(FIL*fp)
{
	if(NULL == fp || FR_OK != res_intflash)
		return;

	f_close(fp);
}

void IntFlash_RmFile(uint8_t*file)
{
	if(FR_OK != res_intflash)
		return;

	f_rmdir((const TCHAR*)file);
}





#ifdef TEST_FATFS

#include "ff.h"
#include "tracelog.h"


void testcase_mount()
{
	TRACELOG_DEBUG("\ttestcase_mount");
	TRACELOG_DEBUG("\t\tmount flash");
	Flash_Mount();
	Flash_Unmount();
	TRACELOG_DEBUG("\t\tmount inflash");
	IntFlash_Mount();
	IntFlash_Unmount();
	TRACELOG_DEBUG("\ttestcase_mount end");
}

void testcase_fatfs()
{
	TRACELOG_DEBUG("Running testcase_fatfs...");
	testcase_mount();
	TRACELOG_DEBUG("testcase_fatfs end...");
}

 #endif  
 






