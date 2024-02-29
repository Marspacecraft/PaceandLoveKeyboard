

#ifndef _TIMER_ADC_H_
#define _TIMER_ADC_H_
#include "main.h"

typedef enum
{
	ADC_CHN_FFT_R = 0,
	ADC_CHN_FFT_L,
	ADC_CHN_JOY_X,
	ADC_CHN_JOY_Y,
	ADC_CHN_PWR,
	ADC_CHN_MAX
}ADC_ChannleType;

typedef void (*adcfunc)(uint16_t val);


void ADC_RegisterChannle(ADC_ChannleType chan,adcfunc func);
void ADC_UnRegisterChannle(ADC_ChannleType chan);
void ADC_Start(void);
void ADC_Stop(void);

#ifdef TEST_ADC

void testcase_adc();

#endif


#endif







