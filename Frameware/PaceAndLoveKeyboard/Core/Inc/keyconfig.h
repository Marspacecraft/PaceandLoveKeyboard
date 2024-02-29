
#ifndef __KEY_CONFIG_
#define __KEY_CONFIG_

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************************

v1.02
	增加启动Oled显示鸡你太美，提示启动完成
	修改rgb灯效，统一提取到rgbmode文件

v1.03
	修改遥杆校准后，写配置错误
	键盘锁定后增加版本显示
	键盘锁定后，按键会导致oled屏闪
	增加F1键功能,切换rgb

v1.04
	注释keybit缓存与keycodehid的缓存，解决异常发送键值问题



**********************************************************************/

#define PALK_VERSION "palk_v1.03"


/**********************************************************************

	RGB配置
	
**********************************************************************/


#define MAX_RGB_PIXEL_NUM 		85
#define RGB_ARRANGE_MODE		2//1：向左斜对齐；2;向两侧斜对齐



/**********************************************************************

	键盘配置
	
**********************************************************************/


#define MAX_KEYBIT_NUM 88

/**********************************************************************

	显示配置
	
**********************************************************************/


//#define SPI_SHOW_LCD

#define SPI_EPAPER

#define I2C_OLED

/**********************************************************************

	USB配置
	
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

	日志配置
	
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

	测试用例配置
	
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
















