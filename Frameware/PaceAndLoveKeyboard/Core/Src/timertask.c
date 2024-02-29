#include <string.h>
#include "timertask.h"
#include "scheduler.h"
#include "tim.h"
#include "tracelog.h"

#define MAX_TIMER_LIST_NUM 		10



typedef struct
{
	timertype_e 	type;
	uint8_t			enable;
	uint8_t 		cycleperiod;
	uint8_t* 		arg;
	timertaskfunc 	func;
}CycleTask_t;


typedef struct cycletimer
{
	CycleTask_t 		task;
	uint8_t 			enable;
	struct cycletimer* 	next;
}CycleTimer_t;


typedef struct
{
	uint8_t 		pos;
	CycleTimer_t*	tasks[MAX_TIMER_LIST_NUM];
}CTimer_t;



CTimer_t sg_TimerLevelMsg[TIEMR_PERIOD_LEVEL_MAX] = {0};
uint16_t sg_TimerLevelCount[TIEMR_PERIOD_LEVEL_MAX] =  {0};
#define TIMER_POOL_NUM 20
static CycleTimer_t sg_TimerPool[TIMER_POOL_NUM];

CycleTimer_t *  GetTimerHandlerFromPool()
{
	static uint8_t pos = 0;
	uint8_t i;
	for(i=0;i<TIMER_POOL_NUM;i++,pos++)
	{
		uint8_t j = (pos)%TIMER_POOL_NUM;
		if(0 == sg_TimerPool[j].enable)
		{
			sg_TimerPool[j].enable = 1;
			return &sg_TimerPool[j];
		}
	}
	return NULL;
}

void  FreeTimerHandler2Pool(CycleTimer_t * timer)
{
	timer->enable = 0;
	return;
}

static void PostTask2Timer(timertype_e type,uint8_t pos,CycleTimer_t* handler)
{
	pos = (sg_TimerLevelMsg[type].pos + pos)%MAX_TIMER_LIST_NUM;
	do 
	{
		handler->next = sg_TimerLevelMsg[type].tasks[pos];
		
	}while(false == KB_ATOMIC_CAS_ADDR((volatile void **)&sg_TimerLevelMsg[type].tasks[pos],handler->next ,handler));
}

CycleTimerHandler* Start_CycleTimer(timertype_e type,uint8_t period,timertaskfunc func,uint8_t* arg)
{

	CycleTimer_t* handler;
	
	if(TIEMR_PERIOD_LEVEL_MAX <= type)
		return NULL;	
	
	handler = GetTimerHandlerFromPool();
	if(NULL == handler)
		return NULL;
	handler->task.func = func;
	handler->task.arg = arg;
	handler->task.type = type;
	handler->task.cycleperiod = period;
	handler->task.enable = 1;

	PostTask2Timer(type,period,handler);

	//TRACELOG_INFO("TIMER start");

	return (CycleTimerHandler*)handler;
}

void Stop_CycleTimer(CycleTimerHandler* timer)
{
	CycleTimer_t* handler = (CycleTimer_t*)timer;
	
	if(NULL == handler || handler->task.type >= TIEMR_PERIOD_LEVEL_MAX)
		return;	
	
	handler->task.enable = 0;
	//TRACELOG_INFO("TIMER stop");
}


static void do_timertask(timertype_e type)
{
	uint8_t pos;
	pos = (sg_TimerLevelMsg[type].pos)%MAX_TIMER_LIST_NUM;

	CycleTimer_t* task = sg_TimerLevelMsg[type].tasks[pos];
	sg_TimerLevelMsg[type].tasks[pos] = NULL;
	sg_TimerLevelMsg[type].pos++;
	
	while(task)
	{
		if(0 == task->task.enable)
		{
			CycleTimer_t* tasktm = task->next;
			FreeTimerHandler2Pool(task);
			task = tasktm;
			continue;
		}
		
		if(task->task.func)
		{
			task->task.func(task->task.arg);
			PostTask2Timer(task->task.type, task->task.cycleperiod, task);
		}
		task = task->next;
	}
}

static uint8_t Timer_Really(timertype_e type,uint16_t count)
{
	if(sg_TimerLevelCount[type] != count)
	{
		sg_TimerLevelCount[type] = count;
		do_timertask(type);
		return true;
	}
	else
	{
		return false;
	}
}

void CycleTimer_Task(eApp_Event event,uint8_t* data,uint16_t ms)
{
	uint16_t count = ms;
	timertype_e i;

	for(i=TIEMR_PERIOD_LEVEL_0;i<TIEMR_PERIOD_LEVEL_MAX;i++)
	{
		if(false==Timer_Really(i,count))
			return;

		count /= MAX_TIMER_LIST_NUM;
	}
}


void CycleTimer_Init(void)
{

	memset(sg_TimerLevelMsg,0,sizeof(sg_TimerLevelMsg));
	memset(sg_TimerLevelCount,0,sizeof(sg_TimerLevelCount));
	memset(sg_TimerPool,0,sizeof(sg_TimerPool));

	CycleTimer_Task(EVENT_NONE,NULL,TIM_GetCounnt_MS());
	CycleTimer_Task(EVENT_NONE,NULL,TIM_GetCounnt_MS());
}


#ifdef TEST_TIMERTASK




void testtimertaskfunc(uint8_t* arg)
{
	TRACELOG_DEBUG_PRINTF("\t\tGet [%d]",(uint32_t)arg);
}

void test_timeperiod()
{
	uint16_t i = 2000;
	CycleTimerHandler* t1;
	TRACELOG_DEBUG("\ttest_timeperiod");
#if 0
	TRACELOG_DEBUG("\ttime period 100MS time20s");
	t1 = Start_CycleTimer(TIMER_PERIOD_100MS,testtimertaskfunc,(uint8_t*)1);
	while(i--)
	{
		do_schedule();
		HAL_Delay(10);
	};		
	Stop_CycleTimer(t1);
#endif
#if 0
	i = 2000;
	TRACELOG_DEBUG("\ttime period 500MS time20s");
	t1 = Start_CycleTimer(TIMER_PERIOD_500MS,testtimertaskfunc,(uint8_t*)2);
	while(i--)
	{
		do_schedule();
		HAL_Delay(10);
	};	
	Stop_CycleTimer(t1);
#endif
#if 0
	i = 2000;
	TRACELOG_DEBUG("\ttime period 1S time20s");
	t1 = Start_CycleTimer(TIMER_PERIOD_1S,testtimertaskfunc,(uint8_t*)3);
	while(i--)
	{
		do_schedule();
		HAL_Delay(10);
	};		
	Stop_CycleTimer(t1);
#endif
#if 1
	i = 240;
	TRACELOG_DEBUG("\ttime period 30S time2min");
	t1 = Start_CycleTimer(TIMER_PERIOD_30S,testtimertaskfunc,(uint8_t*)4);
	while(i--)
	{
		do_schedule();
		HAL_Delay(500);
	};
		
	Stop_CycleTimer(t1);
#endif
}


void test_startstop()
{
	uint16_t i = 2000;
	CycleTimerHandler* t1;
	TRACELOG_DEBUG("\ttest_startstop");
	TRACELOG_DEBUG("\t\t start timer1");
	t1 = Start_CycleTimer(TIMER_PERIOD_1S,testtimertaskfunc,(uint8_t*)1);
	while(i--)
	{
		do_schedule();
		HAL_Delay(10);
	};
	
	Stop_CycleTimer(t1);
	TRACELOG_DEBUG("\t\t stop timer1");
	i = 2000;
	while(i--)
	{
		do_schedule();
		HAL_Delay(1);
	};
	TRACELOG_DEBUG("\ttest_startstop end");
}

#if 0 
uint16_t i = 5000;
	CycleTimerHandler* t1,t2,t3,t4,t5,t6;

TRACELOG_DEBUG("\t test ms timer task");

t1 = Start_CycleTimer(TIEMR_TASK_MS,0,testtimertaskfunc,(uint8_t*)i);
while(i--)
{
	do_schedule();
	HAL_Delay(1);
};

TRACELOG_DEBUG("\t test 10ms 100ms timer task");

t2 = Start_CycleTimer(TIEMR_TASK_10MS,0,testtimertaskfunc,(uint8_t*)(i+MAX_TIMER_LIST_NUM));
t3 = Start_CycleTimer(TIEMR_TASK_100MS,0,testtimertaskfunc,(uint8_t*)(i+2*MAX_TIMER_LIST_NUM));
while(do_schedule());

TRACELOG_DEBUG("\t test s 10s min timer task");

t4 = Start_CycleTimer(TIEMR_TASK_S,0,testtimertaskfunc,(uint8_t*)(i+3*MAX_TIMER_LIST_NUM));
t5 = Start_CycleTimer(TIEMR_TASK_10S,0,testtimertaskfunc,(uint8_t*)(i+4*MAX_TIMER_LIST_NUM));
t6 = Start_CycleTimer(TIEMR_TASK_MIN,0,testtimertaskfunc,(uint8_t*)(5*MAX_TIMER_LIST_NUM));
while(do_schedule());

#endif

void testcase_timertask()
{
	
	TRACELOG_DEBUG("Running testcase_timertask");
	
	//test_startstop();
	test_timeperiod();

	TRACELOG_DEBUG("testcase_timertask end");	
}

#endif





