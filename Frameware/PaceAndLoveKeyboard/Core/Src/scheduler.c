
#include "scheduler.h"
#include "string.h"
#include "tracelog.h"
#include "tim.h"

static tSched_Itemer g_App_Sched_Items[MAX_APP_SCHED_ITEM_NUM]={0};
static volatile uint8_t g_fifo_head = 0;
static volatile uint8_t g_fifo_tail = 0;

#define MAX_MS_QUEUE 		((1000/TIM_COUNT_PERIOD)/15) //15HZ~
#define MAX_MS_QUEUE_NUM 	2 


static tSched_Itemer sg_MSQueue[MAX_MS_QUEUE][MAX_MS_QUEUE_NUM] = {0};
static uint16_t sg_MSCount = 0;

static uint16_t sg_MSQueueNum = 0;

#define MS_QUEUE_TIMER_REALLY (sg_MSCount != TIM_GetCounnt_MS())
void do_ms_schedule(void);


#ifdef DETECT_DELAY

static uint16_t sg_timer_before = 0;

static void FuncBefore()
{
	sg_timer_before = TIM_GetCounnt_MS();

}

static void FuncAfter(tSched_Itemer* item,uint8_t type)
{
	static uint16_t sg_delay_max = 1;
	uint16_t delay = TIM_GetCounnt_MS() - sg_timer_before;
	if(delay >= sg_delay_max)
	{
		sg_delay_max = delay;

		TRACELOG_DEBUG_PRINTF("Max delay[%d] func[%d][%p] event[%d] data[%p] len[%d]",5*sg_delay_max,type,item->handler,item->event,item->data,item->size);
	}
}

#define FUNC_IN_BEFORE() FuncBefore()
#define FUNC_OUT_AFTER(ITEM,TYPE) FuncAfter((ITEM),(TYPE))

#else

#define FUNC_IN_BEFORE() {}
#define FUNC_OUT_AFTER(ITEM,TYPE) {}


#endif

void pull_ms_queue()
{
	uint16_t count_now = TIM_GetCounnt_MS();
	uint16_t count_old = sg_MSCount;
	uint16_t i,j,k;

	for(i=((count_old)%MAX_MS_QUEUE),k=0;k < ((count_now-count_old)%MAX_MS_QUEUE);i++,k++)
	{
		for(j=0;j<MAX_MS_QUEUE_NUM;j++)
		{
			if(sg_MSQueue[i%MAX_MS_QUEUE][j].handler)
			{
				FUNC_IN_BEFORE();
				sg_MSQueue[i%MAX_MS_QUEUE][j].handler(sg_MSQueue[i%MAX_MS_QUEUE][j].event,sg_MSQueue[i%MAX_MS_QUEUE][j].data,sg_MSQueue[i%MAX_MS_QUEUE][j].size);
				FUNC_OUT_AFTER(&sg_MSQueue[i%MAX_MS_QUEUE][j],1);
				sg_MSQueue[i%MAX_MS_QUEUE][j].handler = NULL;
				sg_MSQueueNum--;
			}
		}
	}
	sg_MSCount = count_now;
}


bool push_2_ms_scheduler(tSched_Itemer * pitem,uint8_t ms)
{
	ms++;
	
	uint16_t count = (TIM_GetCounnt_MS()+ms)%MAX_MS_QUEUE;
	uint8_t i;

	for(i=0;i<MAX_MS_QUEUE_NUM;i++)
	{
		if(KB_ATOMIC_CAS_ADDR((volatile void **)&sg_MSQueue[count][i].handler,NULL,pitem->handler))
		{
			sg_MSQueue[count][i].event = pitem->event;
			sg_MSQueue[count][i].data = pitem->data;
			sg_MSQueue[count][i].size = pitem->size;
			sg_MSQueueNum++;
			__DMB();
			return true;
		}
	}
	return false;
	
}



static bool fifo_full()
{
	return ((g_fifo_head - g_fifo_tail) == (MAX_APP_SCHED_ITEM_NUM - 1));
}
//__INLINE
static bool  fifo_empty()
{
	return (((g_fifo_head - g_fifo_tail)%MAX_APP_SCHED_ITEM_NUM) == 0);
}

void init_scheduler(void)
{
	memset(g_App_Sched_Items,0,sizeof(g_App_Sched_Items));
	g_fifo_head = 0;
	g_fifo_tail = 0;

	memset(sg_MSQueue,0,sizeof(sg_MSQueue));
	sg_MSCount = 0;
	sg_MSQueueNum = 0;
}

bool push_2_scheduler(tSched_Itemer * pitem)
{
	uint8_t head;
	do 
	{
		head = g_fifo_head;
		if(fifo_full())
			return false;
		
	}while(false == KB_ATOMIC_CAS(&g_fifo_head,head,head+1));
	g_App_Sched_Items[head%MAX_APP_SCHED_ITEM_NUM] = *pitem;
	return true;
}



static bool pull_from_scheduler(tSched_Itemer * pitem)
{
	uint8_t tail;

	do 
	{	
		tail = g_fifo_tail;
		if(fifo_empty())
			return false;
		*pitem = g_App_Sched_Items[tail%MAX_APP_SCHED_ITEM_NUM];
	}while(false == KB_ATOMIC_CAS(&g_fifo_tail,tail,tail+1));
	
	return true;
}


bool do_schedule(void)
{
	uint8_t count = 10;
	tSched_Itemer item;
	
	do 
	{
		do_ms_schedule();
		//TRACELOG_DEBUG_PRINTF("Count[%d] Now[%d]",sg_MSCount,TIM_GetCounnt_MS());
		if(pull_from_scheduler(&item))
		{
			if(item.handler)
			{
				FUNC_IN_BEFORE();
				item.handler(item.event,item.data,item.size);
				FUNC_OUT_AFTER(&item,0);
			}
		}
		else
		{
			break;
		}
			
	}while(count--);
		
	return (fifo_empty()&&(!sg_MSQueueNum));
}


uint16_t do_ms_schedule_1ms(void)
{
	uint16_t count_new,count_now = TIM_GetCounnt_MS();
	while(count_now == (count_new=TIM_GetCounnt_MS()))
	{
		do_ms_schedule();
	}
	return count_new - count_now;
}

void do_ms_schedule(void)
{
	if(MS_QUEUE_TIMER_REALLY)
	{
		pull_ms_queue();
	}
}




#ifdef TEST_SCHEDULER

#include <stdio.h>

void Scheduler(eApp_Event event,uint8_t* buffer,uint16_t time)
{
	if(0 == event)
	{
		TRACELOG_DEBUG_PRINTF("Get KeyTask %d\r\n",time);
		
	}
	else
	{
		TRACELOG_DEBUG_PRINTF("Get NormalTask %d\r\n",time);
	}
}



void test_doscheduler()
{
	while(!do_schedule())
	{
		HAL_Delay(10);
		
	}
}


bool put_KeyScheduler(uint16_t time)
{
	tSched_Itemer item;
	item.handler = Scheduler;
	item.size = time;
	item.event = (eApp_Event)0;


	TRACELOG_DEBUG_PRINTF("Put KeyTask %d\r\n",time);

	if(false == push_2_ms_scheduler(&item,1))
	{
		TRACELOG_DEBUG_PRINTF("Put KeyTask %d fail!\r\n",time);
		return false;
	}
	
	return true;
}

bool put_NormalScheduler(uint16_t time)
{
	tSched_Itemer item;
	item.handler = Scheduler;
	item.size = time;
	item.event = (eApp_Event)1;
	
	TRACELOG_DEBUG_PRINTF("Put NormalTask %d\r\n",time);

	if(false == push_2_scheduler(&item))
	{
		TRACELOG_DEBUG_PRINTF("Put NormalTask %d fail!\r\n",time);
		return false;
	}
	
	return true;
}

void Scheduler_put(eApp_Event event,uint8_t* buffer,uint16_t time)
{
	if(0 == event)
	{
		TRACELOG_DEBUG_PRINTF("Get PutTask %d\r\n",time);
	}
	else
	{
		TRACELOG_DEBUG_PRINTF("Get PutTask %d\r\n",time);
	}


	put_KeyScheduler(2);
	put_NormalScheduler(10);
}

bool put_NormalScheduler_put(uint16_t time)
{
	tSched_Itemer item;
	item.handler = Scheduler_put;
	item.size = time;
	item.event = (eApp_Event)1;
	
	TRACELOG_DEBUG_PRINTF("Put PutTask %d\r\n",time);

	if(false == push_2_scheduler(&item))
	{
		TRACELOG_DEBUG_PRINTF("Put PutTask %d fail!\r\n",time);
		return false;
	}
	
	return true;
}

void test_put_and_get()
{
	TRACELOG_DEBUG("\ttest_put_and_get!");

	put_NormalScheduler(1);
	test_doscheduler();
	put_KeyScheduler(1);
	test_doscheduler();

	TRACELOG_DEBUG("\ttest_put_and_get end!");
}

void test_normaltask_full()
{
	uint16_t i;
	TRACELOG_DEBUG("\ttest_normaltask_full!");
	for(i=0;i<=MAX_APP_SCHED_ITEM_NUM;i++)
		put_NormalScheduler(i);
	test_doscheduler();
	TRACELOG_DEBUG("\ttest_normaltask_full end!");
	
}



void testcase_scheduler()
{
	TRACELOG_DEBUG("Running testcase_scheduler...");
	init_scheduler();
	test_put_and_get();
	test_normaltask_full();
	TRACELOG_DEBUG("testcase_scheduler end...");
}

#endif

