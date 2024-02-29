#ifndef _FONTS_H_
#define _FONTS_H_
#include "main.h"


typedef enum
{
	FONT_ASCII_5x8 = 0,		
	FONT_ASCII_7x12,
	FONT_ASCII_11x16,
	FONT_ASCII_14x20,
	FONT_ASCII_17x24,

	FONT_KEY_10x40,
	FONT_EXT_16x16,
	FONT_MAX,

	FONT_UNKNOW = FONT_MAX,
	FONT_ASCII = FONT_ASCII_5x8,
	FONT_ASCII_MAX = FONT_ASCII_17x24,

}fonts_e;


typedef struct 
{    
  fonts_e 	Type;
  uint8_t 	Width;
  uint8_t 	FontWidth;
  uint8_t 	Height;
  uint8_t	Font[80];
}font_t;

typedef struct
{
	//fonts_e 		type;
	uint16_t 		startpos;
	unsigned char *	string;
}fonthandler;



void Fonts_Init(void);
font_t * Fonts_GetASCII(fonts_e type,unsigned char c);
//font_t * Fonts_GetUNICODE_chinese(fonts_e type,unsigned char* string);
font_t * Fonts_Get(fonts_e type,fonthandler* handler);

 uint8_t Fonts_Width(fonts_e type);
 uint8_t Fonts_FontWidth(fonts_e type);
 uint8_t Fonts_Height(fonts_e type);


#ifdef TEST_FONT
 void testcase_font(void);
#endif


#endif




