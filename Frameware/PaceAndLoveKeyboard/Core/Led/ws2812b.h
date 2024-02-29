
#ifndef _WS2812B_H_
#define _WS2812B_H_

#include "main.h"


typedef struct
{
	uint8_t r, g, b;
}t_RGBColor;

typedef struct 
{
    float h;    // ɫ��H(hue)  0~360�� {R(0��),G(120��),B(240��)}
    float s;    // ���Ͷ�S(saturation)  0~1.0
    float v;    // ����V(value)  0~1.0  
}t_HSVColor;



typedef struct 
{
 uint32_t c;
}t_U24Color;

typedef enum
{
	TYPE_RGB=0,
	TYPE_HSV,
	TYPE_U24
}e_ColorType;

typedef t_RGBColor color_rgb_t;
typedef  t_HSVColor color_hsv_t ;

extern t_RGBColor RGB_RED;
extern t_RGBColor RGB_GREEN;
extern t_RGBColor RGB_BLUE;
extern t_RGBColor RGB_YELLOW;
extern t_RGBColor RGB_MAGENTA;
extern t_RGBColor RGB_BLACK;
extern t_RGBColor RGB_WHITE; 



#define RGB_HSPI hspi2

void RGB_Send(void);

void RGB_SetColor_Pixel(uint8_t pixel,t_RGBColor* rgb);
void RGB_SetColor_All(t_RGBColor* rgb);
void RGB_CloseAll(void);
#define RGB_Clear()		RGB_SetColor_All(&RGB_BLACK)



#define LEG_HTIM htim8
#define MAX_LED_NUM 		8

void LED_Send(void);

void LED_SetColor_BIT(uint8_t bit,t_RGBColor* rgb);
void LED_SetColor_All(t_RGBColor* rgb);
void LED_CloseAll(void);

#define LED_Clear()		LED_SetColor_All(&RGB_BLACK)

//-------������HSV��ɫ�ռ��RGB��ɫ�ռ��໥ת���ӿ�------------

/**
 * @brief   RGB��ɫ�ռ� ת  HSV��ɫ�ռ� 
 * @param   rgb:RGB��ɫ�ռ����
 * @param   hsv:HSV��ɫ�ռ����
 * @return  none
*/
void rgb2hsv(color_rgb_t *rgb, color_hsv_t *hsv);

/**
 * @brief   HSV��ɫ�ռ� ת RGB��ɫ�ռ�
 * @param   hsv:HSV��ɫ�ռ����
 * @param   rgb:RGB��ɫ�ռ����
 * @return  none
*/
void hsv2rgb(color_hsv_t *hsv, color_rgb_t *rgb);

/**
 * @brief   ����HSV��ɫ�ռ� Hֵ(0.0~360.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   h_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_h(color_hsv_t *hsv, float h_offset);

/**
 * @brief   ����HSV��ɫ�ռ� Sֵ(0.0~1.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   s_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_s(color_hsv_t *hsv, float s_offset);

/**
 * @brief   ����HSV��ɫ�ռ� Vֵ(0.0~1.0)
 * @param   hsv:HSV��ɫ�ռ����
 * @param   v_offset:���ڲ��� ����>0.0����С<0.0
 * @return  none
*/
void hsv_adjust_v(color_hsv_t *hsv, float v_offset);



// ������HSV��ɫ�ռ��RGB��ɫ�ռ��໥ת���ӿ�
/*********************************************************************************************
�죺    
    R   255         H:0
    G   0           S:100
    B   0           V:100    
�̣�
    R   0           H:120   
    G   255         S:100
    B   0           V:100    
����
    R   0           H:240   
    G   0           S:100
    B   255         V:100    
*********************************************************************************************/



#ifdef TEST_RGB
void testcase_rgb(void);
#endif

#endif






