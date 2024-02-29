
#include <string.h>
#include "keylayer.h"
#include "memery.h"
#include "fonts.h"




#define KEYCOMBINE_1(CODE1) 					{(CODE1),KEY_CODE_NONE,KEY_CODE_NONE,KEY_CODE_NONE}
#define KEYCOMBINE_2(CODE1,CODE2) 				{(CODE1),(CODE2),KEY_CODE_NONE,KEY_CODE_NONE}
#define KEYCOMBINE_3(CODE1,CODE2,CODE3) 		{(CODE1),(CODE2),(CODE3),KEY_CODE_NONE}
#define KEYCOMBINE_4(CODE1,CODE2,CODE3,CODE4) {(CODE1),(CODE2),(CODE3),(CODE3)}

#pragma pack(push)
#pragma pack (1)




__align(4) typedef struct
{
#define KEYLAYER_NAME_LEN 32// align 4
#define KEYLAYER_INIT_FLAG ((uint16_t)0xfe4d)// align 4

	uint8_t name[KEYLAYER_NAME_LEN];//32
	uint16_t initfg;
	uint8_t lock;
	uint8_t num;
	keycombine_t combine[MAX_KEY_NUM];//416 byte
}KeyLayer;//452 byte

__align(4) typedef struct
{
	uint8_t working;
//	uint8_t num;
	uint8_t enable[MAX_KEYLAYER_NUM+3];//11 byte align 4
	KeyLayer keylayer;//452 byte
}KeyLayerManager_t;//464 byte

#define KEYLAYER_MSG_ADDR	CONFIG_KEYLAYER_ADDR
#define KEYLAYERS_ADDR		(CONFIG_KEYLAYER_ADDR + sizeof(KeyLayerManager_t))

//totall intflash size: sizeof(KeyLayerManager_t)+MAX_KEYLAYER_NUM*sizeof(KeyLayer)=464+8*452=4080byte


#pragma pack (pop)


const static KeyLayer sg_KeyLayerDef = 
{
		.lock = true,
		.name = {"Default key map layer.(Windows)"},
		.num = 84,
		.combine = {KEYCOMBINE_1(KEY_CODE_ESC) ,KEYCOMBINE_1(KEY_CODE_F1), KEYCOMBINE_1(KEY_CODE_F2), KEYCOMBINE_1(KEY_CODE_F3), KEYCOMBINE_1(KEY_CODE_F4), \
			KEYCOMBINE_1(KEY_CODE_F5), KEYCOMBINE_1(KEY_CODE_F6), KEYCOMBINE_1(KEY_CODE_F7), KEYCOMBINE_1(KEY_CODE_F8), KEYCOMBINE_1(KEY_CODE_F9), \
			KEYCOMBINE_1(KEY_CODE_F10), KEYCOMBINE_1(KEY_CODE_F11), KEYCOMBINE_1(KEY_CODE_F12),  \
			KEYCOMBINE_1(KEY_CODE_DELETE),//line1 14 key
		KEYCOMBINE_1(KEY_CODE_BACKSPACE), KEYCOMBINE_1(KEY_CODE_EQUAL), KEYCOMBINE_1(KEY_CODE_MINUS), KEYCOMBINE_1(KEY_CODE_0), KEYCOMBINE_1(KEY_CODE_9), \
			KEYCOMBINE_1(KEY_CODE_8), KEYCOMBINE_1(KEY_CODE_7), KEYCOMBINE_1(KEY_CODE_6), KEYCOMBINE_1(KEY_CODE_5), KEYCOMBINE_1(KEY_CODE_4), KEYCOMBINE_1(KEY_CODE_3), \
			KEYCOMBINE_1(KEY_CODE_2), KEYCOMBINE_1(KEY_CODE_1),KEYCOMBINE_1(KEY_CODE_GRAVE),//line2 14 key
		KEYCOMBINE_1(KEY_CODE_TAB), KEYCOMBINE_1(KEY_CODE_Q), KEYCOMBINE_1(KEY_CODE_W), KEYCOMBINE_1(KEY_CODE_E), KEYCOMBINE_1(KEY_CODE_R), \
			KEYCOMBINE_1(KEY_CODE_T), KEYCOMBINE_1(KEY_CODE_Y), KEYCOMBINE_1(KEY_CODE_U), KEYCOMBINE_1(KEY_CODE_I), KEYCOMBINE_1(KEY_CODE_O), KEYCOMBINE_1(KEY_CODE_P), \
			KEYCOMBINE_1(KEY_CODE_LEFTBRACE), KEYCOMBINE_1(KEY_CODE_RIGHTBRACE), KEYCOMBINE_1(KEY_CODE_NONE),//line3 14 key
		KEYCOMBINE_1(KEY_CODE_ENTER), KEYCOMBINE_1(KEY_CODE_APOSTROPHE), KEYCOMBINE_1(KEY_CODE_SEMICOLON), KEYCOMBINE_1(KEY_CODE_L), KEYCOMBINE_1(KEY_CODE_K), \
			KEYCOMBINE_1(KEY_CODE_J), KEYCOMBINE_1(KEY_CODE_H), KEYCOMBINE_1(KEY_CODE_G), KEYCOMBINE_1(KEY_CODE_F), KEYCOMBINE_1(KEY_CODE_D), KEYCOMBINE_1(KEY_CODE_S), \
			KEYCOMBINE_1(KEY_CODE_A), KEYCOMBINE_1(KEY_CODE_CAPSLOCK),//line4 13 key
		KEYCOMBINE_1(KEY_CODE_LEFTSHIFT), KEYCOMBINE_1(KEY_CODE_Z), KEYCOMBINE_1(KEY_CODE_X), KEYCOMBINE_1(KEY_CODE_C), KEYCOMBINE_1(KEY_CODE_V), \
			KEYCOMBINE_1(KEY_CODE_B), KEYCOMBINE_1(KEY_CODE_N), KEYCOMBINE_1(KEY_CODE_M), KEYCOMBINE_1(KEY_CODE_COMMA), KEYCOMBINE_1(KEY_CODE_DOT), \
			KEYCOMBINE_1(KEY_CODE_SLASH), KEYCOMBINE_1(KEY_CODE_BACKSLASH), KEYCOMBINE_1(KEY_CODE_UP),//line5 13 key
		KEYCOMBINE_1(KEY_CODE_RIGHT), KEYCOMBINE_1(KEY_CODE_DOWN), KEYCOMBINE_1(KEY_CODE_LEFT), KEYCOMBINE_1(KEY_CODE_RIGHTALT), KEYCOMBINE_1(KEY_CODE_SPACE), \
			KEYCOMBINE_1(KEY_CODE_LEFTALT), KEYCOMBINE_1(KEY_CODE_LEFTMETA), KEYCOMBINE_1(KEY_CODE_LEFTCTRL), \
			KEYCOMBINE_1(KEY_CODE_V), KEYCOMBINE_1(KEY_CODE_C), KEYCOMBINE_1(KEY_CODE_RIGHTCTRL),//line6 11 key
		KEYCOMBINE_1(KEY_CODE_S), KEYCOMBINE_1(KEY_CODE_A), KEYCOMBINE_1(KEY_CODE_X),  // line5 3 key
		KEYCOMBINE_1(KEY_CODE_NONE), KEYCOMBINE_1(KEY_CODE_NONE)//line4 2 key
	//84
	}

};
KeyCodeDis_t* KeyLayer_GetCodecDiscribe(keybit_t bit);

static KeyLayerManager_t sg_KeyLayer_Mnger = {0};
static KeyLayer *sg_Working_KeyLayer = &sg_KeyLayer_Mnger.keylayer;


void For_Each_Bit_KeyCode(keybit_t bit,ForEachCodeFunc_f func,uint8_t pressed)
{
	if(bit >= sg_Working_KeyLayer->num)
		return;
	
	keycode_t* comb = sg_Working_KeyLayer->combine[bit];
	for(uint8_t i=0;i<MAX_KEY_NUM;i++)
	{
		if(comb[i])
		{
			func(comb[i],pressed);
		}
		else
		{
			break;
		}
	}
}


/********************************************************************************************

	Key layer map function

********************************************************************************************/


#define KEY_Set_CurrentKeyLayer_Lock(lock) {sg_KeyLayer_Mnger.keylayer.lock = (lock);}

#define KEY_Save_KeyLayerManager() {IntFlash_Write(KEYLAYER_MSG_ADDR,(uint8_t *)&sg_KeyLayer_Mnger,sizeof(sg_KeyLayer_Mnger));}
#define KEY_Load_KeyLayerManager() {IntFlash_Read(KEYLAYER_MSG_ADDR,(uint8_t *)&sg_KeyLayer_Mnger,sizeof(sg_KeyLayer_Mnger));}
#define KEY_Save_KeyLayer(LAYER) {IntFlash_Write(KEYLAYERS_ADDR+(LAYER)*sizeof(KeyLayer),(uint8_t *)&sg_KeyLayer_Mnger.keylayer,sizeof(KeyLayer));}
#define KEY_Load_KeyLayer(LAYER) {IntFlash_Read(KEYLAYERS_ADDR+(LAYER)*sizeof(KeyLayer),(uint8_t *)&sg_KeyLayer_Mnger.keylayer,sizeof(KeyLayer));}

static uint8_t Get_EmptyLayer()
{
	uint8_t i;
	for(i=0;i<MAX_KEYLAYER_NUM;i++)
	{
		if(sg_KeyLayer_Mnger.enable[i])
			continue;
		return i;
	}
	return MAX_KEYLAYER_NUM;
}


uint8_t KEY_Save2KeyLayer(uint8_t layer)
{
	if(layer >= MAX_KEYLAYER_NUM)
	{
		layer = Get_EmptyLayer();
		if(layer >= MAX_KEYLAYER_NUM)
			return false;
	}
	sg_KeyLayer_Mnger.keylayer.initfg = KEYLAYER_INIT_FLAG;
	KEY_Save_KeyLayer(layer);
	sg_KeyLayer_Mnger.working = layer;
	sg_KeyLayer_Mnger.enable[layer] = 1;
	KEY_Save_KeyLayerManager();
	return true;
}

uint8_t KEY_SaveCurrentKeyLayer()
{
	return KEY_Save2KeyLayer(sg_KeyLayer_Mnger.working);
}

uint8_t KEY_GetCurrentKeyLayer()
{
	return sg_KeyLayer_Mnger.working;
}




uint8_t* KEY_Get_KeyLayer_Name(uint8_t layer)
{
	static uint8_t name[KEYLAYER_NAME_LEN];
	if(layer >= MAX_KEYLAYER_NUM)
		return NULL;
	if(0 == sg_KeyLayer_Mnger.enable[layer])
		return NULL;
	
	IntFlash_Read(KEYLAYERS_ADDR+layer*sizeof(KeyLayer),name,KEYLAYER_NAME_LEN);
	name[KEYLAYER_NAME_LEN-1] = 0;
	return name;
}

 void KEY_Set_CurrentKeyLayer_Name(uint8_t* name)
{
	uint8_t i;
	//if(sg_Working_KeyLayer->lock)
		//return;

	for(i=0;i<KEYLAYER_NAME_LEN-5;i++)
	{
		if(name[i] == 0)
			break;
		KeyCodeDis_t* code = KeyLayer_GetCodecDiscribe(name[i]);
		sg_KeyLayer_Mnger.keylayer.name[i] = code->upline[0];
		if(0 == code->upline[0])
		{	
			break;
		}
		
	}
	
	sg_KeyLayer_Mnger.keylayer.name[i] = 0;
}

uint8_t KEY_Change2KeyLayer(uint8_t layer)
{
	uint16_t initfg;
	if(layer >= MAX_KEYLAYER_NUM)
		return false;
	if(0 == sg_KeyLayer_Mnger.enable[layer])
		return false;
	IntFlash_Read(KEYLAYERS_ADDR+(layer)*sizeof(KeyLayer)+KEYLAYER_NAME_LEN,(uint8_t*)&initfg,2);

	if(initfg == KEYLAYER_INIT_FLAG)
	{
		KEY_Load_KeyLayer(layer);
		sg_KeyLayer_Mnger.working = layer;
		sg_KeyLayer_Mnger.enable[layer] = 1;
		KEY_Save_KeyLayerManager();
		return true;
	}

	return false;
}


uint8_t KEY_Set_KeyLayer_Combine(keybit_t bit,keycombine_t combine)
{
	if(sg_Working_KeyLayer->lock)
		return false;

	if(bit >= sg_Working_KeyLayer->num)
		return false;

	memcpy(sg_Working_KeyLayer->combine[bit],combine,sizeof(keycombine_t));

	return true;
}

void KEY_KeyLayer_Init()
{
	KEY_Load_KeyLayerManager();
	if((sg_KeyLayer_Mnger.working >= MAX_KEYLAYER_NUM) || (!sg_KeyLayer_Mnger.enable[sg_KeyLayer_Mnger.working]))
	{
	LOAD_DEFLAYER:
		memset(&sg_KeyLayer_Mnger,0,sizeof(sg_KeyLayer_Mnger));
		sg_KeyLayer_Mnger.working = MAX_KEYLAYER_NUM;
		KEY_Save_KeyLayerManager();
		memcpy(&sg_KeyLayer_Mnger.keylayer,&sg_KeyLayerDef,sizeof(KeyLayer));
		sg_Working_KeyLayer = &sg_KeyLayer_Mnger.keylayer;		
	}
	else
	{
		KEY_Load_KeyLayer(sg_KeyLayer_Mnger.working);
		if(sg_KeyLayer_Mnger.keylayer.initfg != KEYLAYER_INIT_FLAG)
		{
			goto LOAD_DEFLAYER;
		}
	}
}

bool KeyLayer_IsLocked()
{
	return sg_Working_KeyLayer->lock;
}

void KeyLayer_SetLock(uint8_t lock)
{
	sg_Working_KeyLayer->lock = lock;
}


keybit_t KeyLayer_GetKeyNum()
{
	return sg_KeyLayer_Mnger.keylayer.num;
}

keycode_t KeyLayer_GetBitCodec(keybit_t bit)
{
	return sg_KeyLayer_Mnger.keylayer.combine[bit][0];
}

KeyLayer sg___keylay;

void KeyLayer_Delete(uint8_t layer)
{
	
	if(layer >= MAX_KEYLAYER_NUM)
		return;
	
	memset(&sg___keylay,0,sizeof(KeyLayer));
	IntFlash_Write(KEYLAYERS_ADDR+(layer)*sizeof(KeyLayer),(uint8_t *)&sg___keylay,sizeof(KeyLayer));

	sg_KeyLayer_Mnger.enable[layer] = 0;
	KEY_Save_KeyLayerManager();

}

static fonts_e KeyCode_2_Ascii(keycode_t code,uint8_t *string)
{
	if((code >= KEY_CODE_A)&&(code <=KEY_CODE_Z))
	{
		 string[0] = (code - KEY_CODE_A + 'A');
		 string[1] = 0;
		 return FONT_ASCII_11x16;
	}
	else if((code >= KEY_CODE_1)&&(code <=KEY_CODE_0))
	{
		string[0] = (code - KEY_CODE_1 + '1');
		string[1] = 0;
		return FONT_ASCII_11x16;
	}
	else if((code >= KEY_CODE_F1)&&(code <=KEY_CODE_F12))
	{
		string[0] = 'F';
		string[1] = (code - KEY_CODE_F1 + '1');
		string[2] = 0;
		return FONT_ASCII_11x16;
	}
	else if((code >= KEY_CODE_KP1)&&(code <=KEY_CODE_KP0))
	{
		string[0] = 'N';
		string[1] = (code - KEY_CODE_KP1 + '1');
		string[2] = 0;
		return FONT_ASCII_11x16;
	}
	else if((code >= KEY_CODE_RIGHT)&&(code <=KEY_CODE_UP))
	{
		string[0] = code;
		string[1] = 0;
		return FONT_EXT_16x16;
	}

	
	switch(code)
	{
		case KEY_CODE_ENTER:
			string[0] = KEY_CODE_UP+1;
			string[1] = KEY_CODE_UP+2;
			string[2] = 0;
			return FONT_EXT_16x16;
		case KEY_CODE_ESC:
			string[0] = 'E';
			string[1] = 'S';
			string[2] = 'C';
			string[3] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_BACKSPACE:
			string[0] = 'B';
			string[1] = 'K';
			string[2] = 'S';
			string[3] = 'P';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_TAB:
			string[0] = 'T';
			string[1] = 'A';
			string[2] = 'B';
			string[3] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_SPACE:
			string[0] = '<';
			string[1] = '-';
			string[2] = '-';
			string[3] = '>';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_LEFTCTRL:
		case KEY_CODE_RIGHTCTRL:
			string[0] = 'C';
			string[1] = 'T';
			string[2] = 'R';
			string[3] = 'L';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_LEFTSHIFT:
		case KEY_CODE_RIGHTSHIFT:
			string[0] = 'S';
			string[1] = 'H';
			string[2] = 'F';
			string[3] = 'T';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_LEFTALT:
		case KEY_CODE_RIGHTALT:
			string[0] = 'A';
			string[1] = 'L';
			string[2] = 'T';
			string[3] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_LEFTMETA:
		case KEY_CODE_RIGHTMETA:
			string[0] = 'G';
			string[1] = 'U';
			string[2] = 'I';
			string[3] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_KPLEFTPAREN:
			string[0] = '(';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_KPRIGHTPAREN:
			string[0] = ')';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_KPEQUAL:
			string[0] = 'N';
			string[1] = '=';
			string[2] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_KPDOT:
			string[0] = 'N';
			string[1] = '.';
			string[2] = 0;
			return FONT_ASCII_11x16; 
		case KEY_CODE_KPENTER:
			string[0] = 'N';
			string[1] = 'E';
			string[2] = 'N';
			string[3] = 'T';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_NUMLOCK:
			string[0] = 'L';
			string[1] = 'O';
			string[2] = 'C';
			string[3] = 'K';
			string[4] = 0;
			return FONT_ASCII_11x16;
		
		case KEY_CODE_KPSLASH:
			string[0] = 'N';
			string[1] = '/';
			string[2] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_KPASTERISK:
			string[0] = 'N';
			string[1] = '*';
			string[2] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_KPMINUS:
			string[0] = 'N';
			string[1] = '-';
			string[2] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_KPPLUS:
			string[0] = 'N';
			string[1] = '+';
			string[2] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_SYSRQ:
			string[0] = 'P';
			string[1] = 'r';
			string[2] = 'S';
			string[3] = 'c';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_SCROLLLOCK:
			string[0] = 'S';
			string[1] = 'L';
			string[2] = 'c';
			string[3] = 'k';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_PAUSE:
			string[0] = 'P';
			string[1] = 'A';
			string[2] = 'U';
			string[3] = 'S';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_INSERT:
			string[0] = 'S';
			string[1] = 'E';
			string[2] = 'R';
			string[3] = 'T';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_HOME:
			string[0] = 'H';
			string[1] = 'O';
			string[2] = 'M';
			string[3] = 'E';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_END:
			string[0] = 'E';
			string[1] = 'N';
			string[2] = 'D';
			string[3] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_PAGEUP:
			string[0] = 'P';
			string[1] = 'G';
			string[2] = 'U';
			string[3] = 'P';
			string[4] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_PAGEDOWN:
			string[0] = 'P';
			string[1] = 'G';
			string[2] = 'D';
			string[3] = 'N';
			string[4] = 0;	
			return FONT_ASCII_11x16;
		case KEY_CODE_DELETE:
			string[0] = 'D';
			string[1] = 'E';
			string[2] = 'L';
			string[3] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_MINUS:
			string[0] = '-';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_EQUAL:
			string[0] = '=';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_LEFTBRACE:
			string[0] = '[';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_RIGHTBRACE:
			string[0] = ']';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_BACKSLASH:
			string[0] = '\\';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_GRAVE:
			string[0] = '`';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_SEMICOLON:
			string[0] = ';';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_APOSTROPHE:
			string[0] = '\'';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_HASHTILDE:
			string[0] = '#';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_COMMA:
			string[0] = ',';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_DOT:
			string[0] = '.';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_SLASH:
			string[0] = '/';
			string[1] = 0;
			return FONT_ASCII_11x16;
		case KEY_CODE_CAPSLOCK:
			string[0] = 'C';
			string[1] = 'A';
			string[2] = 'P';
			string[3] = 'S';
			string[4] = 0;
			return FONT_ASCII_11x16;
		
	}
	
	string[0] = 0;
	return FONT_UNKNOW;
}

KeyCodeDis_t* KeyLayer_GetCodecDiscribe(keybit_t bit)
{
	static KeyCodeDis_t discribe;

	discribe.type[0] = KeyCode_2_Ascii(sg_KeyLayer_Mnger.keylayer.combine[bit][0],discribe.upline);
	discribe.type[1] = KeyCode_2_Ascii(sg_KeyLayer_Mnger.keylayer.combine[bit][1],discribe.downline);
	return &discribe;
}







