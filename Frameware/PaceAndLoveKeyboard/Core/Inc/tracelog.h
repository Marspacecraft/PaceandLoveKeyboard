#ifndef __TRACE_LOG
#define __TRACE_LOG
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "keyconfig.h"
#include "main.h"

typedef enum
{
	TL_ERR_NONE = TL_LEVEL_ERRO,

	unknow_error,
	unknow_error_HardFault,
	unknow_error_NonMaskableInt,
	unknow_error_MemoryManagement,
	unknow_error_BusFault,
	unknow_error_UsageFault,
	unknow_error_Handler,
	
	norflash_init_error,
	mount_flash_mkfs,
	mount_flash_mkfs_error,
	mount_flash_error,
	intflash_open_error,
	
	usb_send_error,

	
	
	TL_ERR_MAX,
}eTLError;


#define LOG_HUARTX huart1

#ifdef USE_TRACELOG

void TL_Init(void);
void TL_TraceLog(uint16_t errlevel,char* log);

#else


#define TL_Init() {}
#define TL_TraceLog(errlevel,log) {}


#endif

#if (TL_LEVEL <= TL_LEVEL_DEBUG)
#define TRACELOG_DEBUG(LOG) TL_TraceLog(TL_LEVEL_DEBUG,(char*)LOG);
#else
#define TRACELOG_DEBUG(LOG) {}
#endif

#if (TL_LEVEL <= TL_LEVEL_INFO)
#define TRACELOG_INFO(LOG) TL_TraceLog(TL_LEVEL_INFO,(char*)LOG);
#else
#define TRACELOG_INFO(LOG) {}
#endif

#if (TL_LEVEL <= TL_LEVEL_WARNING)
#define TRACELOG_WARNING(LOG) TL_TraceLog(TL_LEVEL_WARNING,(char*)LOG);
#else
#define TRACELOG_WARNING(LOG) {}
#endif

#define TRACELOG_ERROR(ERROR) TL_TraceLog(ERROR,#ERROR);




#define TRACELOG_DEBUG_PRINTF(...) \
do\
{\
	char buffer[128];\
	sprintf((char *)buffer,__VA_ARGS__);\
	TRACELOG_DEBUG(buffer);\
}while(0)



#ifdef __cplusplus
}
#endif

#endif












