
#ifndef JOYSTICK_H
#define JOYSTICK_H
#include "keyconfig.h"
#include "main.h"

void Joystick_Init(void);

void Joystick_Start(void);
void Joystick_Stop(void);

void Joystick_Calibrate(void);



#ifdef TEST_JOYSTICK

void testcase_joystick(void);

#endif

#endif







