
#ifndef _TIMER_TASK_H_
#define _TIMER_TASK_H_
#include "scheduler.h"
#include "keyconfig.h"

#define TIMER_PERIOD_10MS		TIEMR_PERIOD_LEVEL_0,0
#define TIMER_PERIOD_50MS		TIEMR_PERIOD_LEVEL_0,4
#define TIMER_PERIOD_100MS		TIEMR_PERIOD_LEVEL_1,0
#define TIMER_PERIOD_500MS		TIEMR_PERIOD_LEVEL_1,4
#define TIMER_PERIOD_1S			TIEMR_PERIOD_LEVEL_2,0
#define TIMER_PERIOD_2S			TIEMR_PERIOD_LEVEL_2,1
#define TIMER_PERIOD_10S		TIEMR_PERIOD_LEVEL_3,0
#define TIMER_PERIOD_30S		TIEMR_PERIOD_LEVEL_3,2
#define TIMER_PERIOD_1MIN		TIEMR_PERIOD_LEVEL_3,5
#define TIMER_PERIOD_15MIN		TIEMR_PERIOD_LEVEL_4,8
#define TIMER_PERIOD_1H	X		TIEMR_PERIOD_LEVEL_5,3
#define TIMER_PERIOD_2H	X		TIEMR_PERIOD_LEVEL_5,6
#define TIMER_PERIOD_5H	X		TIEMR_PERIOD_LEVEL_6,1
#define TIMER_PERIOD_12X		TIEMR_PERIOD_LEVEL_6,3
#define TIMER_PERIOD_1D	X		TIEMR_PERIOD_LEVEL_6,8


typedef enum
{
	TIEMR_PERIOD_LEVEL_0 = 0,// 	0(10ms) 	9(100ms)
	TIEMR_PERIOD_LEVEL_1,// 		0(100ms) 	9(1s)	
	TIEMR_PERIOD_LEVEL_2,//  		0(1s) 		9(10s)
	TIEMR_PERIOD_LEVEL_3,//			0(10s)		9(1min40s)
	TIEMR_PERIOD_LEVEL_4,//			0(100s)		9(16min40s)
	TIEMR_PERIOD_LEVEL_5,//			0(1000s)	9(2h46min40s)
	TIEMR_PERIOD_LEVEL_6,//			0(10000s)	9(1d3h46min40s)
	TIEMR_PERIOD_LEVEL_MAX,
}timertype_e;


typedef void (*timertaskfunc)(uint8_t* arg);

typedef uint8_t* CycleTimerHandler;

void CycleTimer_Init(void);

CycleTimerHandler* Start_CycleTimer(timertype_e type,uint8_t period,timertaskfunc func,uint8_t* arg);
void Stop_CycleTimer(CycleTimerHandler* timer);
void CycleTimer_Task(eApp_Event event,uint8_t* data,uint16_t ms);

#ifdef TEST_TIMERTASK

void testcase_timertask(void);

#endif


#endif





