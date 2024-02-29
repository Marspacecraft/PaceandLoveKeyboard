#ifndef _RGB_MODE_H_

#define _RGB_MODE_H_
#include "main.h"
#include "rgb.h"
#include "ws2812b.h"
#include "keylayer.h"


/************************************************************************

	RGB��ʾ
	
	��ʾ�ֲ�
		��ʾ��3��:����-�����㣻�в�-��㣻�ײ�-�����㡣�ϲ㸲���²����ɫ��
	�����㣺
		RGB_KeyAction������������������ʱ��RGB_KeyAction�����ã����bitΪ��Ӧ������
	��㣺
		RGB_BlockAction�����������Ե���(��Լ100msһ��)�����blockΪ����Get_FreeBlock����һ���¿��趨�Ĳ��������tΪʱ���������
	�����㣺
		RGB_BackgroundAction�����������Ե���(��Լ100msһ��)�����tΪʱ���������

	һ��Ӧ�÷�ʽ
		���������ð������µ�action�����ʵ����ˮ�ƣ������Ƶȣ�����������rgb��Ĭ����ʾ��ʽ

	��ɫ������
		RGB_Send������rgb��ɫ
		RGB_SetColor_Pixel��pixel������λ�ã��󲿷ֶ�Ӧkeybit_t��ͨ��csg_rgbmap_def��ѯ��rgb��������ɫ�����û���ֵ������������ʾ����Ҫ����RGB_Sendͳһ��ʾ��
		RGB_SetColor_All����������led��ɫΪrgb�����û���ֵ������������ʾ����Ҫ����RGB_Sendͳһ��ʾ��
		RGB_CloseAll;�ر�����led��
		hsv2rgb��hsvתrgb��

	�������ж���ģʽ-RGB_ARRANGE_MODE
		1������롣F1-1-Q-A-ZΪͬ�У�F9-9-O-L-.-FnΪͬ��
		2��������롣F1-1-Q-A-ZΪͬ�У�F6-6-Y-H-B-�ո�Ϊͬ�У�F9-9-I-J-N-�ո�Ϊͬ��
		
	csg_bitmap_def��
		����keybit��Ӧ������ӳ�䵽��ʾ(x,y)����

	csg_rgbmap_def:
		��ʾ(x,y)���꣬ӳ�䵽����λ�ã�ʹ��RGB_SetColor_Pixel������ɫ

	RGB_Mode_Init������
		����RGB_KeyLayer_Enable��RGB_BlockLayer_Enable��RGB_BackgrounLayer_Enableʵ�ָ����ʹ�ܼ���ʼ��

	Get_FreeBlock��
		ע��һ���µ�block�������funcΪִ�к���������rgbblcokinfo*��ַ�������󱻵���ʱrgbblcokinfo*Ϊfunc����Ρ����֧��16��block��
		
		

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

//��ʾ����(x,y)λ��ӳ�䵽����
extern const keybit_t csg_rgbmap_def[RGB_BIT_COLUMN_MAX][RGB_BIT_ROW_MAX];
//����ӳ�䵽��ʾ����(x,y)λ��
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




