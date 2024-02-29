
#include "rgb.h"
#include "rgbmode.h"


void RGB_Mode_Init()
{
	RGB_KeyLayer_Enable();
	RGB_BlockLayer_Disenable();
	RGB_BackgrounLayer_Disenable();
}


void RGB_KeyAction(keybit_t bit)
{
	uint8_t x,y;
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


	RGB_SetColor_Pixel(csg_rgbmap_def[y][x],&rgb);

}



















