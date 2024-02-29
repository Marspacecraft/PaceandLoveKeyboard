
#ifndef __KEY_CONFIG_
#define __KEY_CONFIG_

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************************

v1.02
	��������Oled��ʾ����̫������ʾ�������
	�޸�rgb��Ч��ͳһ��ȡ��rgbmode�ļ�

v1.03
	�޸�ң��У׼��д���ô���
	�������������Ӱ汾��ʾ
	���������󣬰����ᵼ��oled����
	����F1������,�л�rgb

v1.04
	ע��keybit������keycodehid�Ļ��棬����쳣���ͼ�ֵ����



**********************************************************************/

#define PALK_VERSION "palk_v1.03"


/**********************************************************************

	RGB����
	
**********************************************************************/


#define MAX_RGB_PIXEL_NUM 		85
#define RGB_ARRANGE_MODE		2//1������б���룻2;������б����



/**********************************************************************

	��������
	
**********************************************************************/


#define MAX_KEYBIT_NUM 88

/**********************************************************************

	��ʾ����
	
**********************************************************************/


//#define SPI_SHOW_LCD

#define SPI_EPAPER

#define I2C_OLED

/**********************************************************************

	USB����
	
**********************************************************************/


#define USBD_KEYBOARD_REPORT_CODEC_START	0x04//a
#define USBD_KEYBOARD_REPORT_CODEC_MAX 		0x6B
#define USBD_KEYBOARD_REPORT_CODEC_EXT_MAX	0xe7//231

#define USBD_MSC_2_INTFLASH  1
#if  USBD_MSC_2_INTFLASH
#define USBD_MSC_2_NORFLASH 0
#else
#define USBD_MSC_2_NORFLASH 1
#endif

/**********************************************************************

	��־����
	
**********************************************************************/
#define USE_TRACELOG

#define TL_LEVEL_DEBUG 		0
#define TL_LEVEL_INFO  		1
#define TL_LEVEL_WARNING 	2
#define TL_LEVEL_ERRO 		3
#define TL_LEVEL_MAX		4

#define TL_LEVEL			TL_LEVEL_INFO


//#define USE_TRACELOG_EXTUART

#define USE_TRACELOG_VCP




/**********************************************************************

	������������
	
**********************************************************************/

#define USE_TESTCASE
#ifdef USE_TESTCASE

//#define TEST_NORFLASH

//#define TEST_INTFLASH

//#define TEST_JOYSTICK

//#define TEST_SCHEDULER

//#define TEST_LOWPOWER

//#define TEST_USB

//#define TEST_RGB

//#define TEST_74HC165

//#define TEST_KEYTASKSCHEDULER

//#define TEST_FATFS

//#define TEST_USB_MSC

//#define TEST_EPAPER

//#define TEST_TIMERTASK

//#define DETECT_DELAY	

//#define TEST_ADC

//#define TEST_FNKEY

//#define TEST_LCD
	
//#define TEST_FONT

//#define TEST_REGMODE

//#define TEST_OLED

//#define TEST_GT20


#endif








#ifdef __cplusplus
}
#endif

#endif
















