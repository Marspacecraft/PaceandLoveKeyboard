#include "keyboard.h"
#include "main.h"
#include "usbd_agent.h"
#include <string.h>
#include "gpio.h"
#include "keylayer.h"
#include "tracelog.h"
#include "74hc165d.h"
#include "fonts.h"
#include "st7302.h"
#include "joystick.h"
#include "memery.h"
#include "oled.h"
#include "rgb.h"

#define LCD_SHOW_KEY_1U		2
#define LCD_SHOW_KEY_125U	3
#define LCD_SHOW_KEY_15U	4
#define LCD_SHOW_KEY_175U	5
#define LCD_SHOW_KEY_2U		6
#define LCD_SHOW_KEY_225U	7



#define MOUSE_BUTTON_BITMAP_LEFT	83
#define MOUSE_BUTTON_BITMAP_RIGHT	82

#define BIT_MAP_NUM ((MAX_KEY_NUM%4)?(MAX_KEY_NUM/4):((MAX_KEY_NUM/4)+1))
typedef uint32_t bitmap_t[BIT_MAP_NUM];


typedef enum
{
	FN_KEY_STATIC_FREE = 0,
	FN_KEY_STATIC_PRESSED,
	FN_KEY_STATIC_DEBOUND
}fn_static;


#define KEY_DEBOUNCE_TIME 1	//10 ms
static uint8_t sg_KeyScanCount = 3;//20 ms

static bitmap_t sg_BitMap_Old = {0};
static bitmap_t sg_BitMap_New = {0};
static bitmap_t sg_BitMap_Debound = {0};

static volatile uint8_t sg_BeSilent_flag = false;
static KeySilentModFunc sg_SilentCallBack = NULL;

static hid_keyboard sg_KeyCode_buffer = {0};
;
static fn_static sg_fn_stat = FN_KEY_STATIC_FREE;

void KEY_ShowKeyBit(uint8_t x,uint8_t y,keybit_t bit,uint8_t u);


/************************************************************************************************

	Keybit function

************************************************************************************************/

void For_Each_KeyBit(ForEachBitFunc_f func,bitmap_t bitmap)
{
	uint32_t* map_32 = bitmap;
	keybit_t bit;
	uint8_t i;//32

	for(i=0;i<BIT_MAP_NUM;i++)
	{
		if(map_32[i])
		{
			uint16_t * map_16 = (uint16_t *)&map_32[i];
			uint8_t j;//16
			for(j=0;j<2;j++)
			{
				if(map_16[j])
				{
					uint8_t * map_8 = (uint8_t*)&map_16[j];
					uint8_t k;//8
					for(k=0;k<2;k++)
					{
						if(map_8[k])
						{
							uint8_t l;
							for(l=0;l<8;l++)
							{
								if(map_8[k] & (0x80 >> l))
								{
									bit = i*32 + j*16 + k*8 + l;
									func(bit);
								}
							}
						}
					}
				}
			}
		}
	}

}

void For_Each_KeyBit_Press(ForEachBitFunc_f func)
{
	For_Each_KeyBit(func,sg_BitMap_Old);
}

uint8_t Get_ChangedKeyBitMap(bitmap_t map,bitmap_t bitmap)
{
	uint8_t i;
	uint8_t ret = false;

	for(i=0;i<BIT_MAP_NUM;i++)
	{
		map[i] = sg_BitMap_Old[i] ^ bitmap[i];
		if(map[i])
		{
			ret = true;
		}
	}

	return ret;
}

uint8_t For_Each_KeyBit_Changed(ForEachBitFunc_f func,bitmap_t bitmap,bitmap_t chmap)
{
	
	if(Get_ChangedKeyBitMap(chmap,bitmap))
	{
		For_Each_KeyBit(func,chmap);
		return true;
	}
	return false;
}

bool KeyBit_IsFreeing(keybit_t bitmap)
{
	uint8_t pos = bitmap/8;
	uint8_t bitpos = bitmap%8;
	uint8_t* pOled = (uint8_t*)(sg_BitMap_Old);
	uint8_t* pDebound = (uint8_t*)(sg_BitMap_Debound);

	if((pOled[pos]&(0x80>>bitpos))&&((~pDebound[pos])&(0x80>>bitpos)))
		return true;
	return false;
}

bool KeyBit_IsPressing(keybit_t bitmap)
{
	uint8_t pos = bitmap/8;
	uint8_t bitpos = bitmap%8;
	uint8_t* pOled = (uint8_t*)(sg_BitMap_Old);
	uint8_t* pDebound = (uint8_t*)(sg_BitMap_Debound);

	if((pDebound[pos]&(0x80>>bitpos))&&((~pOled[pos])&(0x80>>bitpos)))
		return true;
	return false;
}

bool KeyBit_IsPressed(keybit_t bitmap)
{
	uint8_t pos = bitmap/8;
	uint8_t bitpos = bitmap%8;
	uint8_t* pOled = (uint8_t*)(sg_BitMap_Old);

	if(pOled[pos]&(0x80>>bitpos))
		return true;
	return false;
}

bool MouseButton_Left_IsPressed(void)
{
	return KeyBit_IsPressed(MOUSE_BUTTON_BITMAP_LEFT);
}

bool MouseButton_Right_IsPressed(void)
{
	return KeyBit_IsPressed(MOUSE_BUTTON_BITMAP_RIGHT);
}


/********************************************************************************************

	other

********************************************************************************************/
 uint8_t* KEY_Get_KeyBitMap_Buffer(void);
 uint8_t* KEY_Get_KeyBitMap_Debounce_Buffer(void);
 
 //if need to debounce and return true,or send key code to host and return false.
 uint8_t KEY_Updata_KeyBitMap_Buffer(void);
 //send key code to host 
 void KEY_Updata_KeyBitMap_Debounce_Buffer(void);


 void KeyCode_Change(keycode_t code,uint8_t pressed)
{
	if(pressed)
	{
		//TRACELOG_DEBUG_PRINTF("SetCodec [%x]",code);
		USBD_Keyboard_SetCodec(&sg_KeyCode_buffer,code);
		//USBD_Print_Keyboard_Report(&sg_KeyCode_buffer);
	}
	else
	{
		//TRACELOG_DEBUG_PRINTF("ResetCodec [%x]",code);
		USBD_Keyboard_ResetCodec(&sg_KeyCode_buffer,code);
		//USBD_Print_Keyboard_Report(&sg_KeyCode_buffer);
	}
}


void KeyBit_Change(keybit_t bit)
{
	uint8_t pressed = KeyBit_IsPressing(bit);

	if(false == sg_BeSilent_flag)
	{
		For_Each_Bit_KeyCode(bit, KeyCode_Change,pressed);
	}
}


void Updata_BitMapOld_2_HIDBuffer(keybit_t bit)
{
	For_Each_Bit_KeyCode(bit, KeyCode_Change,true);
}

void Updata_KeyBitMapBuffer_Old()
{
	memcpy(sg_BitMap_Old,sg_BitMap_Debound,sizeof(bitmap_t));
}

void ResetChanged_DuringDebounce()
{
	for(uint8_t i=0;i<BIT_MAP_NUM;i++)
	{
		uint32_t difbit = ~(sg_BitMap_Debound[i]^sg_BitMap_New[i]);
		sg_BitMap_Debound[i] &= difbit;
	}
}

uint8_t* KEY_Get_KeyBitMap_Buffer(void)
{
	return (uint8_t*)sg_BitMap_New;
}

uint8_t* KEY_Get_KeyBitMap_Debounce_Buffer(void)
{
	return (uint8_t*)sg_BitMap_Debound;
}


uint8_t KEY_Updata_KeyBitMap_Buffer(void)
{	
	bitmap_t chmap;
	//有按键按下，返回进行debounce
	if(Get_ChangedKeyBitMap(chmap,sg_BitMap_New))
	{
		return true;
	}
	//发送之前的HID报告
	//USBD_Keyboard_Send(&sg_KeyCode_buffer);
	return false;
}


void KEY_Updata_KeyBitMap_Debounce_Buffer(void)
{
	static uint16_t updatacount = 0;
	bitmap_t chbitmap;
	uint8_t changed = false;
	
	//去除new和debounce差异
	ResetChanged_DuringDebounce();
	//改变keybit更新到keycode buffer
	changed = For_Each_KeyBit_Changed(KeyBit_Change,sg_BitMap_Debound,chbitmap);
	//发送HID报告
	USBD_Keyboard_Send(&sg_KeyCode_buffer);
	//更新去抖buffer到old buffer，去抖buffer与newbuffer不一致的将不会更新到oldbuffer，等待下次扫描后判断
	Updata_KeyBitMapBuffer_Old();

	//维护keybit的buffer和keycode的buffer一致
	updatacount++;
	if(updatacount > 1000)
	{
		updatacount = 0;
		if(false == sg_BeSilent_flag)
		{
		//	USBD_Keyboard_FreeAll(&sg_KeyCode_buffer);	
		//	For_Each_KeyBit_Press(Updata_BitMapOld_2_HIDBuffer);//更新HIDbuffer
		}
	}
	
	//静默回调
	if(sg_BeSilent_flag && changed && sg_SilentCallBack)
	{
		sg_SilentCallBack((uint32_t*)chbitmap);
	}
}



/********************************************************************************************

	Fn key function

********************************************************************************************/



#define KEYBIT_F1 	1
#define KEYBIT_F2 	2
#define KEYBIT_F3 	3
#define KEYBIT_F4 	4
#define KEYBIT_F5 	5
#define KEYBIT_F6 	6
#define KEYBIT_F7 	7
#define KEYBIT_F8 	8
#define KEYBIT_F9 	9
#define KEYBIT_F10 	10
#define KEYBIT_F11 	11
#define KEYBIT_F12 	12


#define KEYBIT_1 	26
#define KEYBIT_2 	25
#define KEYBIT_3 	24
#define KEYBIT_4 	23
#define KEYBIT_5 	22
#define KEYBIT_6 	21
#define KEYBIT_7 	20
#define KEYBIT_8 	19
#define KEYBIT_9 	18
#define KEYBIT_0 	17

#define KEYBIT_Enter 	42
#define KEYBIT_L 		45
#define KEYBIT_LCTRL 	75




typedef enum
{
	Fn_Mode_None = 0,
	Fn_Mode_LockKeyboard,
	Fn_Mode_SetCombine,
	Fn_Mode_SetKeyLayer,
	Fn_Mode_MouseSwitch,
	Fn_Mode_OpenMSC,
	Fn_Mode_ChangeLEDMode,
	Fn_Mode_BrightUp,//5
		
	Fn_Mode_MAX=13,
}FnMode_e;


typedef enum
{
	Fn_Event_None = 0,
	Fn_Event_Press,
	Fn_Event_Free,
	Fn_Event_KeyChange,
}FnModeEvent_e;

typedef enum
{
	FN_STEP_0 = 0,
	FN_STEP_1,
	FN_STEP_2,
	FN_STEP_3,
	FN_STEP_4,
	FN_STEP_5,
	FN_STEP_6,
	FN_STEP_7,
	FN_STEP_8,
	FN_STEP_9,
	FN_STEP_10,
	FN_STEP_11,
}FnModeStep_e;

typedef void (*FnExitFunc)();

typedef struct
{
	keybit_t 		bit;
	FnExitFunc		exitfunc;
	FnModeStep_e 	step;
	char*	 		modename;
}BitModeMap_t;

void FnKey_ChangeKeyCodeExit(void);
void FnKey_KeyboardLockExit(void);
void FnKey_ChangeLayerExit(void);



static volatile FnMode_e sg_FnModeFlag = Fn_Mode_None;
static uint32_t* sg_FnBitChange = NULL; 
static BitModeMap_t sgc_BitModeMapMnger[Fn_Mode_MAX] = 
{
	{0,NULL,FN_STEP_0,""},
	{KEYBIT_F1,FnKey_KeyboardLockExit,FN_STEP_0,"Lock Keyboard"},//Fn_Mode_LockKeyboard
	{KEYBIT_F2,FnKey_ChangeKeyCodeExit,FN_STEP_0,"Key Map"},//Fn_Mode_SetCombine
	{KEYBIT_F3,FnKey_ChangeLayerExit,FN_STEP_0,"Key Layer"},//Fn_Mode_SetKeyLayer
	{KEYBIT_F4,NULL,FN_STEP_0,"Mouse switch"},//Fn_Mode_MouseSwitch
	{KEYBIT_F5,NULL,FN_STEP_0,"Open MSC"},//Fn_Mode_OpenMSC
	{KEYBIT_F6,NULL,FN_STEP_0,NULL},
	{KEYBIT_F7,NULL,FN_STEP_0,NULL},
	{KEYBIT_F8,NULL,FN_STEP_0,NULL},
	{KEYBIT_F9,NULL,FN_STEP_0,NULL},
	{KEYBIT_F10,NULL,FN_STEP_0,NULL},
	{KEYBIT_F11,NULL,FN_STEP_0,NULL},
	{KEYBIT_F12,NULL,FN_STEP_0,NULL}
};


uint8_t Wait_FnModeKey()
{
	uint8_t i;
	for(i=KEYBIT_F1;i<Fn_Mode_MAX;i++)
	{
		if(NULL == sgc_BitModeMapMnger[i].modename)
			return false;
		
		if(KeyBit_IsPressed(sgc_BitModeMapMnger[i].bit))
		{
			sg_FnModeFlag = (FnMode_e)i;
			return true;
		}
	}
	return false;
}

//当进入Fn模式后，对主机进行按键静默，oldbuffer正常更新，HID数据正常发送，但数据都是0
uint8_t KEY_BeSilent_2_Host(KeySilentModFunc func)
{
	if(sg_BeSilent_flag)
		return false;
	
	sg_BeSilent_flag = true;
	sg_SilentCallBack = func;
	USBD_Keyboard_FreeAll(&sg_KeyCode_buffer);
	USBD_Keyboard_Send(&sg_KeyCode_buffer);
	return true;
}

void KEY_BeDissilent_2_Host(void)
{
	
	For_Each_KeyBit_Press(Updata_BitMapOld_2_HIDBuffer);
	//USBD_Keyboard_Send(&sg_KeyCode_buffer);
	sg_BeSilent_flag = false;
}

void FnKey_Enter(FnModeEvent_e ev);
void KEY_KeyBit_Change_Fn(uint32_t* chbitmap)
{
	sg_FnBitChange = chbitmap;
	FnKey_Enter(Fn_Event_KeyChange);
	sg_FnBitChange = NULL;
}

void FnKey_Exit()
{
	
	if(sg_FnModeFlag < Fn_Mode_MAX)
	{
		if(sgc_BitModeMapMnger[sg_FnModeFlag].exitfunc)
			sgc_BitModeMapMnger[sg_FnModeFlag].exitfunc();
		sgc_BitModeMapMnger[sg_FnModeFlag].step = FN_STEP_0;
	}
	sg_FnModeFlag = Fn_Mode_None;
	KEY_BeDissilent_2_Host();
}



/*****************************************************

	修改键值映射函数
	
*****************************************************/

static keybit_t sg_changekey = MAX_KEYLAYER_NUM;
static keycombine_t sg_changecomb = {0};

#define Fn_SendInfo(BUFF) USBD_VPC_Send((uint8_t*)(BUFF),strlen((BUFF)))
#define Fn_SendInfo_Printf(...) \
	do\
	{\
		char buffer[128];\
		sprintf((char *)buffer,__VA_ARGS__);\
		Fn_SendInfo(buffer);\
	}while(0)


static void SetChangeBit(keybit_t bit)
{
	if(KeyBit_IsPressed(bit))
	{
		sg_changekey = bit;
	}
}

static void SetNewBit(keybit_t bit)
{
	uint8_t i;
	keycode_t code = KeyLayer_GetBitCodec(bit);
	if(KeyBit_IsPressed(bit))
	{
		for(i = 0;i<MAX_KEYCOMBINE_NUM;i++)
		{
			if(0xff == sg_changecomb[i])
			{
				KeyCodeDis_t* name = KeyLayer_GetCodecDiscribe(bit);
				#ifdef I2C_OLED	
					OLED_CharMode_ShowStringAppend(OLED_TITLE_1,"-");
					OLED_CharMode_ShowStringAppend(OLED_TITLE_1,name->upline);
				#endif
				sg_changecomb[i] = code;
				return;
			}
			else if(code == sg_changecomb[i])
			{
				return;
			}
		}
	}
}

void FnKey_KeyboardLockExit()
{
	#ifdef SPI_SHOW_LCD
		LCD_Clear();
	#endif
	
}
void FnKey_ChangeKeyCodeExit()
{
	if((sg_changecomb[0] < 0xff) && (FN_STEP_2 == sgc_BitModeMapMnger[Fn_Mode_SetCombine].step))
	{
		#ifndef SPI_SHOW_LCD
			Fn_SendInfo_Printf("Change  code[%d] to codec[%d-%d-%d-%d]\r\n",KeyLayer_GetBitCodec(sg_changekey)
				,sg_changecomb[0],sg_changecomb[1],sg_changecomb[2],sg_changecomb[3]);
		#endif

		if((sg_changecomb[0] == KEY_CODE_F12)
			&&(sg_changecomb[1] >= KEY_CODE_1)&&(sg_changecomb[1] <= KEY_CODE_0)
			&&(sg_changecomb[2] >= KEY_CODE_1)&&(sg_changecomb[2] <= KEY_CODE_0))//手工输入16进制键值
		{
			sg_changecomb[1] = sg_changecomb[1] - KEY_CODE_1 + 1;
			sg_changecomb[2] = sg_changecomb[2] - KEY_CODE_1 + 1;
			sg_changecomb[1] = sg_changecomb[1] % 10;
			sg_changecomb[2] = sg_changecomb[2] % 10;
			sg_changecomb[0] = (sg_changecomb[1] << 4) + sg_changecomb[2];
			sg_changecomb[1] = 0xff;
		}
		KEY_Set_KeyLayer_Combine(sg_changekey,sg_changecomb);

		#ifdef SPI_SHOW_LCD
			LCD_Clear();
			LCD_ShowString(0, 0, "SUCCESS", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_4_GRAY_0|LCD_SHOW_MODE_GRAY_4_BKG_GRAY_3);
			KEY_ShowKeyBit(0,34,sg_changekey,LCD_SHOW_KEY_1U);
		#endif
		#ifdef I2C_OLED
			{
			OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"SUCCESS");
			}
		#endif
		
		sgc_BitModeMapMnger[Fn_Mode_SetCombine].step++;
	}
	else
	{
		#ifdef SPI_SHOW_LCD
			LCD_Clear();
			LCD_ShowString(0, 0, "No Change", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_4_GRAY_0|LCD_SHOW_MODE_GRAY_4_BKG_GRAY_3);
		#else
			Fn_SendInfo("No Change\r\n");
		#endif
		#ifdef I2C_OLED
			{
			OLED_CharMode_ClearShowOrder();
			OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"No Change");
			}
		#endif
	}
};


void FnKey_ChangeKeyCode()
{	
	if(Fn_Mode_SetCombine != sg_FnModeFlag)
	{
		sgc_BitModeMapMnger[Fn_Mode_SetCombine].step = FN_STEP_0;
		FnKey_Exit();
	}
	
	switch(sgc_BitModeMapMnger[Fn_Mode_SetCombine].step)
	{
		case FN_STEP_0:
			#ifdef I2C_OLED
				OLED_CharMode_Init();
			#endif
			if(KeyLayer_IsLocked())
			{	
				sgc_BitModeMapMnger[Fn_Mode_SetCombine].step = FN_STEP_3;

				#ifndef SPI_SHOW_LCD
					Fn_SendInfo("key layer is locked\r\n");
				#else
					LCD_Clear();
					LCD_ShowString(0, 0, "key layer is locked", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
					LCD_ShowString(0, 0, "Fn key to exit", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
				#endif
				#ifdef I2C_OLED
					{
					OLED_CharMode_ClearShowOrder();
					OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"key layer is locked");
					OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Fn key to exit");
					}
				#endif
				break;
			}
			#ifndef SPI_SHOW_LCD
					Fn_SendInfo(sgc_BitModeMapMnger[Fn_Mode_SetCombine].modename);
					Fn_SendInfo("Input the botton to map\r\n");
			#else
				LCD_Clear();
				LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetCombine].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
				LCD_ShowString(0, 16, (unsigned char*)"Input the botton to map", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
			#endif
			#ifdef I2C_OLED
					{
					OLED_CharMode_ClearShowOrder();
					OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetCombine].modename);
					OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Input the botton to map");
					}
			#endif
			sg_changekey = MAX_KEYLAYER_NUM;
			sgc_BitModeMapMnger[Fn_Mode_SetCombine].step++;
			break;
		case FN_STEP_1:
			if(sg_FnBitChange)
			{
				For_Each_KeyBit(SetChangeBit,sg_FnBitChange);
				if(sg_changekey < KeyLayer_GetKeyNum())
				{	
					#ifndef SPI_SHOW_LCD
						Fn_SendInfo_Printf("Change bit[%d] codec[%d]\r\n",sg_changekey,KeyLayer_GetBitCodec(sg_changekey));
						Fn_SendInfo("Input the new bottom\r\n");
					#else
						KEY_ShowKeyBit(0,34,sg_changekey,LCD_SHOW_KEY_1U);
						LCD_ShowString(44, 43, "Input the new bottom",FONT_ASCII_11x16,LCD_SHOW_MODE_GRAY_2);
					#endif
					#ifdef I2C_OLED
						{
						OLED_CharMode_ClearShowOrder();
						OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Input the new bottom");
						OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"New bottom:");
						}
					#endif
					memset(sg_changecomb,0xff,sizeof(sg_changecomb));
					sgc_BitModeMapMnger[Fn_Mode_SetCombine].step++;
				}
			}
			break;
		case FN_STEP_2:
			if(sg_FnBitChange)
			{
				For_Each_KeyBit(SetNewBit,sg_FnBitChange);
			}
			//break;
		default :
			
			break;
	}
}

/*****************************************************

	改变映射层
	
*****************************************************/

uint8_t KEY_ShowAllLayer()
{
	uint8_t i,n=0;
	#ifndef SPI_SHOW_LCD
		Fn_SendInfo_Printf("Working layer %d\r\n",KEY_GetCurrentKeyLayer()+1);
	#else
		char buffer[128];
		sprintf(( char *)buffer,"Working layer %d",KEY_GetCurrentKeyLayer()+1);
		LCD_Clear();
		LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
		LCD_ShowString(0, 16,(unsigned char *)buffer, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);

	#endif

	
	for(i=1;i<=MAX_KEYLAYER_NUM;i++)
	{
		uint8_t* name = KEY_Get_KeyLayer_Name(i-1);
		if(name)
		{
			#ifndef SPI_SHOW_LCD
				Fn_SendInfo_Printf("L%d:%s\r\n",i,name);
			#else
				sprintf(( char *)buffer,"L%d:%s",i,name);
				LCD_ShowString(0, i*16+16,(unsigned char *)buffer, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
			#endif
			n++;
		}
	}
	if(0 == n)
	{
		#ifndef SPI_SHOW_LCD
			Fn_SendInfo("Using default,No Layer\r\n");
		#else
			LCD_Clear();
			LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
			LCD_ShowString(0, 18,"Using default,No Layer", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
		#endif
		#ifdef I2C_OLED
			{
			OLED_CharMode_ClearShowOrder();
			OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Using default,No Layer");
			}
		#endif
		return 0;
	}
	return 1;
}

void FnKey_ChangeLayerExit()
{	
	#ifndef SPI_SHOW_LCD
	Fn_SendInfo("Change layer exit!\r\n");
	#endif
	sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step = FN_STEP_0;
}
void KEY_ChangeLayer()
{
	uint8_t layer;
	if((KEYBIT_1 < sg_changekey) || (KEYBIT_8 > sg_changekey))
		goto ERROUT;

	layer = KEYBIT_8 + 7 - sg_changekey;
	if(NULL == KEY_Get_KeyLayer_Name(layer))
		goto ERROUT;

	if(KEY_Change2KeyLayer(layer))
	{
		#ifndef SPI_SHOW_LCD
			Fn_SendInfo("SUCCSEE!\r\n");
		#else
			LCD_Clear();
			LCD_ShowString(0, 0, "SUCCESS", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_4_GRAY_0|LCD_SHOW_MODE_GRAY_4_BKG_GRAY_3);
		#endif
		#ifdef I2C_OLED
			{
			OLED_CharMode_ClearShowOrder();
			OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"SUCCSEE");
			}
		#endif
		return;
	}	
ERROUT:
	#ifndef SPI_SHOW_LCD
		Fn_SendInfo("Layer num error!\r\n");
	#else
		LCD_Clear();
		LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
		LCD_ShowString(0, 18,"Layer num error!", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
	#endif

	#ifdef I2C_OLED
		{
		OLED_CharMode_ClearShowOrder();
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Layer num error!");
		}
	#endif

}

static uint8_t layername[32];
static uint8_t layerpos;

uint8_t KEY_SaveLayerName()
{
	if(sg_changekey == KEYBIT_Enter)
	{
		if(layerpos == 0)
		{	
			#ifndef SPI_SHOW_LCD
				Fn_SendInfo("No input!\r\n");
			#else
				LCD_Clear();
				LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
				LCD_ShowString(0, 18,"No input!", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
			#endif
			#ifdef I2C_OLED
				{
				OLED_CharMode_ClearShowOrder();
				OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"No input!");
				}
			#endif
			return 1;
		}
		
	NAME_OK:

		layername[layerpos] = 0;
		KEY_Set_CurrentKeyLayer_Name(layername);
		#ifndef SPI_SHOW_LCD
			Fn_SendInfo("Save to current layer---1\r\n");
			Fn_SendInfo("Save to new layer-------2\r\n");
		#else
			LCD_Clear();
			LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
			LCD_ShowString(0, 18,"Save to current layer---1", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
			LCD_ShowString(0, 36,"Save to new layer-------2", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
		#endif
		#ifdef I2C_OLED
			{
			OLED_CharMode_ClearShowOrder();
			OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
			OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Save to current layer");
			OLED_CharMode_ShowString(OLED_TITLE_2,(uint8_t*)"Save to new layer");
			}
		#endif
		return 1;
	}

	if(KeyBit_IsPressed(sg_changekey))
	{
		layername[layerpos] = sg_changekey;
		layerpos++;
		if(layerpos>=31)
			goto NAME_OK;
	}
	return 0;
	
}

uint8_t KEY_SaveLayer()
{
	if(sg_changekey == KEYBIT_1)
	{
		if(0 == KEY_SaveCurrentKeyLayer())
			goto Err_GoOut;
		
		goto Ok_GotOut;
	}
	else if(sg_changekey == KEYBIT_2)
	{
		if(0 == KEY_Save2KeyLayer(MAX_KEYLAYER_NUM))
			goto Err_GoOut;
		
		goto Ok_GotOut;
	}
	return 0;

Err_GoOut:
	#ifndef SPI_SHOW_LCD
		Fn_SendInfo("Save layer error\r\n");
	#else
		LCD_Clear();
		LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
		LCD_ShowString(0, 18,"Save layer error", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
	#endif
	#ifdef I2C_OLED
		{
		OLED_CharMode_ClearShowOrder();
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
		OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Save layer error");
		}
	#endif
	return 1;
Ok_GotOut:
	#ifndef SPI_SHOW_LCD
		Fn_SendInfo("SUCCSEE!\r\n");
	#else
		LCD_Clear();
		LCD_ShowString(0, 0, "SUCCESS", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_4_GRAY_0|LCD_SHOW_MODE_GRAY_4_BKG_GRAY_3);
	#endif
	#ifdef I2C_OLED
		{
		OLED_CharMode_ClearShowOrder();
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"SUCCESS");
		}
	#endif
	return 1;
}

void FnKey_DeleteLayer()
{
	uint8_t layer;
	if((KEYBIT_1 < sg_changekey) || (KEYBIT_8 > sg_changekey))
		goto ERR;

	layer = KEYBIT_8 + 7 - sg_changekey;
	KeyLayer_Delete(layer);
	#ifndef SPI_SHOW_LCD
		Fn_SendInfo("SUCCSEE!\r\n");
	#else
		LCD_Clear();
		LCD_ShowString(0, 0, "SUCCESS", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_4_GRAY_0|LCD_SHOW_MODE_GRAY_4_BKG_GRAY_3);
	#endif
	#ifdef I2C_OLED
		{
		OLED_CharMode_ClearShowOrder();
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"SUCCESS");
		}
	#endif
	return;
ERR:
	#ifndef SPI_SHOW_LCD
		Fn_SendInfo("Layer num error!\r\n");
	#else
		LCD_Clear();
		LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
		LCD_ShowString(0, 18,"Layer num error!", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
	#endif
	#ifdef I2C_OLED
		{
		OLED_CharMode_ClearShowOrder();
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Layer num error!");
		}
	#endif
	return ;
}

void KEY_SetKeyLayer()
{
	if(Fn_Mode_SetKeyLayer != sg_FnModeFlag)
	{
		sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step = FN_STEP_0;
		FnKey_Exit();
	}
	
	if(sg_FnBitChange)
	{
		For_Each_KeyBit(SetChangeBit,sg_FnBitChange);
	}
	
	switch(sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step)
	{
		case FN_STEP_0:
				#ifndef SPI_SHOW_LCD
					Fn_SendInfo(sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
					Fn_SendInfo("Change layer--------1\r\n");
					Fn_SendInfo("Save layer----------2\r\n");
					Fn_SendInfo("Lock layer----------3\r\n");
					Fn_SendInfo("Unlock layer--------4\r\n");
					Fn_SendInfo("Delete layer--------5\r\n");
				#else
					LCD_Clear();
					LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
					LCD_ShowString(0, 18,"Change layer--------1", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
					LCD_ShowString(0, 36,"Save layer----------2", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
					LCD_ShowString(0, 54,"Lock layer----------3", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
					LCD_ShowString(0, 72,"Unlock layer--------4", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
					LCD_ShowString(0, 90,"Delete layer--------5", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
				#endif

				#ifdef I2C_OLED
					{
					OLED_CharMode_Init();
					OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
					OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Change layer");
					OLED_CharMode_ShowString(OLED_TITLE_2,(uint8_t*)"Save layer");
					OLED_CharMode_ShowString(OLED_TITLE_3,(uint8_t*)"Lock layer");
					OLED_CharMode_ShowString(OLED_TITLE_4,(uint8_t*)"Unlock layer");
					OLED_CharMode_ShowString(OLED_TITLE_5,(uint8_t*)"Delete layer");
					}
				#endif

			sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step++;
			break;
		case FN_STEP_1:
			if(sg_changekey < KeyLayer_GetKeyNum())
			{	
				switch(sg_changekey)
				{
					case KEYBIT_1:
						sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step = FN_STEP_2;
						break;
					case KEYBIT_2:
						sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step = FN_STEP_4;
						break;
					case KEYBIT_3:
						sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step = FN_STEP_7;
						break;
					case KEYBIT_4:
						sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step = FN_STEP_8;
						break;
					case KEYBIT_5:
						sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step = FN_STEP_9;
						break;

				}
			}
		
			break;
		case FN_STEP_2://change layer
			if(0 == KEY_ShowAllLayer())
			{
				FnKey_Exit();
				break;
			}
			sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step++;
			break;
		case FN_STEP_3:
			KEY_ChangeLayer();
			FnKey_Exit();
			break;

		case FN_STEP_4://save layer
			#ifndef SPI_SHOW_LCD
				Fn_SendInfo("Input layer name\r\n");
				Fn_SendInfo("Max name len:26,Enter to finish\r\n");
			#else
				LCD_Clear();
				LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
				LCD_ShowString(0, 18,"Input layer name", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
				LCD_ShowString(0, 36,"Max name len:26,Enter to finish", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
			#endif
			#ifdef I2C_OLED
				{
				OLED_CharMode_ClearShowOrder();
				OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
				OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Input layer name");
				OLED_CharMode_ShowString(OLED_TITLE_2,(uint8_t*)"Max name len:26,Enter to finish");
				}
			#endif
			layerpos = 0;
			layername[31] = 0;
			sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step++;
			break;
		case FN_STEP_5:
			if(KEY_SaveLayerName())
			{
				sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step++;
			}
			break;
		case FN_STEP_6:
			if(KEY_SaveLayer())
			{
				FnKey_Exit();
			}
			break;
		case FN_STEP_7://Lock layer
			KeyLayer_SetLock(1);
			#ifndef SPI_SHOW_LCD
				Fn_SendInfo("Lock current layer\r\n");
			#else
				LCD_Clear();
				LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
				LCD_ShowString(0, 18,"Lock current layer", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
			#endif

			#ifdef I2C_OLED
				{
				OLED_CharMode_ClearShowOrder();
				OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
				OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Lock current layer");
				}
			#endif
			FnKey_Exit();
			break;
		case FN_STEP_8://Unlock layer
			KeyLayer_SetLock(0);
			#ifndef SPI_SHOW_LCD
				Fn_SendInfo("Unlock current layer\r\n");
			#else
				LCD_Clear();
				LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
				LCD_ShowString(0, 18,"Unlock current layer", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
			#endif
			#ifdef I2C_OLED
				{
				OLED_CharMode_ClearShowOrder();
				OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
				OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Unlock current layer");
				}
			#endif
			FnKey_Exit();
			break;
		case FN_STEP_9://Delete layer
				#ifndef SPI_SHOW_LCD
					Fn_SendInfo("Input layer number\r\n");
				#else
					LCD_Clear();
					LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
					LCD_ShowString(0, 18,"Input layer number", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
				#endif
				#ifdef I2C_OLED
				{
				OLED_CharMode_ClearShowOrder();
				OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].modename);
				OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)"Input layer number");
				}
			#endif
			if(0 == KEY_ShowAllLayer())
			{
				FnKey_Exit();
				break;
			}
			sgc_BitModeMapMnger[Fn_Mode_SetKeyLayer].step++;
			break;
		case FN_STEP_10:
			FnKey_DeleteLayer();
			FnKey_Exit();
			break;
		default :
			
			break;
	}
}

void KEY_MouseSwitch()
{
	static uint8_t mousestart = 1;
	#ifdef I2C_OLED
		OLED_CharMode_Init();
	#endif
	if(mousestart)
	{
		#ifdef I2C_OLED
			OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"Mouse Stop SUCCESS");
		#endif
		Joystick_Stop();
		mousestart = 0;
	}
	else
	{
		#ifdef SPI_SHOW_LCD
			LCD_Clear();
			LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_MouseSwitch].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
		#endif
		Joystick_Calibrate();
		#ifndef SPI_SHOW_LCD
			Fn_SendInfo("SUCCSEE!\r\n");
		#else
			LCD_Clear();
			LCD_ShowString(0, 0, "SUCCESS", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_4_GRAY_0|LCD_SHOW_MODE_GRAY_4_BKG_GRAY_3);
		#endif
		#ifdef I2C_OLED
			{
			OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)"SUCCESS");
			}
		#endif
		mousestart = 1;
	}
	FnKey_Exit();
}

void KEY_OpenMSC()
{

	#ifdef SPI_SHOW_LCD
		LCD_Clear();
		LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_OpenMSC].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
		LCD_ShowString(0, 18, "Plug the USB back in!", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
	#else
		Fn_SendInfo("Plug the USB back in!\r\n");
	#endif
	#ifdef I2C_OLED
		{
		OLED_CharMode_Init();
		OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_OpenMSC].modename);
		OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*) "Plug the USB back in!");
		}
	#endif

	MEM_FlagBit_Set(MEM_FLAG_USBMSC);
	MEM_FlagBit_Set(MEM_FLAG_EPAPBIN);
	FnKey_Exit();
}


void FnKey_F1Func()
{
	if(!KeyBit_IsPressed(KEYBIT_LCTRL))
		return;
	
	switch(sg_changekey)
	{
		case KEYBIT_L:
			RGB_Mode_Change();
			break;
		default:
			return;
	}
	sgc_BitModeMapMnger[Fn_Mode_LockKeyboard].step++;
}



#ifdef TEST_FNKEY
void test_Fn(void);
#endif

void FnKey_Enter(FnModeEvent_e ev)
{
	if(Fn_Mode_None == sg_FnModeFlag)
	{
		switch(ev)
		{
			case Fn_Event_Press:
				KEY_BeSilent_2_Host(KEY_KeyBit_Change_Fn);
				#ifdef I2C_OLED
					OLED_CharMode_DeInit();
				#endif
			case Fn_Event_KeyChange:
				Wait_FnModeKey();
				break;
			case Fn_Event_Free:
				FnKey_Exit();
				break;
			default:
				break;
		}
	}
	else
	{
		if((Fn_Event_KeyChange ==  ev) && (sg_BeSilent_flag))
		{
			switch(sg_FnModeFlag)
			{
				case Fn_Mode_LockKeyboard:
					
					#ifdef TEST_FNKEY
						test_Fn();					
					#endif
					
					if(FN_STEP_0 == sgc_BitModeMapMnger[Fn_Mode_LockKeyboard].step)
					{
						#ifdef SPI_SHOW_LCD
							LCD_Clear();
							LCD_ShowString(0, 0, (unsigned char*)sgc_BitModeMapMnger[Fn_Mode_LockKeyboard].modename, FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2_INVERSION);
							LCD_ShowString(0, 18,"Fn key to exit", FONT_ASCII_11x16, LCD_SHOW_MODE_GRAY_2);
						#endif
						#ifdef I2C_OLED
							{
							OLED_CharMode_Init();
							OLED_CharMode_ShowString(OLED_TITLE_0,(uint8_t*)sgc_BitModeMapMnger[Fn_Mode_LockKeyboard].modename);
							OLED_CharMode_ShowString(OLED_TITLE_1,(uint8_t*)PALK_VERSION);
							OLED_CharMode_ShowStringAppend(OLED_TITLE_1,(uint8_t*)" Fn key to exit");
							}
						#endif
						TRACELOG_INFO(PALK_VERSION);
						sgc_BitModeMapMnger[Fn_Mode_LockKeyboard].step++;
					}
					else if(FN_STEP_1 == sgc_BitModeMapMnger[Fn_Mode_LockKeyboard].step)
					{
						if(sg_FnBitChange)
						{
							For_Each_KeyBit(SetChangeBit,sg_FnBitChange);
						}
						FnKey_F1Func();
					}
					
					break;
				case Fn_Mode_SetCombine:		
					FnKey_ChangeKeyCode();
					break;
				case Fn_Mode_SetKeyLayer:
					KEY_SetKeyLayer();
					break;
				case Fn_Mode_MouseSwitch:
					KEY_MouseSwitch();
					break;
				case Fn_Mode_OpenMSC:
					KEY_OpenMSC();
					break;
			
			}
		}
		else if(Fn_Event_Press == ev)
		{
			FnKey_Exit();
		}
	}
}


void KEY_Fn_Enter(eApp_Event ev,uint8_t* data,uint16_t len)
{
	
	tSched_Itemer item;
	//TRACELOG_DEBUG_PRINTF("KEY_Fn_Enter[%d]...",sg_fn_stat);
	if(sg_fn_stat < FN_KEY_STATIC_DEBOUND)
	{
		sg_fn_stat = FN_KEY_STATIC_DEBOUND;
		item.data = NULL;
		item.event = KEY_FN_INTERRUPT;
		item.handler = KEY_Fn_Enter;
		item.size = 0;
		push_2_ms_scheduler(&item,KEY_DEBOUNCE_TIME);
	}
	else
	{
		if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(FN_KEY_WAKEUP_GPIO_Port,FN_KEY_WAKEUP_Pin))
		{
			sg_fn_stat = FN_KEY_STATIC_FREE;
			FnKey_Enter(Fn_Event_Free);
		}
		else
		{
			sg_fn_stat = FN_KEY_STATIC_PRESSED;
			FnKey_Enter(Fn_Event_Press);
		}	
	}

}

void KEY_Fn_Interrupt()
{
	tSched_Itemer item;
	item.data = NULL;
	item.event = KEY_FN_INTERRUPT;
	item.handler = KEY_Fn_Enter;
	item.size = 0;
	#if 0

	TRACELOG_DEBUG_PRINTF("KEY_Fn_Interrupt.ret[%d]..",push_2_scheduler(&item));
	#else
	push_2_scheduler(&item);

	#endif
	

}

/********************************************************************************************

	Init function

********************************************************************************************/


static void KEY_ResetBitMap()
{
	memset((uint8_t*)sg_BitMap_Old,0,sizeof(bitmap_t));
	memset((uint8_t*)sg_BitMap_New,0,sizeof(bitmap_t));
	memset((uint8_t*)sg_BitMap_Debound,0,sizeof(bitmap_t));
}
void KEY_Init()
{

	HAL_GPIO_WritePin(KEY_PL_GPIO_Port, KEY_PL_Pin, GPIO_PIN_RESET);

	KEY_KeyLayer_Init();

	KEY_ResetBitMap();
	USBD_Keyboard_FreeAll(&sg_KeyCode_buffer);
}


/********************************************************************************************

	Key task

********************************************************************************************/
#include "scheduler.h"

void KEY_Scan_Start()
{
	tSched_Itemer item;

	item.event = SCAN_TIMEROUT;
	item.handler = Keyboard_Task;
	push_2_ms_scheduler(&item,sg_KeyScanCount);
}

static void KEY_Debounce_Start()
{
	tSched_Itemer item;

	item.event = DEBOUNCE_TIMEROUT;
	item.handler = Keyboard_Task;

	push_2_ms_scheduler(&item,KEY_DEBOUNCE_TIME);
}



static void KEY_ScanTimerOut()
{
	KEY_74HC165D_Scan(KEY_Get_KeyBitMap_Buffer(),MAX_KEYBIT_SIZE);
	
	if(KEY_Updata_KeyBitMap_Buffer())
	{
		KEY_Debounce_Start();
	}
	else
	{
		KEY_Scan_Start();
	}
}

static void KEY_DebounceTimerOut()
{
	KEY_74HC165D_Scan(KEY_Get_KeyBitMap_Debounce_Buffer(),MAX_KEYBIT_SIZE);
	KEY_Updata_KeyBitMap_Debounce_Buffer();
	KEY_Scan_Start();
}


void Keyboard_Task(eApp_Event event,uint8_t* data,uint16_t len)
{

	switch(event)
	{
		case SCAN_TIMEROUT:
			KEY_ScanTimerOut();
			//TRACELOG_DEBUG_PRINTF("ScanTimerOut [0x%x]",sg_BitMap_New[0]);	
		break;
		case DEBOUNCE_TIMEROUT:			
			KEY_DebounceTimerOut();
			//TRACELOG_DEBUG_PRINTF("DebounceTimerOut [0x%x]",sg_BitMap_Debound[0]);
		break;
	#ifdef TEST_KEYTASKSCHEDULER
		case KEY_TASK_TEST:
			KEY_74HC165D_Scan(KEY_Get_KeyBitMap_Buffer(),MAX_KEYBIT_SIZE);
			TRACELOG_DEBUG_PRINTF("Do Keyboard_Task [0x%x][0x%x][0x%x][0x%x]",sg_BitMap_New[0],sg_BitMap_New[1],sg_BitMap_New[2],sg_BitMap_New[3]);	
			void KEY_StartKeyTaskTest(void);
			KEY_StartKeyTaskTest();
		break;
	#endif
	}
}

#ifdef TEST_KEYTASKSCHEDULER

void KEY_StartKeyTaskTest(void)
{
	tSched_Itemer item;

	item.event = (eApp_Event)KEY_TASK_TEST;
	item.handler = (App_Sched_Handler)Keyboard_Task;
	push_2_ms_scheduler(&item,sg_KeyScanCount);
}

void test_TimerRead74HC()
{
	TRACELOG_DEBUG("\ttest_TimerRead74HC...");
	KEY_StartKeyTaskTest();
	while(1)
	{
		do_schedule();
		HAL_Delay(1);
	}

	TRACELOG_DEBUG("\ttest_TimerRead74HC end...");
}

void test_KeyTimerOut()
{
	TRACELOG_DEBUG("\ttest_KeyTimerOut...");
	KEY_Scan_Start();
	while(1)
	{
		do_schedule();
		HAL_Delay(1);
	}

	TRACELOG_DEBUG("\ttest_KeyTimerOut end...");

}

void testcase_keytaskscheduler(void)
{
	TRACELOG_DEBUG("Running testcase_keytaskscheduler...");
	//test_TimerRead74HC();
	test_KeyTimerOut();

	TRACELOG_DEBUG("testcase_keytaskscheduler end...");
}


#endif



#ifdef TEST_FNKEY

void test_Fn()
{
	TRACELOG_DEBUG("\ttest_Fn...");
	if(KeyBit_IsPressed(KEYBIT_F2))
	{
		TRACELOG_DEBUG("\t\tF2 pressed...");
	}
	else
	{
		TRACELOG_DEBUG("\t\tF2 not pressed...");
	}

	TRACELOG_DEBUG("\ttest_Fn end...");
}

void test_ShowKey()
{
	KEY_ShowKeyBit(0,0,8,2);
}

void testcase_Fnkey()
{
	TRACELOG_DEBUG("Running testcase_Fnkey...");
	//test_Fn();
	test_ShowKey();
}

#endif


void KEY_SetScanHz(uint16_t hz)
{
	if(hz > 1000)
		hz = 1000;
	if(hz < 100)
		hz = 100;
	
	sg_KeyScanCount = 1000/hz ;
}

void RGB_SetDisplayHz(uint16_t hz)
{
	if(hz > 60)
		hz = 60;
	if(hz < 15)
		hz = 15;

	//RGB_DISPLAY_TIMEER_PRESCALER = RGB_DISPLAY_TIMEER_PRESCALER_60HZ * 60 / hz;
	//RGB_StartTimer(RGB_DISPLAY_TIMEER_PRESCALER);
}



void KEY_ShowKeyBit(uint8_t x,uint8_t y,keybit_t bit,uint8_t u)
{
	unsigned char key[10];
	KeyCodeDis_t* discribe;

	key[0] = 1;
	memset(&key[1],2,u);
	key[u+1] = 3;
	key[u+2] = 0;
	LCD_ShowString(x, y, key, FONT_KEY_10x40, LCD_SHOW_MODE_GRAY_2);

	discribe = KeyLayer_GetCodecDiscribe(bit);

	LCD_ShowString(x+4, y+4, discribe->upline, discribe->type[0], LCD_SHOW_MODE_GRAY_2);
	LCD_ShowString(x+4, y+20, discribe->downline, discribe->type[1], LCD_SHOW_MODE_GRAY_2);

	LCD_Flush();
}


