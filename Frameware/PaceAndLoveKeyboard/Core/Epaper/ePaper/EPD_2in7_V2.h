/*****************************************************************************
* | File      	:   EPD_2in7_V2.h
* | Author      :   Waveshare team
* | Function    :   2.7inch V2 e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2022-08-18
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef __EPD_2IN7_V2_H_
#define __EPD_2IN7_V2_H_


#include "DEV_Config.h"
#include "bmpdecode.h"
#include "keyconfig.h"
#ifdef SPI_EPAPER
// Display resolution
//2:3
#define EPD_2IN7_V2_WIDTH       BMP_WIDTH
#define EPD_2IN7_V2_HEIGHT      BMP_HEIGHT

#define EPD_FRAMEBUF_SIZE_2SCALE		BMP_DATA_BUFFER_SIZE

typedef struct
{//warning: block size not larger then EPD_BLOCKBUF_SIZE
	UWORD 	X_s;
	UWORD	Y_s;
	UWORD	X_e;
	UWORD 	Y_e;

	UWORD 	Rotate;
}tePaperBlockPos;


#pragma pack(push)
#pragma pack (1)

typedef struct
{
	bmp_t head;
	uint8_t body[EPD_FRAMEBUF_SIZE_2SCALE];
}bmpbuffer_t;
#pragma pack (pop)

typedef enum
{
	EPD_TYPE_FRAME = 0,
	EPD_TYPE_BLOCK,
}ePaperType_t;

typedef struct
{
	ePaperType_t  	type;
	tePaperBlockPos pos;
	bmpbuffer_t 	bmp;
}ePaperFrame_t;

extern ePaperFrame_t Hinata;
void EPD_Init(void);
void EPD_StartShow(void);

void EPD_Display(ePaperFrame_t* pframe);
void EPD_Display_Fast(ePaperFrame_t* pframe);
void EPD_Display_Partial(ePaperFrame_t* pblock);
void EPD_Exit(void);

#ifdef TEST_EPAPER

void testcase_ePaper(void);

#endif
#endif
#endif
