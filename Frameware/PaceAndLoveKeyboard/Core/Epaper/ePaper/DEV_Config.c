/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* |	This version:   V2.0
* | Date        :   2018-10-30
* | Info        :
# ******************************************************************************
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
#include "DEV_Config.h"
#include "stm32f1xx_hal_spi.h"
#include "main.h"
#include "spi.h"

extern void do_ms_schedule(void);
extern uint16_t do_ms_schedule_1ms(void);

void DEV_Delay_ms(uint16_t __xms)
{
	__xms /= 5;
	do
	{
		uint16_t delta = do_ms_schedule_1ms();
		if(delta >= __xms)
			return;
		
		__xms -= delta;
	}while(__xms);
}



void DEV_SPI_WriteByte(UBYTE value)
{
    HAL_SPI_Transmit(&EXT_SPI_HANDLER, &value, 1, 1000);
}

void DEV_SPI_WriteBuffer(UBYTE* buffer,UWORD len)
{
	DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
	while(HAL_SPI_GetState(&EXT_SPI_HANDLER) == HAL_SPI_STATE_BUSY_TX)
	{
		do_ms_schedule();
	};
    HAL_SPI_Transmit_DMA(&EXT_SPI_HANDLER, buffer, len);
	DEV_Digital_Write(EPD_CS_PIN, 1);
}


int DEV_Module_Init(void)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
	DEV_Digital_Write(EPD_PWR_PIN, 1);
    DEV_Digital_Write(EPD_RST_PIN, 1);
		return 0;
}

void DEV_Module_Exit(void)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);

    //close 5V
	DEV_Digital_Write(EPD_PWR_PIN, 0);
    DEV_Digital_Write(EPD_RST_PIN, 0);
}

