#ifndef __TRACE_LOG
#define __TRACE_LOG
#ifdef __cplusplus
extern "C" {
#endif

//#include <stdio.h>
#include "main.h"




#define TL_LEVEL_DEBUG 		0
#define TL_LEVEL_INFO  		1
#define TL_LEVEL_WARNING 	2
#define TL_LEVEL_ERRO 		3
#define TL_LEVEL_MAX		4

#define TL_LEVEL			TL_LEVEL_ERRO



#define LOG_HUARTX huart1



void TL_Init(void);
void TL_TraceLog(uint16_t errlevel,char* log);



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

#define TRACELOG_ERROR(LOG) TL_TraceLog(TL_LEVEL_ERRO,(char*)LOG);



#ifdef __cplusplus
}
#endif

#endif












