#include "./usbdinc/usbd_core.h"
#include "./usbdinc/usbd_hid.h"
#include "./usbdinc/usbd_cdc.h"
#include "./usbd_agent.h"
#include "keyconfig.h"
#include "tracelog.h"

#define USBD_VID           				0x3717
#define USBD_PID           				0x0E85
#define USBD_MAX_POWER    				500
#define USBD_LANGID_STRING 				1033

#define HID_KEYBOARD_REPORT_DESC_SIZE 63//85 
#define HID_MOUSE_REPORT_DESC_SIZE 		74

#define HID_MOUSE_DESCRIPTOR_LEN 		25
#define HID_KEYBOARD_DESCRIPTOR_LEN 	25
#define USB_CONFIG_VPC_HID_SIZE 		(9 + CDC_ACM_DESCRIPTOR_LEN + HID_MOUSE_DESCRIPTOR_LEN + HID_KEYBOARD_DESCRIPTOR_LEN)//MSC_DESCRIPTOR_LEN + 25)

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif


#define HID_VCP_MODE_LOCK_TRUE {\
if(USBD_AGENT_MODE_HID_VCP != usbd_agent_mode())\
	return true;\
}

#define HID_VCP_MODE_LOCK_FALSE {\
if(USBD_AGENT_MODE_HID_VCP != usbd_agent_mode())\
	return true;\
}

#define HID_VCP_MODE_LOCK_NONE {\
if(USBD_AGENT_MODE_HID_VCP != usbd_agent_mode())\
	return;\
}

#define HID_VCP_MODE_LOCK(RET) HID_VCP_MODE_LOCK_##RET


#define USB_DEVICE_STRING_MFC \
    0x1C,                       /* bLength */ \
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */ \
    'M', 0x00,                  /* wcChar0 */ \
    'a', 0x00,                  /* wcChar1 */ \
    'r', 0x00,                  /* wcChar2 */ \
    's', 0x00,                  /* wcChar3 */ \
    'p', 0x00,                  /* wcChar4 */ \
    'a', 0x00,                  /* wcChar5 */ \
    'c', 0x00,                  /* wcChar6 */ \
    'e', 0x00,                  /* wcChar7 */ \
    'c', 0x00,                  /* wcChar8 */ \
    'r', 0x00,                  /* wcChar9 */ \
    'a', 0x00,                  /* wcChar10 */ \
    'f', 0x00,                  /* wcChar11 */ \
    't', 0x00,                  /* wcChar12 */

#define USB_DEVICE_STRING_PRODUCT \
	0x0E,						/* bLength */ \
   USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */ \
   WBVAL(0x7231),				   /* wcChar0 爱*/ \
   WBVAL(0x4e0e),				   /* wcChar1 与*/ \
   WBVAL(0x548c),				   /* wcChar2 和*/ \
   WBVAL(0x5e73),				   /* wcChar3 平*/ \
   WBVAL(0x952e),				   /* wcChar4 键*/ \
   WBVAL(0x76d8),				   /* wcChar5 盘*/ 

#define USB_DEVICE_STRING_PRODUCT_MOUSE \
	0x0E,						/* bLength */ \
   USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */ \
   WBVAL(0x7231),				   /* wcChar0 爱*/ \
   WBVAL(0x4e0e),				   /* wcChar1 与*/ \
   WBVAL(0x548c),				   /* wcChar2 和*/ \
   WBVAL(0x5e73),				   /* wcChar3 平*/ \
   WBVAL(0x9f20),				   /* wcChar4 鼠*/ \
   WBVAL(0x6807),				   /* wcChar5 标*/ 


#define USB_DEVICE_STRING_PRODUCT_VPC \
	0x0E,						/* bLength */ \
   USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */ \
   WBVAL(0x7231),				   /* wcChar0 爱*/ \
   WBVAL(0x4e0e),				   /* wcChar1 与*/ \
   WBVAL(0x548c),				   /* wcChar2 和*/ \
   WBVAL(0x5e73),				   /* wcChar3 平*/ \
   WBVAL(0x4e32),				   /* wcChar4 串*/ \
   WBVAL(0x53e3),				   /* wcChar5 口*/ 


#define USB_DEVICE_STRING_SERIAL \
0x12,						/* bLength */ \
USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */ \
'2', 0x00,					/* wcChar0 */ \
'0', 0x00,					/* wcChar1 */ \
'0', 0x00,					/* wcChar2 */ \
'6', 0x00,					/* wcChar3 */ \
'3', 0x00,					/* wcChar4 */ \
'7', 0x00,					/* wcChar5 */ \
'1', 0x00,					/* wcChar6 */ \
'7', 0x00,					/* wcChar7 */ 


static struct usbd_interface mouse_intf;
static struct usbd_interface keyboard_intf;
static struct usbd_interface cdc_acm_intf;
static struct usbd_interface cdc_acm_intf1;

#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1
static volatile uint8_t hid_kb_state = HID_STATE_IDLE;
static volatile uint8_t hid_ms_state = HID_STATE_IDLE;
static volatile uint8_t ep_tx_busy_flag = HID_STATE_IDLE;
static volatile uint8_t dtr_enable = true;


void usbd_hid_int_callback(uint8_t ep, uint32_t nbytes);
void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes);
void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes);


/*!< endpoint address */

#define HID_INT_KB_EP          0x85
#define HID_INT_KB_EP_SIZE     8
#define HID_INT_KB_EP_INTERVAL 10
static struct usbd_endpoint hid_kb_in_ep = 
{
    .ep_cb = usbd_hid_int_callback,
    .ep_addr = HID_INT_KB_EP
};

#define HID_INT_MS_EP          0x84
#define HID_INT_MS_EP_SIZE     4
#define HID_INT_MS_EP_INTERVAL 10

static struct usbd_endpoint hid_ms_in_ep = 
{
    .ep_cb = usbd_hid_int_callback,
    .ep_addr = HID_INT_MS_EP
};


#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83
struct usbd_endpoint cdc_out_ep = 
{
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = 
{
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};



#if 0

0x12,        // bLength
0x01,        // bDescriptorType (Device)
0x00, 0x02,  // bcdUSB 2.00
0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
0x00,        // bDeviceSubClass 
0x00,        // bDeviceProtocol 
0x40,        // bMaxPacketSize0 64
0xFF, 0xFF,  // idVendor 0xFFFF
0xFF, 0xFF,  // idProduct 0xFFFF
0x00, 0x20,  // bcdDevice 52.00
0x01,        // iManufacturer (String Index)
0x02,        // iProduct (String Index)
0x03,        // iSerialNumber (String Index)
0x01,        // bNumConfigurations 1

0x09,        // bLength
0x02,        // bDescriptorType (Configuration)
0x7D, 0x00,  // wTotalLength 125
0x04,        // bNumInterfaces 4
0x01,        // bConfigurationValue
0x00,        // iConfiguration (String Index)
0x80,        // bmAttributes
0xFF,        // bMaxPower 510mA

0x08,        // bLength
0x04,        // bDescriptorType (Interface)
0x00,        // bInterfaceNumber 0
0x02,        // bAlternateSetting
0x02,        // bNumEndpoints 2
0x02,        // bInterfaceClass
0x01,        // bInterfaceSubClass
0x00,        // bInterfaceProtocol

0x09,        // bLength
0x04,        // bDescriptorType (Interface)
0x00,        // bInterfaceNumber 0
0x00,        // bAlternateSetting
0x01,        // bNumEndpoints 1
0x02,        // bInterfaceClass
0x02,        // bInterfaceSubClass
0x01,        // bInterfaceProtocol
0x02,        // iInterface (String Index)

0x05,        // bLength
0x24,        // bDescriptorType (Dependant on Type)
0x00, 0x10, 0x01, 
0x05,        // bLength
0x24,        // bDescriptorType (Dependant on Type)
0x01, 0x00, 0x01, 
0x04,        // bLength
0x24,        // bDescriptorType (Dependant on Type)
0x02, 0x02, 
0x05,        // bLength
0x24,        // bDescriptorType (Dependant on Type)
0x06, 0x00, 0x01, 
0x07,        // bLength
0x05,        // bDescriptorType (Endpoint)
0x83,        // bEndpointAddress (IN/D2H)
0x03,        // bmAttributes (Interrupt)
0x08, 0x00,  // wMaxPacketSize 8
0x0A,        // bInterval 10 (unit depends on device speed)

0x09,        // bLength
0x04,        // bDescriptorType (Interface)
0x01,        // bInterfaceNumber 1
0x00,        // bAlternateSetting
0x02,        // bNumEndpoints 2
0x0A,        // bInterfaceClass
0x00,        // bInterfaceSubClass
0x00,        // bInterfaceProtocol
0x00,        // iInterface (String Index)

0x07,        // bLength
0x05,        // bDescriptorType (Endpoint)
0x02,        // bEndpointAddress (OUT/H2D)
0x02,        // bmAttributes (Bulk)
0x40, 0x00,  // wMaxPacketSize 64
0x00,        // bInterval 0 (unit depends on device speed)

0x07,        // bLength
0x05,        // bDescriptorType (Endpoint)
0x81,        // bEndpointAddress (IN/D2H)
0x02,        // bmAttributes (Bulk)
0x40, 0x00,  // wMaxPacketSize 64
0x00,        // bInterval 0 (unit depends on device speed)




0x09,        // bLength
0x04,        // bDescriptorType (Interface)
0x02,        // bInterfaceNumber 2
0x00,        // bAlternateSetting
0x01,        // bNumEndpoints 1
0x03,        // bInterfaceClass
0x01,        // bInterfaceSubClass
0x02,        // bInterfaceProtocol
0x00,        // iInterface (String Index)

0x09,        // bLength
0x21,        // bDescriptorType (HID)
0x11, 0x01,  // bcdHID 1.11
0x00,        // bCountryCode
0x01,        // bNumDescriptors
0x22,        // bDescriptorType[0] (HID)
0x4A, 0x00,  // wDescriptorLength[0] 74

0x07,        // bLength
0x05,        // bDescriptorType (Endpoint)
0x84,        // bEndpointAddress (IN/D2H)
0x03,        // bmAttributes (Interrupt)
0x04, 0x00,  // wMaxPacketSize 4
0x0A,        // bInterval 10 (unit depends on device speed)

0x09,        // bLength
0x04,        // bDescriptorType (Interface)
0x03,        // bInterfaceNumber 3
0x00,        // bAlternateSetting
0x01,        // bNumEndpoints 1
0x03,        // bInterfaceClass
0x01,        // bInterfaceSubClass
0x01,        // bInterfaceProtocol
0x00,        // iInterface (String Index)

0x09,        // bLength
0x21,        // bDescriptorType (HID)
0x11, 0x01,  // bcdHID 1.11
0x00,        // bCountryCode
0x01,        // bNumDescriptors
0x22,        // bDescriptorType[0] (HID)
0x3F, 0x00,  // wDescriptorLength[0] 63

0x07,        // bLength
0x05,        // bDescriptorType (Endpoint)
0x85,        // bEndpointAddress (IN/D2H)
0x03,        // bmAttributes (Interrupt)
0x08, 0x00,  // wMaxPacketSize 8
0x0A,        // bInterval 10 (unit depends on device speed)

0x04,        // bLength
0x03,        // bDescriptorType (String)
0x09, 0x04, 
0x1C,        // bLength
0x03,        // bDescriptorType (String)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x0E,        // bLength
0x03,        // bDescriptorType (String)
0x32, 0x72, 0x0E, 0x4E, 0x8C, 0x54, 0x73, 0x5E, 0x2E, 0x95, 0xD8, 0x76, 
0x12,        // bLength
0x03,        // bDescriptorType (String)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00,        // bLength
// 208 bytes


#endif


const uint8_t cdc_acm_hid_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_VPC_HID_SIZE, 0x04, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02),
    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09,                          /* bLength: Interface Descriptor size */
    USB_DESCRIPTOR_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
    0x02,                          /* bInterfaceNumber: Number of Interface */
    0x00,                          /* bAlternateSetting: Alternate setting */
    0x01,                          /* bNumEndpoints */
    0x03,                          /* bInterfaceClass: HID */
    0x01,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
    0x02,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
    0x00,                          /* iInterface: Index of string descriptor */
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09,                    /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE_HID, /* bDescriptorType: HID */
    0x11,                    /* bcdHID: HID Class Spec release number */
    0x01,
    0x00,                       /* bCountryCode: Hardware target country */
    0x01,                       /* bNumDescriptors: Number of HID class descriptors to follow */
    0x22,                       /* bDescriptorType */
    HID_MOUSE_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
    0x00,
    /******************** Descriptor of Mouse endpoint ********************/
    /* 27 */
    0x07,                         /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
    HID_INT_MS_EP,                   /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                         /* bmAttributes: Interrupt endpoint */
    HID_INT_MS_EP_SIZE,              /* wMaxPacketSize: 4 Byte max */
    0x00,
    HID_INT_MS_EP_INTERVAL, /* bInterval: Polling Interval */

	/************** Descriptor of keyboard interface ****************/
    /* 09 */
    0x09,                          /* bLength: Interface Descriptor size */
    USB_DESCRIPTOR_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
    0x03,                          /* bInterfaceNumber: Number of Interface */
    0x00,                          /* bAlternateSetting: Alternate setting */
    0x01,                          /* bNumEndpoints */
    0x03,                          /* bInterfaceClass: HID */
    0x01,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
    0x01,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
    0x00,                    	  /* iInterface: Index of string descriptor */
    /******************** Descriptor of keyboard HID ********************/
    /* 18 */
    0x09,                    /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE_HID, /* bDescriptorType: HID */
    0x11,                    /* bcdHID: HID Class Spec release number */
    0x01,
    0x00,                          /* bCountryCode: Hardware target country */
    0x01,                          /* bNumDescriptors: Number of HID class descriptors to follow */
    0x22,                          /* bDescriptorType */
    HID_KEYBOARD_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
    0x00,
    /******************** Descriptor of keyboard endpoint ********************/
    /* 27 */
    0x07,                         /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
    HID_INT_KB_EP,                   /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                         /* bmAttributes: Interrupt endpoint */
    HID_INT_KB_EP_SIZE,              /* wMaxPacketSize: 4 Byte max */
    0x00,
    HID_INT_KB_EP_INTERVAL, /* bInterval: Polling Interval */
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
	USB_DEVICE_STRING_MFC
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
  	USB_DEVICE_STRING_PRODUCT
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
	USB_DEVICE_STRING_SERIAL
	///////////////////////////////////////

    0x00
};


/*******************************************************

8bit功能按键位
8bit填充位
6*8bit所有按键位
104bit普通按键位
5bit LED显示位
3bit填充位

共22byte

*******************************************************/

#define USBD_KEYBOARD_REPORT_START_POS 		8



static const uint8_t hid_keyboard_report_desc[HID_KEYBOARD_REPORT_DESC_SIZE] = {
	 0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)  
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
	0x95, 0x06, 	   //	Report Count (6)
	0x75, 0x08, 	   //	Report Size (8)
	0x15, 0x00, 	   //	Logical Minimum (0)
	0x25, USBD_KEYBOARD_REPORT_CODEC_EXT_MAX, 	   //	Logical Maximum (231)
	0x05, 0x07, 	   //	Usage Page (Kbrd/Keypad)
	0x19, 0x00, 	   //	Usage Minimum (0x00)
	0x29, USBD_KEYBOARD_REPORT_CODEC_EXT_MAX, 	   //	Usage Maximum (0xe7)
	0x81, 0x00, 	   //	Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)



	 
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, USBD_KEYBOARD_REPORT_CODEC_START,                    //   USAGE_MINIMUM (Keyboard a and A)
    0x29, USBD_KEYBOARD_REPORT_CODEC_MAX,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x68,                    //   REPORT_COUNT (104)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
#if 0  
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
#endif
    0xc0                           // END_COLLECTION


};

/*******************************************************

BYTE1字节：
bit7～3：补充的常数，无意义，这里为0即可
bit2: 1表示中键按下
bit1: 1表示右键按下 0表示右键抬起
bit0: 1表示左键按下 0表示左键抬起
BYTE2 — X坐标变化量，与byte的bit4组成9位符号数,负数表示向左移，正数表右移。用补码表示变化量
BYTE3 — Y坐标变化量，与byte的bit5组成9位符号数，负数表示向下移，正数表上移。用补码表示变化量
BYTE4 — 滚轮变化。0x01表示滚轮向前滚动一格；0xFF表示滚轮向后滚动一格；0x80是个中间值，不滚动

4字节

*******************************************************/


/*!< hid mouse report descriptor */
static const uint8_t hid_mouse_report_desc[HID_MOUSE_REPORT_DESC_SIZE] = {
   0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x02, // USAGE (Mouse)
    0xA1, 0x01, // COLLECTION (Application)
    0x09, 0x01, //   USAGE (Pointer)

    0xA1, 0x00, //   COLLECTION (Physical)
    0x05, 0x09, //     USAGE_PAGE (Button)
    0x19, 0x01, //     USAGE_MINIMUM (Button 1)
    0x29, 0x03, //     USAGE_MAXIMUM (Button 3)

    0x15, 0x00, //     LOGICAL_MINIMUM (0)
    0x25, 0x01, //     LOGICAL_MAXIMUM (1)
    0x95, 0x03, //     REPORT_COUNT (3)
    0x75, 0x01, //     REPORT_SIZE (1)

    0x81, 0x02, //     INPUT (Data,Var,Abs)
    0x95, 0x01, //     REPORT_COUNT (1)
    0x75, 0x05, //     REPORT_SIZE (5)
    0x81, 0x01, //     INPUT (Cnst,Var,Abs)

    0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30, //     USAGE (X)
    0x09, 0x31, //     USAGE (Y)
    0x09, 0x38,

    0x15, 0x81, //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F, //     LOGICAL_MAXIMUM (127)
    0x75, 0x08, //     REPORT_SIZE (8)
    0x95, 0x03, //     REPORT_COUNT (2)

    0x81, 0x06, //     INPUT (Data,Var,Rel)
    0xC0, 0x09,
    0x3c, 0x05,
    0xff, 0x09,

    0x01, 0x15,
    0x00, 0x25,
    0x01, 0x75,
    0x01, 0x95,

    0x02, 0xb1,
    0x22, 0x75,
    0x06, 0x95,
    0x01, 0xb1,

    0x01, 0xc0 //   END_COLLECTION

// 51 bytes

// best guess: USB HID Report Descriptor
};

extern bool KB_ATOMIC_CAS(volatile uint8_t *pdst,uint8_t osrc,uint8_t nsrc);

void usbd_event_handler(uint8_t event)
{
    switch (event) {
        case USBD_EVENT_RESET:
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}


/*****************************************************************

VPC + HID init

*****************************************************************/


void USBD_CDC_ACM_HID_Init(void)
{
	HID_VCP_MODE_LOCK(NONE);
	
    usbd_desc_register(cdc_acm_hid_descriptor);

    usbd_add_interface(usbd_cdc_acm_init_intf(&cdc_acm_intf));
    usbd_add_interface(usbd_cdc_acm_init_intf(&cdc_acm_intf1));
    usbd_add_endpoint(&cdc_out_ep);
    usbd_add_endpoint(&cdc_in_ep);

    usbd_add_interface(usbd_hid_init_intf(&mouse_intf, hid_mouse_report_desc, HID_MOUSE_REPORT_DESC_SIZE));
    usbd_add_endpoint(&hid_ms_in_ep);
	usbd_add_interface(usbd_hid_init_intf(&keyboard_intf, hid_keyboard_report_desc, HID_KEYBOARD_REPORT_DESC_SIZE));
    usbd_add_endpoint(&hid_kb_in_ep);

    usbd_initialize();
}




/*************************************************************************************


VPC

**************************************************************************************/

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[2048];

void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual out len:%d\r\n", nbytes);
    usbd_ep_start_read(CDC_OUT_EP, read_buffer, 2048);
}

void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual in len:%d\r\n", nbytes);

    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes) 
	{
        /* send zlp */
        usbd_ep_start_write(CDC_IN_EP, NULL, 0);
    } 
	else 
	{
        ep_tx_busy_flag = HID_STATE_IDLE;
    }
}

void USBD_VPC_DTR_Enable(bool dtr)
{
    if (dtr) 
	{
        dtr_enable = true;
    }
	else 
	{
        dtr_enable = false;
    }
}

uint8_t USBD_VPC_Send(uint8_t *data, uint32_t data_len)
{
	uint16_t i = 10000;
	HID_VCP_MODE_LOCK(FALSE);
	 if (dtr_enable) 
	{
		while(i--)
		{
			if(KB_ATOMIC_CAS(&ep_tx_busy_flag,HID_STATE_IDLE,HID_STATE_BUSY))
			{
				return (0 == usbd_ep_start_write(CDC_IN_EP, data, data_len)); 
			}
		};		      
    }
	return false;
}


/*****************************************************************

键盘

*****************************************************************/

void usbd_hid_int_callback(uint8_t ep, uint32_t nbytes)
{
    if(ep == HID_INT_KB_EP)
	{
		hid_kb_state = HID_STATE_IDLE;
	}

	if(ep == HID_INT_MS_EP)
	{
		hid_ms_state = HID_STATE_IDLE;
	}
}

static void USBD_Keyboard_SetCodecBit(uint8_t *repbuffer, uint8_t codec)
{
	uint8_t cpos,bpos;
	if(codec > USBD_KEYBOARD_REPORT_CODEC_MAX || codec < USBD_KEYBOARD_REPORT_CODEC_START)
		return;

	codec -= USBD_KEYBOARD_REPORT_CODEC_START;
	cpos = (codec/8) + USBD_KEYBOARD_REPORT_START_POS;
	bpos = 0x01 << (codec%8);

	repbuffer[cpos] |= bpos;
}

static void USBD_Keyboard_ResetCodecBit(uint8_t *repbuffer, uint8_t codec)
{
	uint8_t cpos,bpos;
	if(codec > USBD_KEYBOARD_REPORT_CODEC_MAX || codec < USBD_KEYBOARD_REPORT_CODEC_START)
		return;
	codec -= USBD_KEYBOARD_REPORT_CODEC_START;
	cpos = (codec/8) + USBD_KEYBOARD_REPORT_START_POS;
	bpos = 0x01 << (codec%8);

	repbuffer[cpos] &= ~bpos;
}

void USBD_Keyboard_SetCodec(hid_keyboard * buf, uint8_t codec)
{
	if(codec >= USBD_KEYBOARD_REPORT_CODEC_EXT_MAX)
		return;

	if(codec >= HID_KBD_USAGE_LCTRL && codec <=  HID_KBD_USAGE_RGUI)
	{
		buf->buffer[0] |= (0x01 << (codec-HID_KBD_USAGE_LCTRL));
		return;
	}

	if(buf->bioskeypos < 6)
	{
		buf->buffer[buf->bioskeypos+2] = codec;
		buf->bioskeypos++;
	}
	else
	{
		if(codec > USBD_KEYBOARD_REPORT_CODEC_MAX || codec < USBD_KEYBOARD_REPORT_CODEC_START)
		{
			uint8_t tmp = buf->buffer[(buf->bioskeypos%6)+2];
			buf->buffer[(buf->bioskeypos%6)+2] = codec;
			buf->bioskeypos++;
			
			codec = tmp;
		}
		USBD_Keyboard_SetCodecBit(buf->buffer,codec);
	}
}

void USBD_Keyboard_ResetCodec(hid_keyboard * buf, uint8_t codec)
{
	uint8_t i;

	if(codec >= HID_KBD_USAGE_LCTRL && codec <=  HID_KBD_USAGE_RGUI)
	{
		buf->buffer[0] &= ~(0x01 << (codec-HID_KBD_USAGE_LCTRL));
		return;
	}

	for(i=2;i<8;i++)
	{
		if(buf->buffer[i] == codec)
		{
			buf->buffer[i] = 0x00;
		}
	}
	
	USBD_Keyboard_ResetCodecBit(buf->buffer,codec);
	
}


void USBD_Keyboard_FreeAll(hid_keyboard * buf)
{
	memset(buf,0,sizeof(hid_keyboard));
}

void USBD_Keyboard_SetLED(hid_keyboard * buf,hid_kb_led led)
{
	buf->buffer[USBD_KEYBOARD_REPORT_SIZE-1] |= (0x01 << led);
}
void USBD_Keyboard_ResetLED(hid_keyboard * buf,hid_kb_led led)
{
	buf->buffer[USBD_KEYBOARD_REPORT_SIZE-1] &= ~(0x01 << led);
}



void USBD_Print_Keyboard_Report(hid_keyboard *data)
{
	uint32_t* pbit = (uint32_t*)(&(data->buffer[USBD_KEYBOARD_REPORT_START_POS]));
	TRACELOG_DEBUG_PRINTF("Report[%x] [%x][%x][%x][%x][%x][%x]"
		,data->buffer[0],data->buffer[2],data->buffer[3],data->buffer[4],data->buffer[5],data->buffer[6],data->buffer[7]);
	TRACELOG_DEBUG_PRINTF("\t[%x]",pbit[0]);
	TRACELOG_DEBUG_PRINTF("\t[%x]",pbit[1]);
	TRACELOG_DEBUG_PRINTF("\t[%x]",pbit[2]);
}

uint8_t USBD_Keyboard_Send(hid_keyboard *data)
{
	uint16_t cout = 1000;
	HID_VCP_MODE_LOCK(TRUE);
	//TRACELOG_DEBUG_PRINTF("USBD_Keyboard_Send");

	while(!KB_ATOMIC_CAS(&hid_kb_state,HID_STATE_IDLE,HID_STATE_BUSY))
	{
		cout--;
		if(0 == cout)
		{
			TRACELOG_ERROR(usb_send_error);
			return false;
		}
	};
	//USBD_Print_Keyboard_Report(data);
	return (0 == usbd_ep_start_write(HID_INT_KB_EP, data->buffer, USBD_KEYBOARD_REPORT_SIZE));
	
}

/*****************************************************************

鼠标

*****************************************************************/

void  USBD_Mouse_SetButton(hid_mouse* rep,hid_botton_type button,uint8_t pressed)
{
	if(pressed)
	{
		rep->buttons |= ((0x01)<<button);
	}
	else
	{
		rep->buttons &= ~((0x01)<<button);
	}
}


uint8_t USBD_Mouse_Send(hid_mouse *data)
{
	HID_VCP_MODE_LOCK(TRUE);
	if(KB_ATOMIC_CAS(&hid_ms_state,HID_STATE_IDLE,HID_STATE_BUSY))
	{
		return (0 == usbd_ep_start_write(HID_INT_MS_EP, (uint8_t*)data, sizeof(hid_mouse)));
	}    
	return false;
}



static usbd_agent_mode_e sg_USB_Mode = USBD_AGENT_MODE_NONE;


usbd_agent_mode_e usbd_agent_mode()
{
	return sg_USB_Mode;
}
extern void MX_USB_DEVICE_DeInit(void);
void usbd_change_mode_2(usbd_agent_mode_e mode)
{
	if(mode == USBD_AGENT_MODE_NONE)
		return;
/*
	switch(sg_USB_Mode)
	{
		case USBD_AGENT_MODE_HID_VCP:
			usbd_deinitialize();
			break;
		case USBD_AGENT_MODE_MSC:
			MX_USB_DEVICE_DeInit();
			break;
		default:
			break;
	}
*/
	sg_USB_Mode = mode;
}


void usbd_start_hid_vcp_mode()
{
	usbd_change_mode_2(USBD_AGENT_MODE_HID_VCP);
	USBD_CDC_ACM_HID_Init();
	USBD_VPC_DTR_Enable(1);
}
void usbd_start_msc_mode()
{
	usbd_change_mode_2(USBD_AGENT_MODE_MSC);
	extern void MX_USB_DEVICE_Init(void);
	MX_USB_DEVICE_Init();
}



#ifdef TEST_USB

#include "tracelog.h"
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX hid_mouse mouse_cfg ={0};


void hid_mouse_test(void)
{
	TRACELOG_DEBUG("\ttest mouse. mouse to right.");
	
    /*!< move mouse pointer */
    mouse_cfg.x += 30;
    mouse_cfg.y = 0;

    USBD_Mouse_Send(&mouse_cfg);
}




void cdc_acm_data_send_with_dtr_test(void)
{
	TRACELOG_DEBUG("\ttest VPC.transmit a*2048.");
	memset(write_buffer,'a',2048);
	USBD_VPC_Send(write_buffer,2048);
}

//https://key-test.com/cn/
void hid_keyboard_bit_test(void)
{
	hid_keyboard keybuffer;
	uint8_t key;
	
	TRACELOG_DEBUG("\thid_keyboard_bit_test.");

	USBD_Keyboard_FreeAll(&keybuffer);
	for(key=USBD_KEYBOARD_REPORT_CODEC_START;key<=USBD_KEYBOARD_REPORT_CODEC_MAX;key++ )
	{
		USBD_Keyboard_SetCodecBit(keybuffer.buffer,key);
		USBD_Keyboard_Send(&keybuffer);
		
		HAL_Delay(20);
		
		USBD_Keyboard_ResetCodecBit(keybuffer.buffer,key);
		USBD_Keyboard_Send(&keybuffer);
		HAL_Delay(20);
	}

	TRACELOG_DEBUG("\thid_keyboard_bit_test end.");

}

//https://key-test.com/cn/
void hid_keyboard_bioskey_test(void)
{
	hid_keyboard keybuffer;
	uint8_t key;

	TRACELOG_DEBUG("\thid_keyboard_bioskey_test.");
	
	USBD_Keyboard_FreeAll(&keybuffer);
	for(key=USBD_KEYBOARD_REPORT_CODEC_START;key<=USBD_KEYBOARD_REPORT_CODEC_MAX;key++ )
	{
		USBD_Keyboard_SetCodec(&keybuffer,key);
		USBD_Keyboard_Send(&keybuffer);
		
		HAL_Delay(20);
		
		USBD_Keyboard_ResetCodec(&keybuffer,key);
		USBD_Keyboard_Send(&keybuffer);
		HAL_Delay(20);
	}
	TRACELOG_DEBUG("\thid_keyboard_bioskey_test end.");
}


void testcase_usb()
{
	TRACELOG_DEBUG("Running testcase_usb!...");
	
	//hid_keyboard_bit_test();
	//hid_keyboard_bioskey_test();
	HAL_Delay(1000);
	//cdc_acm_data_send_with_dtr_test();	
	HAL_Delay(1000);
	hid_mouse_test();
	
	TRACELOG_DEBUG("testcase_usb end!...");
}

#endif



#ifdef TEST_USB_MSC

extern uint8_t USBD_MSB_state(void);
void testcase_usbmsc()
{
	TRACELOG_DEBUG("Running testcase_usbmsc...");

	TRACELOG_DEBUG("\tstart msc");
	usbd_start_msc_mode();
	while(4 != USBD_MSB_state())
	{
		TRACELOG_DEBUG_PRINTF("MSB_state[%d]",USBD_MSB_state());
		HAL_Delay(1000);
	}

	TRACELOG_DEBUG("testcase_usbmsc end...");
}

#endif



