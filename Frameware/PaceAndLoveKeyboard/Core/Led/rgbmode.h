#ifndef _RGB_MODE_H_

#define _RGB_MODE_H_
#include "main.h"
#include "rgb.h"
#include "ws2812b.h"
#include "keylayer.h"


/************************************************************************

	RGB显示
	
	显示分层
		显示分3层:顶层-按键层；中层-块层；底层-背景层。上层覆盖下层的颜色。
	按键层：
		RGB_KeyAction函数：当按键被按下时，RGB_KeyAction被调用，入参bit为对应按键。
	块层：
		RGB_BlockAction函数：周期性调用(大约100ms一次)。入参block为调用Get_FreeBlock生成一个新块设定的参数；入参t为时间计数器。
	背景层：
		RGB_BackgroundAction函数：周期性调用(大约100ms一次)。入参t为时间计数器。

	一般应用方式
		按键层设置按键按下的action；块层实现流水灯，跳动灯等；背景层设置rgb的默认显示方式

	颜色函数：
		RGB_Send：更新rgb颜色
		RGB_SetColor_Pixel：pixel：像素位置，大部分对应keybit_t。通过csg_rgbmap_def查询；rgb：像素颜色。设置缓存值，不会立即显示，需要调用RGB_Send统一显示。
		RGB_SetColor_All：设置所有led颜色为rgb。设置缓存值，不会立即显示，需要调用RGB_Send统一显示。
		RGB_CloseAll;关闭所有led。
		hsv2rgb：hsv转rgb。

	像素排列对齐模式-RGB_ARRANGE_MODE
		1：左对齐。F1-1-Q-A-Z为同列；F9-9-O-L-.-Fn为同列
		2：两侧对齐。F1-1-Q-A-Z为同列；F6-6-Y-H-B-空格为同列；F9-9-I-J-N-空格为同列
		
	csg_bitmap_def：
		按键keybit对应的像素映射到显示(x,y)坐标

	csg_rgbmap_def:
		显示(x,y)坐标，映射到像素位置，使用RGB_SetColor_Pixel设置颜色

	RGB_Mode_Init函数：
		调用RGB_KeyLayer_Enable；RGB_BlockLayer_Enable；RGB_BackgrounLayer_Enable实现各层的使能及初始化

	Get_FreeBlock：
		注册一个新的block对象，入参func为执行函数；返回rgbblcokinfo*地址，当对象被调用时rgbblcokinfo*为func的入参。最多支持16个block。
		
		

************************************************************************/

#define RGB_BIT_COLUMN_MAX (6)
#if (RGB_ARRANGE_MODE == 1)
#define RGB_BIT_ROW_MAX 	(19)//18u 1 = 0.25u
#else
#define RGB_BIT_ROW_MAX 	(21)//18u 1 = 0.25u
#endif
typedef struct
{
	uint8_t xs;
	uint8_t ys;
	uint8_t xe;
	uint8_t ye;
	uint16_t count;
}rgbblcokinfo;//5

typedef void (*rgbblockfunc)(rgbblcokinfo* block,uint16_t t);
typedef void (*rgbkeyfunc)(keybit_t);
typedef void (*rgbbackfunc)(uint16_t t);

//显示行列(x,y)位置映射到像素
extern const keybit_t csg_rgbmap_def[RGB_BIT_COLUMN_MAX][RGB_BIT_ROW_MAX];
//按键映射到显示行列(x,y)位置
extern const uint8_t csg_bitmap_def[MAX_KEYBIT_NUM][2];



rgbblcokinfo* Get_FreeBlock(rgbblockfunc func);

void RGB_KeyLayer_Enable(void);
void RGB_KeyLayer_Disenable(void);
void RGB_BlockLayer_Enable(void);
void RGB_BlockLayer_Disenable(void);
void RGB_BackgrounLayer_Enable(void);
void RGB_BackgrounLayer_Disenable(void);


void RGB_Mode_Init(void);
void RGB_KeyAction(keybit_t bit);
void  RGB_BlockAction(rgbblcokinfo* block,uint16_t t);
void  RGB_BackgroundAction(uint16_t t);





#endif




