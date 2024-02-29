
#include "joystick.h"
#include "main.h"
#include "tracelog.h"
#include "timeradc.h"
#include "scheduler.h"
#include "tim.h"
#include "usbd_agent.h"
#include "memery.h"
#include "keyboard.h"
#include <string.h>
#include "st7302.h"
#include "oled.h"
#define JOY_ORIGIN_DEATHLINE 	40//2%
#define JOY_ORIGIN_CALIBRATELINE 	10//1%

#define MOUSE_HID_MAX_VALUE 	127


static uint16_t sg_Joystic_X = 0;
static uint16_t sg_Joystic_Y = 0;

static uint16_t sg_Joystic_X_Origin = 2048;
static uint16_t sg_Joystic_Y_Origin = 2048;

static uint16_t sg_Joystic_X_Max = 4096;
static uint16_t sg_Joystic_Y_Max = 4096;

static uint16_t sg_Joystic_X_Min = 0;
static uint16_t sg_Joystic_Y_Min = 0;


#define Joystick_USBSend(INFO) 	 USBD_VPC_Send((INFO),strlen((INFO)))

void Joystick_CalibrateOrigin(void);
void Joystick_CalibrateBounding(void);


uint8_t Compare_ABS(uint16_t base,uint16_t cvalue,uint16_t range)
{
	if((cvalue>base) && (cvalue < base + range))
		return true;

	if((cvalue<base) && (cvalue + range > base))
		return true;

	return false;
}

void Joystick_DynamicCalibrate_X(uint16_t value)
{

	static uint16_t Origin = 0;
	static uint8_t count = 0;

	if(Origin==0)
		Origin = sg_Joystic_X_Origin;

	if(Compare_ABS(Origin,value,JOY_ORIGIN_CALIBRATELINE))
	{
		count++;
	}
	else
	{
		count = 0;
		Origin = value;
		return;
	}

	if(count>=50)//5s
	{
		if(Compare_ABS(sg_Joystic_X_Origin,Origin,200))
		{
			sg_Joystic_X_Origin = Origin;
		}
		else
		{
			Origin = value;
		}
		count = 0;
	}
}

void Joystick_DynamicCalibrate_Y(uint16_t value)
{
	static uint16_t Origin = 0;
	static uint8_t count = 0;

	if(Origin==0)
		Origin = sg_Joystic_Y_Origin;

	if(Compare_ABS(Origin,value,JOY_ORIGIN_CALIBRATELINE))
	{
		count++;
	}
	else
	{
		count = 0;
		Origin = value;
		return;
	}

	if(count>=50)//5s
	{
		if(Compare_ABS(sg_Joystic_Y_Origin,Origin,200))
		{
			sg_Joystic_Y_Origin = Origin;
		}
		else
		{
			Origin = value;
		}
		count = 0;
	}
}


void Joystick_ReadConfig()
{
	uint32_t addr = CONFIG_JOYSTICK_ADDR;
	if(MEM_FlagBit_IsEnabled(MEM_FLAG_JOYSTICK))
	{
		IntFlash_Read(addr,(uint8_t*)&sg_Joystic_X_Origin,2);addr+=2;
		IntFlash_Read(addr,(uint8_t*)&sg_Joystic_Y_Origin,2);addr+=2;
		
		IntFlash_Read(addr,(uint8_t*)&sg_Joystic_X_Max,2);addr+=2;
		IntFlash_Read(addr,(uint8_t*)&sg_Joystic_Y_Max,2);addr+=2;
		
		IntFlash_Read(addr,(uint8_t*)&sg_Joystic_X_Min,2);addr+=2;
		IntFlash_Read(addr,(uint8_t*)&sg_Joystic_Y_Min,2);
	}
}

void Joystick_WriteConfig()
{
	uint32_t addr = CONFIG_JOYSTICK_ADDR;
	IntFlash_Write(addr,(uint8_t*)&sg_Joystic_X_Origin,2);addr+=2;
	IntFlash_Write(addr,(uint8_t*)&sg_Joystic_Y_Origin,2);addr+=2;
	
	IntFlash_Write(addr,(uint8_t*)&sg_Joystic_X_Max,2);addr+=2;
	IntFlash_Write(addr,(uint8_t*)&sg_Joystic_Y_Max,2);addr+=2;
	
	IntFlash_Write(addr,(uint8_t*)&sg_Joystic_X_Min,2);addr+=2;
	IntFlash_Write(addr,(uint8_t*)&sg_Joystic_Y_Min,2);

	MEM_FlagBit_Set(MEM_FLAG_JOYSTICK);
}



void Post_Joystick(uint16_t adcX,uint16_t adcY)
{
	int8_t X,Y;
	uint16_t bound;
	int16_t Delta;
	hid_mouse report;

	Joystick_DynamicCalibrate_X(adcX);
	Joystick_DynamicCalibrate_Y(adcY);
	
	//TRACELOG_DEBUG_PRINTF("X[%d-%d] Y[%d-%d]",adcX,sg_Joystic_X_Origin,adcY,sg_Joystic_Y_Origin);
	Delta = ((int16_t) adcX) - ((int16_t)sg_Joystic_X_Origin);
	if(Delta > JOY_ORIGIN_DEATHLINE)
	{
		bound = sg_Joystic_X_Max - sg_Joystic_X_Origin;
	}
	else if(Delta < (-JOY_ORIGIN_DEATHLINE))
	{
		bound = sg_Joystic_X_Origin - sg_Joystic_X_Min;
	}
	else
	{
		Delta = 0;
		
	}
	X = (int8_t)(Delta * ((int16_t)MOUSE_HID_MAX_VALUE) / ((int16_t)bound));

	Delta = ((int16_t)adcY) - ((int16_t)sg_Joystic_Y_Origin);
	if(Delta > JOY_ORIGIN_DEATHLINE)
	{
		bound = sg_Joystic_Y_Max - sg_Joystic_Y_Origin;
	}
	else if(Delta < (-JOY_ORIGIN_DEATHLINE))
	{
		bound = sg_Joystic_Y_Origin - sg_Joystic_Y_Min;
	}
	else
	{
		Delta = 0;
	}
	Y = -(int8_t)(Delta * ((int16_t)MOUSE_HID_MAX_VALUE) / ((int16_t)bound));

	report.buttons = 0;
	USBD_Mouse_SetButton(&report,HID_BOTTON_LEFT,MouseButton_Left_IsPressed());
	USBD_Mouse_SetButton(&report,HID_BOTTON_RIGHT,MouseButton_Right_IsPressed());
	if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(JOYSTICK_KEY_GPIO_Port,JOYSTICK_KEY_Pin))
	{
		USBD_Mouse_SetButton(&report,HID_BOTTON_MID,false);
	}
	else
	{
		USBD_Mouse_SetButton(&report,HID_BOTTON_MID,true);
	}
	
	report.x = X;
	report.y = Y;
	report.wheel = 0;

	USBD_Mouse_Send(&report);
	
}


static void TimerADCCB_X(uint16_t val)
{
	sg_Joystic_X = val;
}

static void TimerADCCB_Y(uint16_t val)
{
	sg_Joystic_Y = val;
	Post_Joystick(sg_Joystic_X,sg_Joystic_Y);
}


void Joystick_Start()
{
	Joystick_CalibrateOrigin();
	ADC_RegisterChannle(ADC_CHN_JOY_X,TimerADCCB_X);
	ADC_RegisterChannle(ADC_CHN_JOY_Y,TimerADCCB_Y);
	
}

void Joystick_Stop()
{
	ADC_UnRegisterChannle(ADC_CHN_JOY_Y);
	ADC_UnRegisterChannle(ADC_CHN_JOY_X);
	
}

volatile static uint16_t sg_calibcount = 0;
static void TimerADCCB_CalibXOri(uint16_t val)
{
	sg_Joystic_X_Origin += val;
}

static void TimerADCCB_CalibYOri(uint16_t val)
{
	sg_Joystic_Y_Origin += val;
	sg_calibcount--;
}

void Joystick_CalibrateOrigin()
{
	sg_Joystic_X_Origin = 0;
	sg_Joystic_Y_Origin = 0;
	sg_calibcount = 10;//max 10
	ADC_RegisterChannle(ADC_CHN_JOY_X,TimerADCCB_CalibXOri);
	ADC_RegisterChannle(ADC_CHN_JOY_Y,TimerADCCB_CalibYOri);

	do
	{
		do_schedule();
	}while(sg_calibcount);

	sg_Joystic_X_Origin /= 10;
	sg_Joystic_Y_Origin /= 10;
	//Joystick_Stop();
}

static void TimerADCCB_CalibXBound(uint16_t val)
{
	if(sg_Joystic_X_Max < val)
		sg_Joystic_X_Max = val;
	if(sg_Joystic_X_Min > val)
		sg_Joystic_X_Min = val;
}

static void TimerADCCB_CalibYBound(uint16_t val)
{
	if(sg_Joystic_Y_Max < val)
		sg_Joystic_Y_Max = val;
	if(sg_Joystic_Y_Min > val)
		sg_Joystic_Y_Min = val;
}


void Joystick_CalibrateBounding()
{
	uint8_t before,after;

	sg_Joystic_X_Max = sg_Joystic_X_Origin;
	sg_Joystic_Y_Max = sg_Joystic_Y_Origin;
	sg_Joystic_X_Min = sg_Joystic_X_Origin;
	sg_Joystic_Y_Min = sg_Joystic_Y_Origin;
	
	sg_calibcount = 2000;//10s
	ADC_RegisterChannle(ADC_CHN_JOY_X,TimerADCCB_CalibXBound);
	ADC_RegisterChannle(ADC_CHN_JOY_Y,TimerADCCB_CalibYBound);

	do
	{
		before = TIM_GetCounnt_MS();
		do_schedule();
		after = TIM_GetCounnt_MS() - before;
		if(sg_calibcount <= after)
			break;
		sg_calibcount -= after;
	}while(1);

	//Joystick_Stop();
	Joystick_WriteConfig();

	ADC_RegisterChannle(ADC_CHN_JOY_X,TimerADCCB_X);
	ADC_RegisterChannle(ADC_CHN_JOY_Y,TimerADCCB_Y);
}


void Joystick_Init()
{
	Joystick_ReadConfig();
}

void Joystick_Calibrate()
{

	#ifndef SPI_SHOW_LCD
		Joystick_USBSend("Calibrate Origin,Do not touch...");
	#else
		LCD_ShowString(0, 18, "Calibrate Origin,Do not touch...", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
	#endif

	#ifdef I2C_OLED
		{
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Calibrate Origin,Do not touch...");
		}
	#endif

	Joystick_CalibrateOrigin();

	#ifndef SPI_SHOW_LCD
		Joystick_USBSend("Calibrate Origin end.");
		Joystick_USBSend("Calibrate Bounding,Rotating Rod...");
	#else
		LCD_ShowString(0, 18, "Calibrate Bounding,Rotating Rod...", FONT_ASCII_11x16,LCD_SHOW_MODE_GRAY_2);
	#endif

	#ifdef I2C_OLED
		{
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Calibrate Bounding,Rotating Rod...");
		}
	#endif
	
	Joystick_CalibrateBounding();

	#ifndef SPI_SHOW_LCD
		Joystick_USBSend("Calibrate Origin end.");
	#else
		LCD_ShowString(0, 18, "Calibrate Origin end.", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
	#endif

	#ifdef I2C_OLED
		{
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Calibrate Origin end.");
		}
	#endif

}



#ifdef TEST_JOYSTICK
#include "tracelog.h"

void test_mouse()
{
	TRACELOG_DEBUG("\ttest_mouse...");
	Joystick_Start();
}



void testcase_joystick()
{
	TRACELOG_DEBUG("\tRunning testcase_joystick...");
	ADC_Start();
	Joystick_Calibrate();
	HAL_Delay(10000);
	test_mouse();
	TRACELOG_DEBUG("\ttestcase_joystick end...");
}

#endif


