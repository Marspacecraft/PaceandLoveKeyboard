#include "tracelog.h"
#ifdef USE_TRACELOG


#include <string.h>
#include "main.h"
static uint16_t sg_ErrLogStat[TL_ERR_MAX] = {0};


#ifdef USE_TRACELOG_EXTUART
#include "usart.h"

void TL_Init()
{
}

void TL_TraceLog(uint16_t errlevel,char* log)
{
	if((errlevel < (TL_LEVEL))||(errlevel >= (TL_ERR_MAX)))	
		return ;
	
	switch(errlevel)
	{
		case TL_LEVEL_DEBUG:
			HAL_UART_Transmit(&LOG_HUARTX,"\r\n[D]:",6,200);
			break;
		case TL_LEVEL_INFO:
			HAL_UART_Transmit(&LOG_HUARTX,"\r\n[I]:",6,200);
			break;
		case TL_LEVEL_WARNING:
			HAL_UART_Transmit(&LOG_HUARTX,"\r\n[W]:",6,200);
			break;
		default:
			HAL_UART_Transmit(&LOG_HUARTX,"\r\n[E]:",6,200);
			sg_ErrLogStat[errlevel]++;
			
	}
	HAL_UART_Transmit(&LOG_HUARTX,(uint8_t*)log,strlen(log),200);
}

#else

#ifdef USE_TRACELOG_VCP

#include "usbd_agent.h"

void TL_Init()
{
}

void TL_TraceLog(uint16_t errlevel,char* log)
{
	if((errlevel < (TL_LEVEL))||(errlevel >= (TL_ERR_MAX)))	
		return ;
	
	switch(errlevel)
	{
		case TL_LEVEL_DEBUG:
			USBD_VPC_Send("\r\n[D]:",6);
			break;
		case TL_LEVEL_INFO:
			USBD_VPC_Send("\r\n[I]:",6);
			break;
		case TL_LEVEL_WARNING:
			USBD_VPC_Send("\r\n[W]:",6);
			break;
		default:
			//USBD_VPC_Send("\r\n[E]:",6);
			sg_ErrLogStat[errlevel]++;
			return;
			
	}
	USBD_VPC_Send((uint8_t*)log,strlen(log));
}

#else

void TL_Init()
{
}

void TL_TraceLog(uint16_t errlevel,char* log)
{

}



#endif
#endif
















#endif

















