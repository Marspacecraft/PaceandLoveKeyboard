
#ifndef APP_SCHEDULER
#define APP_SCHEDULER


#define MAX_APP_SCHED_ITEM_NUM 64

#include "main.h"
typedef enum
{
	EVENT_NONE = 0,
	EVENT_TEST,
	EVENT_KEY_SCAN,
	EVENT_CYCLE_TIMER,

	SCAN_TIMEROUT,
	DEBOUNCE_TIMEROUT,//5
	KEY_TASK_TEST,

	KEY_FN_INTERRUPT,

}eApp_Event;

typedef void (*App_Sched_Handler)(eApp_Event,uint8_t*,uint16_t);

typedef struct
{
   	volatile App_Sched_Handler 	handler;        
	eApp_Event			event;
	uint8_t*			data;
    uint16_t    		size;
} tSched_Itemer;


void init_scheduler(void);
bool push_2_scheduler(tSched_Itemer * pitem);

//ms:0 ~63 15Hz to 1000Hz
//核心键盘扫描函数调度队列，SPI1 USB相关函数和非BUS函数方可入队
//非SPI1的BUS等待时将会由do_ms_schedule和do_ms_schedule_1ms调度
bool push_2_ms_scheduler(tSched_Itemer * pitem,uint8_t ms);

bool do_schedule(void);

uint16_t do_ms_schedule_1ms(void);
void do_ms_schedule(void);


#ifdef TEST_SCHEDULER
void testcase_scheduler(void);
#endif

#endif





