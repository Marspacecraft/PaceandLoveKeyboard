
#include "gt20.h"
#include "gpio.h"
#include "spi.h"
#if 1
#define ZK_CS_Clr()     HAL_GPIO_WritePin(LED_PWM_GPIO_Port, LED_PWM_Pin, GPIO_PIN_RESET)//CS2
#define ZK_CS_Set()      HAL_GPIO_WritePin(LED_PWM_GPIO_Port, LED_PWM_Pin, GPIO_PIN_SET)

void Send_Command_to_ROM(uint8_t dat)
{
	HAL_SPI_Transmit(&EXT_SPI_HANDLER, &dat, 1, 1000);
}

uint8_t Get_data_from_ROM(void)
{
	uint8_t read=0;
	HAL_SPI_Receive(&EXT_SPI_HANDLER, &read, 1, 1000);
	return read;
}
#else

#define ZK_SCLK_Clr()   HAL_GPIO_WritePin(EXT_SPI2_CLK_GPIO_Port,EXT_SPI2_CLK_Pin, GPIO_PIN_RESET)//SCL
#define ZK_SCLK_Set()   HAL_GPIO_WritePin(EXT_SPI2_CLK_GPIO_Port,EXT_SPI2_CLK_Pin, GPIO_PIN_SET)

#define ZK_MOSI_Clr()   HAL_GPIO_WritePin(EXT_SPI2_MOSI_GPIO_Port,EXT_SPI2_MOSI_Pin, GPIO_PIN_RESET)//SDA
#define ZK_MOSI_Set()   HAL_GPIO_WritePin(EXT_SPI2_MOSI_GPIO_Port,EXT_SPI2_MOSI_Pin, GPIO_PIN_SET)

#define ZK_MISO()       HAL_GPIO_ReadPin(EXT_SPI2_MISO_GPIO_Port,EXT_SPI2_MISO_Pin)//FS0

#define ZK_CS_Clr()     HAL_GPIO_WritePin(LED_PWM_GPIO_Port, LED_PWM_Pin, GPIO_PIN_RESET)//CS2
#define ZK_CS_Set()   	HAL_GPIO_WritePin(LED_PWM_GPIO_Port, LED_PWM_Pin, GPIO_PIN_SET)



void Send_Command_to_ROM(uint8_t dat)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		ZK_SCLK_Clr();
		if(dat&0x80)
		{
			ZK_MOSI_Set();
    }
		else
		{
			ZK_MOSI_Clr();
    }
		dat<<=1;
		ZK_SCLK_Set();
  }
}

uint8_t Get_data_from_ROM(void)
{
	uint8_t i,read=0;
	for(i=0;i<8;i++)
	{
		ZK_SCLK_Clr();
		read<<=1;
		if(ZK_MISO())
		{
			read++;
    }
		ZK_SCLK_Set();
  }
	return read;
}



#endif

void OLED_get_data_from_ROM(uint8_t addrHigh,uint8_t addrMid,uint8_t addrLow,uint8_t *pbuff,uint8_t DataLen)
{
	uint8_t i;
	ZK_CS_Clr();
	Send_Command_to_ROM(0x03);
	Send_Command_to_ROM(addrHigh);
	Send_Command_to_ROM(addrMid);
	Send_Command_to_ROM(addrLow);
	for(i=0;i<DataLen;i++)
	{
		*(pbuff+i)=Get_data_from_ROM();
  	}
	ZK_CS_Set();
}



uint8_t Fonts_GB2312_Chinese(uint8_t *text,uint8_t*fontbuf)
{
	uint8_t addrHigh,addrMid,addrLow;
	uint32_t fontaddr=0;


	if((text[0]>=0xb0)&&(text[0]<=0xf7)&&(text[1]>=0xa1))
	{
		fontaddr=(text[0]-0xb0)*94;
		fontaddr+=(text[1]-0xa1)+846;
		fontaddr=fontaddr*32;
		
		addrHigh=(fontaddr&0xff0000)>>16;   
		addrMid=(fontaddr&0xff00)>>8;       
		addrLow=(fontaddr&0xff);          
		
		OLED_get_data_from_ROM(addrHigh,addrMid,addrLow,fontbuf,32);
		return 2;

	}
	else if((text[0]>=0xa1)&&(text[0]<=0xa3)&&(text[1]>=0xa1))
	{
		
		fontaddr=(text[0]-0xa1)*94;
		fontaddr+=(text[1]-0xa1);
		fontaddr=fontaddr*32;
		
		addrHigh=(fontaddr&0xff0000)>>16;
		addrMid=(fontaddr&0xff00)>>8;
		addrLow=(fontaddr&0xff);
		
		OLED_get_data_from_ROM(addrHigh,addrMid,addrLow,fontbuf,32);
		return 2;

	}
	else if((text[0]>=0x20)&&(text[0]<=0x7e))
	{
		fontaddr=(text[0]-0x20);
		fontaddr=(unsigned long)(fontaddr*16);
		fontaddr=(unsigned long)(fontaddr+0x3cf80);
		
		addrHigh=(fontaddr&0xff0000)>>16;
		addrMid=(fontaddr&0xff00)>>8;
		addrLow=fontaddr&0xff;
		
		OLED_get_data_from_ROM(addrHigh,addrMid,addrLow,fontbuf,16);
		return 1;

	}
	
	return 0;
}

#ifdef TEST_GT20
#include "tracelog.h"
void testcase_gt20()
{
	TRACELOG_DEBUG("Running testcase_gt20...");
	__HAL_SPI_ENABLE(&EXT_SPI_HANDLER);   
	uint8_t buff[32] = {0};
	uint8_t chinese[2] = {0xbb,0xbe};
	Fonts_GB2312_Chinese(chinese,buff);

	TRACELOG_DEBUG("Testcase_gt20 end...");
}

#endif




