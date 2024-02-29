#include "ff.h"
#include "fatfs.h"
#include "diskio.h"
#include "tracelog.h"

#define INTFLASH_ROOT_DIR "1:"

/*******************************************************************

        fatfs interface

*******************************************************************/
static FATFS fs_intflash;												
static FRESULT res_intflash = FR_INT_ERR;               

uint8_t IntFlash_Mount()
{
	res_intflash = f_mount(&fs_intflash,INTFLASH_ROOT_DIR,1);
	if(res_intflash!=FR_OK)
	{
		TRACELOG_ERROR("mount_flash_error");
		return 0;
	}
	else
	{
		TRACELOG_DEBUG("mount intflash ok");	
	}

	return 1;
}



void IntFlash_Unmount()
{
	if(FR_OK == res_intflash)
	{
		f_mount(NULL,INTFLASH_ROOT_DIR,1);
		res_intflash = FR_INT_ERR;
	}
}






