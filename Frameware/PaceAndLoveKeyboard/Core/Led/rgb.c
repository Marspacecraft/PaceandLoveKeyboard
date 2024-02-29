

#include "keylayer.h"
#include "keyconfig.h"
#include "keyboard.h"
#include "ws2812b.h"
#include "timertask.h"
#include "tim.h"
#include "rgbmode.h"
#include "string.h"

#define MAX_RGB_BLOCK_NUM 16
#define MAX_RGB_MODE_NUM 8

#define RGB_NONE  			MAX_KEY_NUM//1
#define RGB_BIT_1U_NONE 	RGB_NONE//4
#define RGB_BIT_4U_OFFSET 	RGB_BIT_1U_NONE,RGB_BIT_1U_NONE,RGB_BIT_1U_NONE,RGB_BIT_1U_NONE//16



#if (RGB_ARRANGE_MODE == 1)

#define RGB_BIT_ROW_MAX 	(19)//18u 1 = 0.25u


#define RGB_BIT_1U(BIT) 	(BIT)//1
#define RGB_BIT_125U(BIT) 	(BIT)//1
#define RGB_BIT_15U(BIT) 	(BIT)//1
#define RGB_BIT_15U_(BIT) 	(BIT),(BIT)//2
#define RGB_BIT_175U(BIT) 	(BIT)//2
#define RGB_BIT_2U(BIT) 	(BIT),(BIT)//2
#define RGB_BIT_225U(BIT) 	(BIT),(BIT),(BIT)//3
#define RGB_BIT_625U(BIT) 	(BIT),(BIT),(BIT),(BIT),(BIT),(BIT)//6
extern void For_Each_KeyBit_Press(ForEachBitFunc_f func);
//像素点位映射到rgb显示bit
const keybit_t csg_rgbmap_def[RGB_BIT_COLUMN_MAX][RGB_BIT_ROW_MAX] = 
{
	//line1
	{RGB_BIT_4U_OFFSET,RGB_BIT_1U(0),RGB_BIT_1U(1),RGB_BIT_1U(2),RGB_BIT_1U(3),RGB_BIT_1U(4),RGB_BIT_1U(5),RGB_BIT_1U(6),
		RGB_BIT_1U(7),RGB_BIT_1U(8),RGB_BIT_1U(9),RGB_BIT_1U(10),RGB_BIT_1U(11),RGB_BIT_1U(12),RGB_BIT_1U_NONE,RGB_BIT_1U(13)},
	//line2
	{RGB_BIT_4U_OFFSET,RGB_BIT_1U(27),RGB_BIT_1U(26),RGB_BIT_1U(25),RGB_BIT_1U(24),RGB_BIT_1U(23),RGB_BIT_1U(22),RGB_BIT_1U(21),
		RGB_BIT_1U(20),RGB_BIT_1U(19),RGB_BIT_1U(18),RGB_BIT_1U(17),RGB_BIT_1U(16),RGB_BIT_1U(15),RGB_BIT_2U(14)},
	//line3
	{RGB_BIT_4U_OFFSET,RGB_BIT_15U(28),RGB_BIT_1U(29),RGB_BIT_1U(30),RGB_BIT_1U(31),RGB_BIT_1U(32),RGB_BIT_1U(33),RGB_BIT_1U(34),
		RGB_BIT_1U(35),RGB_BIT_1U(36),RGB_BIT_1U(37),RGB_BIT_1U(38),RGB_BIT_1U(39),RGB_BIT_1U(40),RGB_BIT_15U_(41)},
	//line4
	{RGB_BIT_1U(83),RGB_BIT_1U_NONE,RGB_BIT_1U(82),RGB_BIT_1U_NONE,
					RGB_BIT_1U(54),RGB_BIT_1U(53),RGB_BIT_1U(52),RGB_BIT_1U(51),
		RGB_BIT_1U(50),RGB_BIT_1U(49),RGB_BIT_1U(48),RGB_BIT_1U(47),RGB_BIT_1U(46),RGB_BIT_1U(45),RGB_BIT_1U(44),RGB_BIT_1U(43),RGB_BIT_225U(42)},
	//line5
	{RGB_BIT_1U(79),RGB_BIT_1U(80),RGB_BIT_1U(81),RGB_BIT_1U_NONE,
					RGB_BIT_1U(55),RGB_BIT_1U(56),RGB_BIT_1U(57),RGB_BIT_1U(58),
		RGB_BIT_1U(59),RGB_BIT_1U(60),RGB_BIT_1U(61),RGB_BIT_1U(62),RGB_BIT_1U(63),RGB_BIT_1U(64),RGB_BIT_1U(65),RGB_BIT_1U(66),RGB_BIT_225U(67)},
	//line6
	{RGB_BIT_1U(78),RGB_BIT_1U(77),RGB_BIT_1U(76),RGB_BIT_1U(75),
					RGB_BIT_1U(74),RGB_BIT_1U(73),RGB_BIT_625U(72),
		RGB_BIT_1U(71),RGB_BIT_1U(84),RGB_BIT_1U(70),RGB_BIT_1U(69),RGB_BIT_225U(68)}
};
#endif
#if (RGB_ARRANGE_MODE == 2)


#define RGB_BIT_1U(BIT) 	(BIT)//1
#define RGB_BIT_125U(BIT) 	(BIT)//1
#define RGB_BIT_15U(BIT) 	(BIT)//1
#define RGB_BIT_15U_(BIT) 	(BIT),(BIT)//2
#define RGB_BIT_175U(BIT) 	(BIT)//2
#define RGB_BIT_2U(BIT) 	(BIT),(BIT)//2
#define RGB_BIT_225U(BIT) 	(BIT),(BIT),(BIT)//3
#define RGB_BIT_625U(BIT) 	(BIT),(BIT),(BIT),(BIT),(BIT),(BIT)//6
extern void For_Each_KeyBit_Press(ForEachBitFunc_f func);
//像素点位映射到rgb显示bit
const keybit_t csg_rgbmap_def[RGB_BIT_COLUMN_MAX][RGB_BIT_ROW_MAX] = 
{
	//line1
	{RGB_BIT_4U_OFFSET,RGB_BIT_1U(0),RGB_BIT_1U(1),RGB_BIT_1U(2),RGB_BIT_1U(3),RGB_BIT_1U(4),RGB_BIT_1U(5),RGB_BIT_1U(6),
		RGB_BIT_1U(7),RGB_BIT_1U(8),RGB_BIT_1U(9),RGB_BIT_1U(10),RGB_BIT_1U(11),RGB_BIT_1U(12),RGB_BIT_1U_NONE,RGB_BIT_1U(13),RGB_BIT_1U_NONE,RGB_BIT_1U_NONE},
	//line2
	{RGB_BIT_4U_OFFSET,RGB_BIT_1U(27),RGB_BIT_1U(26),RGB_BIT_1U(25),RGB_BIT_1U(24),RGB_BIT_1U(23),RGB_BIT_1U(22),RGB_BIT_1U(21),
		RGB_BIT_1U(20),RGB_BIT_1U(19),RGB_BIT_1U(18),RGB_BIT_1U(17),RGB_BIT_1U(16),RGB_BIT_1U(15),RGB_BIT_2U(14),RGB_BIT_1U_NONE,RGB_BIT_1U_NONE},
	//line3				0				1				2				3				4				5				6				
	{RGB_BIT_4U_OFFSET,RGB_BIT_1U(28),RGB_BIT_1U(29),RGB_BIT_1U(30),RGB_BIT_1U(31),RGB_BIT_1U(32),RGB_BIT_1U(33),RGB_BIT_1U(33),
	//	7				8				9				10				11				12				13 				14
		RGB_BIT_1U(34),RGB_BIT_1U(35),RGB_BIT_1U(36),RGB_BIT_1U(37),RGB_BIT_1U(38),RGB_BIT_1U(39),RGB_BIT_1U(40),RGB_BIT_1U(41),RGB_BIT_1U_NONE,RGB_BIT_1U_NONE},
	//line4
	{RGB_BIT_1U(83),RGB_BIT_1U_NONE,RGB_BIT_1U(82),RGB_BIT_1U_NONE,
						RGB_BIT_1U(54),RGB_BIT_1U(53),RGB_BIT_1U(52),RGB_BIT_1U(51),RGB_BIT_1U(50),RGB_BIT_1U(49),RGB_BIT_1U(49),
		RGB_BIT_1U(48),RGB_BIT_1U(48),RGB_BIT_1U(47),RGB_BIT_1U(46),RGB_BIT_1U(45),RGB_BIT_1U(44),RGB_BIT_1U(43),RGB_BIT_1U(42),RGB_BIT_1U_NONE,RGB_BIT_1U_NONE},
	//line5
	{RGB_BIT_1U(79),RGB_BIT_1U(80),RGB_BIT_1U(81),RGB_BIT_1U_NONE,
					RGB_BIT_1U(55),RGB_BIT_1U(56),RGB_BIT_1U(57),RGB_BIT_1U(58),RGB_BIT_1U(59),RGB_BIT_1U(60),RGB_BIT_1U(60),
		RGB_BIT_1U(60),RGB_BIT_1U(60),RGB_BIT_1U(61),RGB_BIT_1U(62),RGB_BIT_1U(63),RGB_BIT_1U(64),RGB_BIT_1U(65),RGB_BIT_1U(66),RGB_BIT_1U(67),RGB_BIT_1U_NONE},
	//line6
	{RGB_BIT_1U(78),RGB_BIT_1U(77),RGB_BIT_1U(76),RGB_BIT_1U(75),
						RGB_BIT_1U(74),RGB_BIT_1U(73),RGB_BIT_1U(72),RGB_BIT_1U(72),RGB_BIT_1U(72),RGB_BIT_1U(72),RGB_BIT_1U(72),
		RGB_BIT_1U(72),RGB_BIT_1U(72),RGB_BIT_1U(72),RGB_BIT_1U(72),RGB_BIT_1U(72),		RGB_BIT_1U(71),RGB_BIT_1U(84),RGB_BIT_1U(70),RGB_BIT_1U(69),RGB_BIT_1U(68)}
};

const uint8_t csg_bitmap_def[MAX_KEYBIT_NUM][2] = 
{
	{0,4},//line 0
	{0,5},
	{0,6},
	{0,7},
	{0,8},
	{0,9},
	{0,10},
	{0,11},
	{0,12},
	{0,13},
	{0,14},
	{0,15},
	{0,16},
	{0,18},//14

	{1,17},//line 1
	{1,16},
	{1,15},
	{1,14},
	{1,13},
	{1,12},
	{1,11},
	{1,10},
	{1,9},
	{1,8},
	{1,7},
	{1,6},
	{1,5},
	{1,4},//14 28

	{2,4},//line 2
	{2,5},//q
	{2,6},//w
	{2,7},//e
	{2,8},//r
	{2,9},//t
	{2,11},//y
	{2,12},
	{2,13},
	{2,14},
	{2,15},
	{2,16},
	{2,17},
	{2,18},//14 42


	{3,18},//line 3
	{3,17},
	{3,16},
	{3,15},
	{3,14},
	{3,13},//j
	{3,11},//h
	{3,9},//g
	{3,8},//f
	{3,7},//d
	{3,6},//s
	{3,5},//a
	{3,4},//13 55

	{4,4},//line 4
	{4,5},//z
	{4,6},//x
	{4,7},//c
	{4,8},//v
	{4,9},//b
	{4,13},//n
	{4,14},
	{4,15},
	{4,16},
	{4,17},
	{4,18},
	{4,19},//13 68

	{5,20},//-> //line 5
	{5,19},//
	{5,18},//<-
	{5,16},//alt
	{5,9},//space
	{5,5},//alt
	{5,4},//win//7 75


	{5,3},//ctrl
	{5,2},//
	{5,1},//
	{5,0},//4 79
	
	{4,0},//
	{4,1},//
	{4,2},//3 82

	
	{3,2},//2 84
	{3,0},

	{5,17},//fn 1 85

};

#endif

typedef struct
{
	uint8_t 		enable;
	rgbblcokinfo 	info;//6
	rgbblockfunc 	func;
}rgbblock;//11

typedef struct
{
	#define RGB_LEVEL_BACK 		0
	#define RGB_LEVEL_BLOCK 	1
	#define RGB_LEVEL_KEY 		2
	uint8_t 	enable[3];
	rgbkeyfunc	keyfunc;//10
	rgbblock	block[MAX_RGB_BLOCK_NUM];//36
	rgbbackfunc backgroud;//4
}rgbmode;//56

typedef struct
{
	uint8_t enable;
	rgbmode mode;
}rgbmodemsg_t;//68


rgbmodemsg_t sg_RGB_Msg = {0};


void RGB_DrawBlock(uint16_t t)
{
	uint8_t i;//,x,y;
	for(i=0;i<MAX_RGB_BLOCK_NUM;i++)
	{
		if(sg_RGB_Msg.mode.block[i].enable && sg_RGB_Msg.mode.block[i].func)
		{
			sg_RGB_Msg.mode.block[i].func(&sg_RGB_Msg.mode.block[i].info,t);
#if 0
			for(x=sg_RGB_Msg.mode.block[i].info.xs;x<=sg_RGB_Msg.mode.block[i].info.xe;x++)
			{
				if(x>=RGB_BIT_COLUMN_MAX)
						break;
				for(y=sg_RGB_Msg.mode.block[i].info.ys;y<=sg_RGB_Msg.mode.block[i].info.ye;y++)
				{
					if(y>=RGB_BIT_ROW_MAX)
						break;
							
					sg_RGB_Msg.mode.block[i].func(x,y,&sg_RGB_Msg.mode.block[i].info,t);
					
					if(sg_RGB_Msg.mode.block[i].info.count)
					{
						sg_RGB_Msg.mode.block[i].info.count--;
					}
					else
					{
						sg_RGB_Msg.mode.block[i].enable = 0;
					}
				}
			}
#endif
			
		}
	}
}

void RGB_Draw(uint8_t* arg)
{
	static uint16_t t = 0;
	if(0 == sg_RGB_Msg.enable)
		return;

	if(sg_RGB_Msg.mode.enable[RGB_LEVEL_BACK])
	{
		if(sg_RGB_Msg.mode.backgroud)
		{
			sg_RGB_Msg.mode.backgroud(t);
		}
	}

	if(sg_RGB_Msg.mode.enable[RGB_LEVEL_BLOCK])
	{
		RGB_DrawBlock(t);
	}  
 
 	if(sg_RGB_Msg.mode.enable[RGB_LEVEL_KEY])
 	{
 		if(sg_RGB_Msg.mode.keyfunc)
 		{
 			RGB_Clear();
 			For_Each_KeyBit_Press(sg_RGB_Msg.mode.keyfunc);
 		}
 	}

	RGB_Send();
	
 	t++;
} 

rgbblcokinfo* Get_FreeBlock(rgbblockfunc func)
{
	uint8_t i;
	for(i=0;i<MAX_RGB_BLOCK_NUM;i++)
	{
		if(0 == sg_RGB_Msg.mode.block[i].enable)
		{
			sg_RGB_Msg.mode.block[i].enable = 1;
			sg_RGB_Msg.mode.block[i].func = func;
			return &sg_RGB_Msg.mode.block[i].info;
		}
	}
	return NULL;
}

static CycleTimerHandler* sg_rgbtimerhandler = NULL;
void RGB_Start(rgbmodinitfunc func)
{
	if(NULL == func || sg_rgbtimerhandler)
	{
		return;
	}
	memset(&sg_RGB_Msg,0,sizeof(sg_RGB_Msg));
	HAL_TIM_Base_Start(&LEG_HTIM);
	sg_RGB_Msg.enable = 0;
	func();
	sg_RGB_Msg.enable = 1;
	

	sg_rgbtimerhandler = Start_CycleTimer(TIMER_PERIOD_100MS,RGB_Draw,NULL);
	
	
}

void RGB_Stop()
{
	
	if(sg_rgbtimerhandler)
	{
		sg_RGB_Msg.enable = 0;
		Stop_CycleTimer(sg_rgbtimerhandler);
		RGB_CloseAll();
		HAL_TIM_Base_Stop(&LEG_HTIM);	
		sg_rgbtimerhandler = NULL;	
	}
}




/*********************************************************

	RGB MODE
	
**********************************************************/


void RGB_BreathingLight(uint16_t t)
{
	#define MAX_TIME_BREATHING_V 		90//9s
	#define MAX_TIME_BREATHING_S 		60
	uint16_t s,v;
	t_RGBColor rgb;
	t_HSVColor hsv;

	if((t%(2*MAX_TIME_BREATHING_V))>=MAX_TIME_BREATHING_V)
	{
		v = 2*MAX_TIME_BREATHING_V - (t%(2*MAX_TIME_BREATHING_V));
	}
	else
	{
		v = (t%MAX_TIME_BREATHING_V);
	}
	hsv.v = ((float) v )/((float)MAX_TIME_BREATHING_V);

	if((t%(2*MAX_TIME_BREATHING_S))>=MAX_TIME_BREATHING_S)
	{
		s = 2*MAX_TIME_BREATHING_S - (t%(2*MAX_TIME_BREATHING_S));
	}
	else
	{
		s = (t%MAX_TIME_BREATHING_S);
	}
	hsv.s = ((float) s )/((float)MAX_TIME_BREATHING_S);

	hsv.h = ((float) (t %360))/((float)360);

	hsv2rgb(&hsv,&rgb);		
	RGB_SetColor_All(&rgb);
}
void RGB_Init_BreathingLight()
{
	sg_RGB_Msg.mode.enable[RGB_LEVEL_BACK] = 1;
	sg_RGB_Msg.mode.enable[RGB_LEVEL_BLOCK] = 0;
	sg_RGB_Msg.mode.enable[RGB_LEVEL_KEY] = 0;
	sg_RGB_Msg.mode.backgroud = RGB_BreathingLight;
}



void RGB_blockline(rgbblcokinfo* block,uint16_t t)
{
	uint8_t i,x,y;
	t_RGBColor rgb;
	t_HSVColor hsv;
	if(t%4)//200ms
		return;
	#define RGB_BLOCKLINE_LEN 6
	RGB_Clear();
	hsv.h = 0;
	hsv.s = 1;
	hsv.v = 0.5;
	

	for(i=0,x= block->xs,y=block->ys;i<RGB_BLOCKLINE_LEN;x++,i++)
	{
		if(x>=RGB_BIT_ROW_MAX)
		{
			x = 0;
			y++;
		}
		if(y>=RGB_BIT_COLUMN_MAX)
		{
			y = 0;
		}

		hsv.h += 0.1666;
		if(RGB_NONE != csg_rgbmap_def[y][x])
		{

			hsv2rgb(&hsv,&rgb);	
			RGB_SetColor_Pixel(csg_rgbmap_def[y][x],&rgb);
		}
	}
	block->xs++;
	if(block->xs>=RGB_BIT_ROW_MAX)
	{
		block->xs = 0;
		block->ys++;
	}

	if(block->ys>=RGB_BIT_COLUMN_MAX)
	{
		block->ys = 0;
	}
}

void RGB_Init_RunningLine(void)
{
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BACK] = 0;
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BLOCK] = 1;
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_KEY] = 0;
	sg_RGB_Msg.mode	.block[0].enable = 1;
	sg_RGB_Msg.mode	.block[0].info.xs = 0;
	sg_RGB_Msg.mode	.block[0].info.xe = 3;
	sg_RGB_Msg.mode	.block[0].info.ys = 0;
	sg_RGB_Msg.mode	.block[0].info.ye = 0;
	sg_RGB_Msg.mode	.block[0].info.count = 2;
	sg_RGB_Msg.mode	.block[0].func = RGB_blockline;
}

void RGB_Key_ShowLine(keybit_t bit)
{
	uint8_t x,y,i;
	t_RGBColor rgb;
	t_HSVColor hsv;
	
	if(bit >= MAX_KEYBIT_NUM)
		return;
	hsv.h = 1;
	hsv.s = 1;
	hsv.v = 0.5;
	hsv2rgb(&hsv,&rgb);	
	
	x = csg_bitmap_def[bit][1];
	y = csg_bitmap_def[bit][0];

	for(i=0;i<RGB_BIT_ROW_MAX;i++)
	{
		RGB_SetColor_Pixel(csg_rgbmap_def[y][i],&rgb);
	}

	for(i=0;i<RGB_BIT_COLUMN_MAX;i++)
	{
		RGB_SetColor_Pixel(csg_rgbmap_def[i][x],&rgb);
	}
}

void RGB_Init_KeyCross(void)
{
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BACK] = 0;
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BLOCK] = 0;
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_KEY] = 1;
	sg_RGB_Msg.mode	.keyfunc = RGB_Key_ShowLine;
}



__weak void RGB_KeyAction(keybit_t bit)
{

}

__weak void  RGB_BlockAction(rgbblcokinfo* block,uint16_t t)
{

}

__weak void  RGB_BackgroundAction(uint16_t t)
{

}

void RGB_KeyLayer_Enable()
{
	sg_RGB_Msg.mode	.keyfunc = RGB_KeyAction;
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_KEY] = 1;
}
void RGB_KeyLayer_Disenable()
{
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_KEY] = 0;
}


void RGB_BlockLayer_Enable()
{
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BLOCK] = 1;
}

void RGB_BlockLayer_Disenable()
{
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BLOCK] = 0;
}


void RGB_BackgrounLayer_Enable()
{
	sg_RGB_Msg.mode.backgroud = RGB_BackgroundAction;
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BACK] = 1;
}

void RGB_BackgrounLayer_Disenable()
{
	sg_RGB_Msg.mode	.enable[RGB_LEVEL_BACK] = 0;
}

#define MODE_CHANGE_NUM 4
static rgbmodinitfunc sg_rgbmode[MODE_CHANGE_NUM] = 
{
	NULL,
	RGB_Init_KeyCross,
	RGB_Init_RunningLine,	
	RGB_Init_BreathingLight,
	
};

void RGB_Mode_Change()
{
	static uint8_t i = 0;

	RGB_Stop();
	if(sg_rgbmode[i%MODE_CHANGE_NUM])
	{
		RGB_Stop();
		RGB_Start(sg_rgbmode[i%MODE_CHANGE_NUM]);

	}
	
	i++;
}


__weak void RGB_Mode_Init()
{
	RGB_Init_KeyCross();
}




#ifdef TEST_REGMODE
#include <string.h>

static rgbmode sg_rgbmode_breathe = 
{
	.enable[RGB_LEVEL_BACK] = 1,
	.enable[RGB_LEVEL_BLOCK] = 0,
	.enable[RGB_LEVEL_KEY] = 0,
	.backgroud = RGB_BreathingLight
};

static rgbmode sg_rgbmode_blockline = 
{
	.enable[RGB_LEVEL_BACK] = 0,
	.enable[RGB_LEVEL_BLOCK] = 1,
	.enable[RGB_LEVEL_KEY] = 0,
	.block[0].enable = 1,
	.block[0].info.xs = 0,
	.block[0].info.xe = 3,
	.block[0].info.ys = 0,
	.block[0].info.ye = 0,
	.block[0].info.count = 2,
	.block[0].func = RGB_blockline,

};


void test_rgbmode(rgbmode* mode)
{
	memcpy(&sg_RGB_Msg.mode,mode,sizeof(rgbmode));
	sg_RGB_Msg.enable = 1;


	while(1)
	{
		RGB_Draw();
		HAL_Delay(100);
	}
	
}


void testcase_testrgbmode()
{
	//test_rgbmode(&sg_rgbmode_breathe);
	test_rgbmode(&sg_rgbmode_blockline);
	
}

#endif


