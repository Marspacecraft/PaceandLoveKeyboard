
#include "timeradc.h"
#include "timertask.h"

#define ADC_ADCX                            ADC1 
#define ADC_ADCX_DMACx                      DMA1_Channel1         
#define ADC_ADCX_DMACx_IS_TC()              ( DMA1->ISR & (1 << 1) )    /* ?D?? DMA1_Channel1 ∩?那?赤那3谷㊣那??, ?a那?辰????迄o‘那yD?那?,                                                                      */
#define ADC_ADCX_DMACx_CLR_TC()             do{ DMA1->IFCR |= 1 << 1; }while(0) /* ??3y DMA1_Channel1 ∩?那?赤那3谷㊣那?? */


#define ADC_FILTERING_NUM 		8 // largest value is 16
#define ADC_CHANNEL_NUM 		ADC_CHN_MAX
#define ADC_DMA_BUF_SIZE        ADC_FILTERING_NUM * ADC_CHANNEL_NUM   

static uint16_t sg_adc_dma_buf[ADC_DMA_BUF_SIZE];   /* ADC DMA BUF */     


static uint8_t sg_ADC_DATA_Really = 0;

static adcfunc sg_ADCChannelCB[ADC_CHN_MAX] = {0};
static CycleTimerHandler* adctimer = NULL;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;


void ADC_RegisterChannle(ADC_ChannleType chan,adcfunc func)
{
	if(chan >= ADC_CHN_MAX)
		return;
	
	sg_ADCChannelCB[chan] = func;
}

void ADC_UnRegisterChannle(ADC_ChannleType chan)
{
	if(chan >= ADC_CHN_MAX)
		return;
	
	sg_ADCChannelCB[chan] = NULL;
}



void TimerADC_DMA_init()
{
    HAL_ADCEx_Calibration_Start(&hadc1);              
    HAL_DMA_Start_IT(&hdma_adc1, (uint32_t)&ADC1->DR, (uint32_t)sg_adc_dma_buf, 0);   
    HAL_ADC_Start_DMA(&hadc1, ( uint32_t*)&sg_adc_dma_buf, ADC_DMA_BUF_SIZE);    
}

void TimerADC_DMA_CallBack()
{
	if (ADC_ADCX_DMACx_IS_TC())
    {  	
        ADC_ADCX_DMACx_CLR_TC();             
		sg_ADC_DATA_Really = 1;
    }
}



void TimerADC_Start()
{
    ADC_ADCX->CR2 &= ~(1 << 0);               

    ADC_ADCX_DMACx->CCR &= ~(1 << 0);          
    while (ADC_ADCX_DMACx->CCR & (1 << 0));    
    ADC_ADCX_DMACx->CNDTR = ADC_DMA_BUF_SIZE;      
    ADC_ADCX_DMACx->CCR |= 1 << 0;            

    ADC_ADCX->CR2 |= 1 << 0;                  
    ADC_ADCX->CR2 |= 1 << 22;   
	//HAL_ADC_Start();
	sg_ADC_DATA_Really = 0;
}

void TimerADC_DMA_Really()
{
	typedef uint16_t (*ARRAY_2)[ADC_CHANNEL_NUM];
	uint8_t i,j;
	ARRAY_2 dma = (ARRAY_2)sg_adc_dma_buf;

	for(i=0;i<ADC_CHANNEL_NUM;i++)
	{
		for(j=1;j<ADC_FILTERING_NUM;j++)
		{
			dma[0][i] += dma[j][i];
		}
		dma[0][i] /= ADC_FILTERING_NUM;
	}
}



void TimerADC_Loop(uint8_t* arg)
{
	uint8_t i;
	if(0 == sg_ADC_DATA_Really)
		return;
	
	TimerADC_DMA_Really();

	for(i=0;i<ADC_CHANNEL_NUM;i++)
	{
		if(sg_ADCChannelCB[i])
		{
			sg_ADCChannelCB[i](sg_adc_dma_buf[i]);
		}
	}
	TimerADC_Start();
}

void ADC_Stop()
{
	if(adctimer)
	{
		Stop_CycleTimer(adctimer);
		adctimer = NULL;
	}
}

void ADC_Start()
{
	if(adctimer)
	{
		ADC_Stop();
	}
	TimerADC_DMA_init();
	TimerADC_Start();
	adctimer = Start_CycleTimer(TIMER_PERIOD_50MS,TimerADC_Loop,NULL);
}


#ifdef TEST_ADC
#include "tracelog.h"

void test_fft_r(uint16_t val)
{
	TRACELOG_DEBUG_PRINTF("\tFFT right adc[%d]",val);
}

void test_fft_l(uint16_t val)
{
	TRACELOG_DEBUG_PRINTF("\tFFT left adc[%d]",val);
}

void test_joy_x(uint16_t val)
{
	TRACELOG_DEBUG_PRINTF("\tjoy x  adc[%d]",val);
}

void test_joy_y(uint16_t val)
{
	TRACELOG_DEBUG_PRINTF("\tjoy y adc[%d]",val);
}

void test_pwr(uint16_t val)
{
	TRACELOG_DEBUG_PRINTF("\tpwr adc[%d]",val);
}





void testcase_adc()
{
	TRACELOG_DEBUG("Running testcase_adc...");
	TimerADC_DMA_init();
	TimerADC_Start();
	ADC_RegisterChannle(ADC_CHN_FFT_R,test_fft_r);
	ADC_RegisterChannle(ADC_CHN_FFT_L,test_fft_l);
	ADC_RegisterChannle(ADC_CHN_JOY_X,test_joy_x);
	ADC_RegisterChannle(ADC_CHN_JOY_Y,test_joy_y);
	ADC_RegisterChannle(ADC_CHN_PWR,test_pwr);

	while(1)
	{
		TimerADC_Loop(NULL);
		HAL_Delay(100);
	}

	TRACELOG_DEBUG("testcase_adc end...");
}

#endif
