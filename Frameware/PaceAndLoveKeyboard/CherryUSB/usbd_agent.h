
#ifndef _CHERRYUSB_H_
#define _CHERRYUSB_H_
#include "keyconfig.h"
#include "main.h"

typedef enum
{
	USBD_AGENT_MODE_NONE = 0,
	USBD_AGENT_MODE_HID_VCP,
	USBD_AGENT_MODE_MSC,
}usbd_agent_mode_e;

/********************************************
	支持键值 0~231
	支持0x04~0x6B + 6个其他键共110键无冲
	支持bios
********************************************/
#define USBD_KEYBOARD_REPORT_SIZE 			21//22


/*!< keyboard report struct */
typedef struct
{
	uint8_t bioskeypos;
	uint8_t buffer[USBD_KEYBOARD_REPORT_SIZE];
}hid_keyboard;

typedef enum
{
	LED_NUML=0,
	LED_CAPSL,
	LED_SCROLLL,
}hid_kb_led;


/*!< mouse report struct */
typedef struct  
{
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
}hid_mouse;

typedef enum
{
	HID_BOTTON_LEFT = 0,
	HID_BOTTON_RIGHT,
	HID_BOTTON_MID,
}hid_botton_type;

//https://cherryusb.readthedocs.io/zh-cn/latest/index.html


usbd_agent_mode_e usbd_agent_mode(void);
void usbd_start_hid_vcp_mode(void);
void usbd_start_msc_mode(void);


void USBD_VPC_DTR_Enable(bool dtr);
uint8_t USBD_VPC_Send(uint8_t *data, uint32_t data_len);

uint8_t USBD_Mouse_Send(hid_mouse *data);
void  USBD_Mouse_SetButton(hid_mouse* rep,hid_botton_type button,uint8_t pressed);


void USBD_Keyboard_FreeAll(hid_keyboard * buf);
void USBD_Keyboard_SetCodec(hid_keyboard * buf, uint8_t codec);
void USBD_Keyboard_ResetCodec(hid_keyboard * buf, uint8_t codec);
void USBD_Keyboard_SetLED(hid_keyboard * buf,hid_kb_led led);
void USBD_Keyboard_ResetLED(hid_keyboard * buf,hid_kb_led led);

uint8_t USBD_Keyboard_Send(hid_keyboard *data);

void USBD_Print_Keyboard_Report(hid_keyboard *data);

#ifdef TEST_USB

void testcase_usb(void);

#endif

#ifdef TEST_USB_MSC

void testcase_usbmsc(void);

#endif



#endif







