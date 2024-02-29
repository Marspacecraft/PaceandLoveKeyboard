#include "tracelog.h"
#include <string.h>

#include "main.h"
#include "usart.h"

void TL_Init()
{
}

void TL_TraceLog(uint16_t errlevel,char* log)
{
	if(errlevel < (TL_LEVEL))	
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
			
	}
	HAL_UART_Transmit(&LOG_HUARTX,(uint8_t*)log,strlen(log),200);
}


















