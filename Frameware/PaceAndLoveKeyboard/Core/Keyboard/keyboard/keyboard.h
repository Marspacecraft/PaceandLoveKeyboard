
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "main.h"
#include "scheduler.h"
#include "keyconfig.h"

#define MAX_KEYBIT_SIZE ((MAX_KEYBIT_NUM%8)?((MAX_KEYBIT_NUM/8)+1):(MAX_KEYBIT_NUM/8))


typedef void (*KeySilentModFunc)(uint32_t*);
typedef void (*ForEachBitFunc_f)(uint8_t);

void KEY_Init(void);


//go into silent mode,if has keys change KeySilentModFunc will be called
uint8_t KEY_BeSilent_2_Host(KeySilentModFunc);
void KEY_BeDissilent_2_Host(void);

bool MouseButton_Left_IsPressed(void);
bool MouseButton_Right_IsPressed(void);


void KEY_Fn_Pressed(void);
void KEY_Fn_Free(void);


void KEY_SetScanHz(uint16_t hz);

void KEY_Scan_Start(void);


	
void Keyboard_Task(eApp_Event event,uint8_t* data,uint16_t len);


#ifdef TEST_KEYTASKSCHEDULER
void testcase_keytaskscheduler(void);

#endif



#endif





