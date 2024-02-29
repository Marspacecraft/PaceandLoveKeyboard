
#ifndef __OLED_H__
#define __OLED_H__

#include "main.h"

#include "keyconfig.h"
#ifdef I2C_OLED

// 阴码 低位在前 列行式
#define OLED_ADDR 0x78

#define OLED_WIDTH 128
#define OLED_HEIGHT 32

#define OLED_BUFFER_WIDTH	(OLED_WIDTH*4)// 36 个字符 32个汉字
#define OLED_BUFFER_HEIGHT 8//8列

#define OLED_BUFFER_MAX_ASCII 	((OLED_BUFFER_WIDTH-8)/8)
#define OLED_BUFFER_MAX_CHINESE ((OLED_BUFFER_WIDTH-8)/16)



#define OLED_TITLE_0 ((uint8_t)0)
#define OLED_TITLE_1 ((uint8_t)1)
#define OLED_TITLE_2 ((uint8_t)2)
#define OLED_TITLE_3 ((uint8_t)3)
#define OLED_TITLE_4 ((uint8_t)4)
#define OLED_TITLE_5 ((uint8_t)5)
#define OLED_TITLE_6 ((uint8_t)6)
#define OLED_TITLE_7 ((uint8_t)7)
#define OLED_TITLE_NULL ((uint8_t)8)




void OLED_ShowPic(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[]);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t accuracy);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t length);
void OLED_ShowStr(uint8_t x, uint8_t y, char *ch);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t ch);
void OLED_Allfill(void);
void OLED_Set_Position(uint8_t x, uint8_t y);
void OLED_Clear(void);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);

void OLED_CharMode_Init(void);
void OLED_CharMode_DeInit(void);
void OLED_CharMode_ShowString(uint8_t title,uint8_t* string);
void OLED_CharMode_ShowStringAppend(uint8_t title,uint8_t* string);

void OLED_CharMode_SetShowOrder(uint8_t* titles);
void OLED_CharMode_ClearShowOrder(void);





#ifdef TEST_OLED
void testcase_oled(void);

#endif
#endif
#endif  /*__OLED_H__*/

