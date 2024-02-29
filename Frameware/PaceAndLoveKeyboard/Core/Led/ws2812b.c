#include "ws2812b.h"
#include "spi.h"
#include "tracelog.h"
#include "string.h"
#include "tim.h"

#define MAX_COLOR_CHAR_NUM 3
#define COLOR_CHAR_RED 		1
#define COLOR_CHAR_GREEN 	0
#define COLOR_CHAR_BLUE 	2

#define DELAY_NUM 3
static uint16_t sg_LEDBuffer [MAX_LED_NUM+DELAY_NUM][MAX_COLOR_CHAR_NUM][8] = {0};
static uint16_t sg_PixelBuffer[MAX_RGB_PIXEL_NUM+DELAY_NUM][MAX_COLOR_CHAR_NUM][8] = {0};

t_RGBColor RGB_RED   		= {255,0,0};
t_RGBColor RGB_GREEN    	= {0,255,0}; 
t_RGBColor RGB_BLUE     	= {0,0,255};
t_RGBColor RGB_YELLOW   	= {255,255,0};
t_RGBColor RGB_MAGENTA  	= {255,0,255};
t_RGBColor RGB_BLACK    	= {0,0,0};
t_RGBColor RGB_WHITE    	= {255,255,255};

#define HIGH 65
#define LOW 26
//#define WS_HIGH 	0xFE
//#define WS_LOW		0xE0




#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max3(a,b,c) (((a) > (b) ? (a) : (b)) > (c) ? ((a) > (b) ? (a) : (b)) : (c))
#define min3(a,b,c) (((a) < (b) ? (a) : (b)) < (c) ? ((a) < (b) ? (a) : (b)) : (c))



void hsv2rgb(color_hsv_t *hsv, color_rgb_t *rgb)
{

    float h = hsv->h;
    float s = hsv->s;
    float v = hsv->v/8;//亮度太高，电流过大，影响稳定性
	float r, g, b;
	
	int i = (int)(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);
	
	switch(i % 6)
	{
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
	
	rgb->r = r * 255;
	rgb->g  = g * 255;
	rgb->b  = b * 255;

}

/**
 * @brief   调节HSV颜色空间 H值(0.0~360.0)
 * @param   hsv:HSV颜色空间参数
 * @param   h_offset:调节参数 增加>0.0，减小<0.0
 * @return  none
*/
void hsv_adjust_h(color_hsv_t *hsv, float h_offset)
{
    hsv->h += h_offset;
    if (hsv->h >= 360) {
        hsv->h = 360;
    } else if (hsv->h<=0) {
        hsv->h = 0;
    }
}


/**
 * @brief   调节HSV颜色空间 S值(0.0~1.0)
 * @param   hsv:HSV颜色空间参数
 * @param   s_offset:调节参数 增加>0.0，减小<0.0
 * @return  none
*/
void hsv_adjust_s(color_hsv_t *hsv, float s_offset)
{
    hsv->s += s_offset;
    if (hsv->s >= 1.0) {
        hsv->s = 1.0;
    } else if (hsv->s <= 0) {
        hsv->s = 0;
    }
}


/**
 * @brief   调节HSV颜色空间 V值(0.0~1.0)
 * @param   hsv:HSV颜色空间参数
 * @param   v_offset:调节参数 增加>0.0，减小<0.0
 * @return  none
*/
void hsv_adjust_v(color_hsv_t *hsv, float v_offset)
{
    hsv->v += v_offset;
    if (hsv->v >= 1.0) {
        hsv->v = 1.0;
    } else if (hsv->v<=0) {
        hsv->v = 0;
    }
}




void RGB_Send()
{
	HAL_TIM_PWM_Start_DMA(&LEG_HTIM,TIM_CHANNEL_1,(uint32_t*)&sg_PixelBuffer,sizeof(sg_PixelBuffer));
}


void RGB_SetColor_Pixel(uint8_t pixel,t_RGBColor* rgb)
{
	uint8_t i;

	if(pixel >= MAX_RGB_PIXEL_NUM)
		return;
	
    for (i = 0; i < 8; i++)
    {

		sg_PixelBuffer[pixel+DELAY_NUM][COLOR_CHAR_RED][i] = (((rgb->r << i) & 0X80) ? HIGH : LOW);
		sg_PixelBuffer[pixel+DELAY_NUM][COLOR_CHAR_GREEN][i] = (((rgb->g << i) & 0X80) ? HIGH : LOW);
		sg_PixelBuffer[pixel+DELAY_NUM][COLOR_CHAR_BLUE][i] = (((rgb->b << i) & 0X80) ? HIGH : LOW);
    }
}

void RGB_SetColor_All(t_RGBColor* rgb)
{
	uint8_t i = 0;
	for(i=0;i<MAX_RGB_PIXEL_NUM;i++)
		RGB_SetColor_Pixel(i,rgb);
}


void RGB_CloseAll()
{
	RGB_SetColor_All(&RGB_BLACK);
	RGB_Send();
}


/**************************LED LINE**************************************/




void LED_SetColor_BIT(uint8_t bit,t_RGBColor* rgb)
{
	uint8_t i;
	if(bit >= MAX_LED_NUM)
		return;
	
	for(i=0;i<8;i++)
	{
		sg_LEDBuffer[bit+DELAY_NUM][COLOR_CHAR_RED][i] = (((rgb->r << i) & 0X80) ? HIGH : LOW);
		sg_LEDBuffer[bit+DELAY_NUM][COLOR_CHAR_GREEN][i] = (((rgb->g << i) & 0X80) ? HIGH : LOW);
		sg_LEDBuffer[bit+DELAY_NUM][COLOR_CHAR_BLUE][i] = (((rgb->b << i) & 0X80) ? HIGH : LOW);
	}
}



void LED_SetColor_All(t_RGBColor* rgb)
{
	uint8_t i = 0;
	for(i=0;i<MAX_LED_NUM;i++)
		LED_SetColor_BIT(i,rgb);
}


void LED_Send(void)
{
	HAL_TIM_PWM_Start_DMA(&LEG_HTIM,TIM_CHANNEL_2,(uint32_t*)&sg_LEDBuffer,sizeof(sg_LEDBuffer));
}

void LED_CloseAll(void)
{
	LED_SetColor_All(&RGB_BLACK);
	LED_Send();
}


#ifdef TEST_RGB



void test_rgb()
{
	TRACELOG_DEBUG("\ttest_rgb...");
	TRACELOG_DEBUG("\t\tShow red color.");
	t_RGBColor color;
	color.r = 255;
	color.g = 0;
	color.b = 0;
	RGB_SetColor_All(&color);
	RGB_Send();
	HAL_Delay(1000);

	TRACELOG_DEBUG("\t\tShow green color.");
	color.r = 0;
	color.g = 255;
	color.b = 0;
	RGB_SetColor_All(&color);
	RGB_Send();
	HAL_Delay(1000);

	TRACELOG_DEBUG("\t\tShow blue color.");
	color.r = 0;
	color.g = 0;
	color.b = 255;
	RGB_SetColor_All(&color);
	RGB_Send();
	HAL_Delay(1000);

	TRACELOG_DEBUG("\t\tShow close rgb.");
	RGB_CloseAll();
	TRACELOG_DEBUG("\ttest_rgb end...");

}

void test_led()
{
	TRACELOG_DEBUG("\ttest_led...");
	TRACELOG_DEBUG("\t\tShow red color.");
	t_RGBColor color;
	color.r = 255;
	color.g = 0;
	color.b = 0;
	LED_SetColor_All(&color);
	LED_Send();
	HAL_Delay(1000);

	TRACELOG_DEBUG("\t\tShow green color.");
	color.r = 0;
	color.g = 255;
	color.b = 0;
	LED_SetColor_All(&color);
	LED_Send();
	HAL_Delay(1000);

	TRACELOG_DEBUG("\t\tShow blue color.");
	color.r = 0;
	color.g = 0;
	color.b = 255;
	LED_SetColor_All(&color);
	LED_Send();
	HAL_Delay(1000);

	TRACELOG_DEBUG("\t\tShow close led.");
	LED_CloseAll();
	
	TRACELOG_DEBUG("\ttest_led end...");

}


void testcase_rgb()
{
	TRACELOG_DEBUG("Running testcase_rgb...");
	
	test_rgb();
	test_led();
	
	TRACELOG_DEBUG("testcase_rgb end...");
}



#endif



