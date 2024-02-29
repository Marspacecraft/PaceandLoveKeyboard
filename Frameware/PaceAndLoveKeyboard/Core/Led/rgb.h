#ifndef _RGB_H_
#define _RGB_H_

#include "main.h"
#include "keyconfig.h"

typedef uint16_t rgbbit_t;



typedef void (*rgbmodinitfunc)(void);

void RGB_Start(rgbmodinitfunc func);
void RGB_Stop(void);

void RGB_Init_BreathingLight(void);
void RGB_Init_RunningLine(void);
void RGB_Init_KeyCross(void);
void RGB_Mode_Init(void);
void RGB_Mode_Change(void);




#ifdef TEST_REGMODE

void testcase_testrgbmode(void);

#endif



#endif



