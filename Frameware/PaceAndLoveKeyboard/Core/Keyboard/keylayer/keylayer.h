
#ifndef _KEY_LAYER_H_
#define _KEY_LAYER_H_

#include "main.h"
#include "fonts.h"


#define KEY_CODE_NONE 		0x00 // No key pressed
#define KEY_CODE_ERROVF 	0x01 //  Keyboard Error Roll Over - used for all slots if too many keys are pressed ("Phantom key")
#define KEY_CODE_POSTFAIL 	0x02 //  Keyboard POST Fail
#define KEY_CODE_ERRUNDEF 	0x03 //  Keyboard Error Undefined
#define KEY_CODE_A 0x04 // Keyboard a and A
#define KEY_CODE_B 0x05 // Keyboard b and B
#define KEY_CODE_C 0x06 // Keyboard c and C
#define KEY_CODE_D 0x07 // Keyboard d and D
#define KEY_CODE_E 0x08 // Keyboard e and E
#define KEY_CODE_F 0x09 // Keyboard f and F
#define KEY_CODE_G 0x0a // Keyboard g and G
#define KEY_CODE_H 0x0b // Keyboard h and H
#define KEY_CODE_I 0x0c // Keyboard i and I
#define KEY_CODE_J 0x0d // Keyboard j and J
#define KEY_CODE_K 0x0e // Keyboard k and K
#define KEY_CODE_L 0x0f // Keyboard l and L
#define KEY_CODE_M 0x10 // Keyboard m and M
#define KEY_CODE_N 0x11 // Keyboard n and N
#define KEY_CODE_O 0x12 // Keyboard o and O
#define KEY_CODE_P 0x13 // Keyboard p and P
#define KEY_CODE_Q 0x14 // Keyboard q and Q
#define KEY_CODE_R 0x15 // Keyboard r and R
#define KEY_CODE_S 0x16 // Keyboard s and S
#define KEY_CODE_T 0x17 // Keyboard t and T
#define KEY_CODE_U 0x18 // Keyboard u and U
#define KEY_CODE_V 0x19 // Keyboard v and V
#define KEY_CODE_W 0x1a // Keyboard w and W
#define KEY_CODE_X 0x1b // Keyboard x and X
#define KEY_CODE_Y 0x1c // Keyboard y and Y
#define KEY_CODE_Z 0x1d // Keyboard z and Z

#define KEY_CODE_1 0x1e // Keyboard 1 and !
#define KEY_CODE_2 0x1f // Keyboard 2 and @
#define KEY_CODE_3 0x20 // Keyboard 3 and #
#define KEY_CODE_4 0x21 // Keyboard 4 and $
#define KEY_CODE_5 0x22 // Keyboard 5 and %
#define KEY_CODE_6 0x23 // Keyboard 6 and ^
#define KEY_CODE_7 0x24 // Keyboard 7 and &
#define KEY_CODE_8 0x25 // Keyboard 8 and *
#define KEY_CODE_9 0x26 // Keyboard 9 and (
#define KEY_CODE_0 0x27 // Keyboard 0 and )

#define KEY_CODE_ENTER 		0x28 // Keyboard Return (ENTER)
#define KEY_CODE_ESC 		0x29 // Keyboard ESCAPE
#define KEY_CODE_BACKSPACE 0x2a // Keyboard DELETE (Backspace)
#define KEY_CODE_TAB 		0x2b // Keyboard Tab
#define KEY_CODE_SPACE 		0x2c // Keyboard Spacebar
#define KEY_CODE_MINUS 		0x2d // Keyboard - and _
#define KEY_CODE_EQUAL 		0x2e // Keyboard = and +
#define KEY_CODE_LEFTBRACE 0x2f // Keyboard [ and {
#define KEY_CODE_RIGHTBRACE 0x30 // Keyboard ] and }
#define KEY_CODE_BACKSLASH 0x31 // Keyboard \ and |
#define KEY_CODE_HASHTILDE 0x32 // Keyboard Non-US # and ~
#define KEY_CODE_SEMICOLON 0x33 // Keyboard ; and :
#define KEY_CODE_APOSTROPHE 0x34 // Keyboard ' and "
#define KEY_CODE_GRAVE 		0x35 // Keyboard ` and ~
#define KEY_CODE_COMMA 		0x36 // Keyboard , and <
#define KEY_CODE_DOT 		0x37 // Keyboard . and >
#define KEY_CODE_SLASH 		0x38 // Keyboard / and ?
#define KEY_CODE_CAPSLOCK 	0x39 // Keyboard Caps Lock

#define KEY_CODE_F1 0x3a // Keyboard F1
#define KEY_CODE_F2 0x3b // Keyboard F2
#define KEY_CODE_F3 0x3c // Keyboard F3
#define KEY_CODE_F4 0x3d // Keyboard F4
#define KEY_CODE_F5 0x3e // Keyboard F5
#define KEY_CODE_F6 0x3f // Keyboard F6
#define KEY_CODE_F7 0x40 // Keyboard F7
#define KEY_CODE_F8 0x41 // Keyboard F8
#define KEY_CODE_F9 0x42 // Keyboard F9
#define KEY_CODE_F10 0x43 // Keyboard F10
#define KEY_CODE_F11 0x44 // Keyboard F11
#define KEY_CODE_F12 0x45 // Keyboard F12

#define KEY_CODE_SYSRQ 			0x46 // Keyboard Print Screen
#define KEY_CODE_SCROLLLOCK 	0x47 // Keyboard Scroll Lock
#define KEY_CODE_PAUSE 			0x48 // Keyboard Pause
#define KEY_CODE_INSERT 		0x49 // Keyboard Insert
#define KEY_CODE_HOME 			0x4a // Keyboard Home
#define KEY_CODE_PAGEUP 		0x4b // Keyboard Page Up
#define KEY_CODE_DELETE 		0x4c // Keyboard Delete Forward
#define KEY_CODE_END 			0x4d // Keyboard End
#define KEY_CODE_PAGEDOWN 		0x4e // Keyboard Page Down
#define KEY_CODE_RIGHT 			0x4f // Keyboard Right Arrow
#define KEY_CODE_LEFT 			0x50 // Keyboard Left Arrow
#define KEY_CODE_DOWN 			0x51 // Keyboard Down Arrow
#define KEY_CODE_UP 			0x52 // Keyboard Up Arrow

#define KEY_CODE_NUMLOCK 0x53 // Keyboard Num Lock and Clear
#define KEY_CODE_KPSLASH 0x54 // Keypad /
#define KEY_CODE_KPASTERISK 0x55 // Keypad *
#define KEY_CODE_KPMINUS 0x56 // Keypad -
#define KEY_CODE_KPPLUS 0x57 // Keypad +
#define KEY_CODE_KPENTER 0x58 // Keypad ENTER
#define KEY_CODE_KP1 0x59 // Keypad 1 and End
#define KEY_CODE_KP2 0x5a // Keypad 2 and Down Arrow
#define KEY_CODE_KP3 0x5b // Keypad 3 and PageDn
#define KEY_CODE_KP4 0x5c // Keypad 4 and Left Arrow
#define KEY_CODE_KP5 0x5d // Keypad 5
#define KEY_CODE_KP6 0x5e // Keypad 6 and Right Arrow
#define KEY_CODE_KP7 0x5f // Keypad 7 and Home
#define KEY_CODE_KP8 0x60 // Keypad 8 and Up Arrow
#define KEY_CODE_KP9 0x61 // Keypad 9 and Page Up
#define KEY_CODE_KP0 0x62 // Keypad 0 and Insert
#define KEY_CODE_KPDOT 0x63 // Keypad . and Delete

#define KEY_CODE_102ND 0x64 // Keyboard Non-US \ and |
#define KEY_CODE_COMPOSE 0x65 // Keyboard Application
#define KEY_CODE_POWER 0x66 // Keyboard Power
#define KEY_CODE_KPEQUAL 0x67 // Keypad =

#define KEY_CODE_F13 0x68 // Keyboard F13
#define KEY_CODE_F14 0x69 // Keyboard F14
#define KEY_CODE_F15 0x6a // Keyboard F15
#define KEY_CODE_F16 0x6b // Keyboard F16
#define KEY_CODE_F17 0x6c // Keyboard F17
#define KEY_CODE_F18 0x6d // Keyboard F18
#define KEY_CODE_F19 0x6e // Keyboard F19
#define KEY_CODE_F20 0x6f // Keyboard F20
#define KEY_CODE_F21 0x70 // Keyboard F21
#define KEY_CODE_F22 0x71 // Keyboard F22
#define KEY_CODE_F23 0x72 // Keyboard F23
#define KEY_CODE_F24 0x73 // Keyboard F24

#define KEY_CODE_OPEN 0x74 // Keyboard Execute
#define KEY_CODE_HELP 0x75 // Keyboard Help
#define KEY_CODE_PROPS 0x76 // Keyboard Menu
#define KEY_CODE_FRONT 0x77 // Keyboard Select
#define KEY_CODE_STOP 0x78 // Keyboard Stop
#define KEY_CODE_AGAIN 0x79 // Keyboard Again
#define KEY_CODE_UNDO 0x7a // Keyboard Undo
#define KEY_CODE_CUT 0x7b // Keyboard Cut
#define KEY_CODE_COPY 0x7c // Keyboard Copy
#define KEY_CODE_PASTE 0x7d // Keyboard Paste
#define KEY_CODE_FIND 0x7e // Keyboard Find
#define KEY_CODE_MUTE 0x7f // Keyboard Mute
#define KEY_CODE_VOLUMEUP 0x80 // Keyboard Volume Up
#define KEY_CODE_VOLUMEDOWN 0x81 // Keyboard Volume Down

#define KEY_CODE_KPCOMMA 0x85 // Keypad Comma

#define KEY_CODE_RO 0x87 // Keyboard International1
#define KEY_CODE_KATAKANAHIRAGANA 0x88 // Keyboard International2
#define KEY_CODE_YEN 0x89 // Keyboard International3
#define KEY_CODE_HENKAN 0x8a // Keyboard International4
#define KEY_CODE_MUHENKAN 0x8b // Keyboard International5
#define KEY_CODE_KPJPCOMMA 0x8c // Keyboard International6

#define KEY_CODE_HANGEUL 0x90 // Keyboard LANG1
#define KEY_CODE_HANJA 0x91 // Keyboard LANG2
#define KEY_CODE_KATAKANA 0x92 // Keyboard LANG3
#define KEY_CODE_HIRAGANA 0x93 // Keyboard LANG4
#define KEY_CODE_ZENKAKUHANKAKU 0x94 // Keyboard LANG5

#define KEY_CODE_KPLEFTPAREN 0xb6 // Keypad (
#define KEY_CODE_KPRIGHTPAREN 0xb7 // Keypad )

#define KEY_CODE_LEFTCTRL 0xe0 // Keyboard Left Control
#define KEY_CODE_LEFTSHIFT 0xe1 // Keyboard Left Shift
#define KEY_CODE_LEFTALT 0xe2 // Keyboard Left Alt
#define KEY_CODE_LEFTMETA 0xe3 // Keyboard Left GUI
#define KEY_CODE_RIGHTCTRL 0xe4 // Keyboard Right Control
#define KEY_CODE_RIGHTSHIFT 0xe5 // Keyboard Right Shift
#define KEY_CODE_RIGHTALT 0xe6 // Keyboard Right Alt
#define KEY_CODE_RIGHTMETA 0xe7 // Keyboard Right GUI

#define KEY_CODE_MEDIA_PLAYPAUSE 0xe8
#define KEY_CODE_MEDIA_STOPCD 0xe9
#define KEY_CODE_MEDIA_PREVIOUSSONG 0xea
#define KEY_CODE_MEDIA_NEXTSONG 0xeb
#define KEY_CODE_MEDIA_EJECTCD 0xec
#define KEY_CODE_MEDIA_VOLUMEUP 0xed
#define KEY_CODE_MEDIA_VOLUMEDOWN 0xee
#define KEY_CODE_MEDIA_MUTE 0xef
#define KEY_CODE_MEDIA_WWW 0xf0
#define KEY_CODE_MEDIA_BACK 0xf1
#define KEY_CODE_MEDIA_FORWARD 0xf2
#define KEY_CODE_MEDIA_STOP 0xf3
#define KEY_CODE_MEDIA_FIND 0xf4
#define KEY_CODE_MEDIA_SCROLLUP 0xf5
#define KEY_CODE_MEDIA_SCROLLDOWN 0xf6
#define KEY_CODE_MEDIA_EDIT 0xf7
#define KEY_CODE_MEDIA_SLEEP 0xf8
#define KEY_CODE_MEDIA_COFFEE 0xf9
#define KEY_CODE_MEDIA_REFRESH 0xfa
#define KEY_CODE_MEDIA_CALC 0xfb



#define MAX_KEY_NUM			104
#define MAX_KEYLAYER_NUM 	8
#define MAX_KEYCOMBINE_NUM 4//4

typedef struct
{
	fonts_e type[2];
	uint8_t upline[5];
	uint8_t downline[5];
}KeyCodeDis_t;



typedef uint8_t keybit_t;
typedef uint8_t keycode_t;
typedef void (*ForEachCodeFunc_f)(keycode_t,uint8_t);
typedef keycode_t keycombine_t[MAX_KEYCOMBINE_NUM];//4 byte

void KEY_KeyLayer_Init(void);

bool KeyLayer_IsLocked(void);
void KeyLayer_SetLock(uint8_t lock);

keybit_t KeyLayer_GetKeyNum(void);
keycode_t KeyLayer_GetBitCodec(keybit_t bit);

uint8_t KEY_Change2KeyLayer(uint8_t layer);
uint8_t KEY_Set_KeyLayer_Combine(keybit_t bit,keycombine_t combine);

uint8_t* KEY_Get_KeyLayer_Name(uint8_t layer);
void KEY_Set_CurrentKeyLayer_Name(uint8_t* name);
uint8_t KEY_Save2KeyLayer(uint8_t layer);
uint8_t KEY_SaveCurrentKeyLayer(void);
uint8_t KEY_GetCurrentKeyLayer(void);

void KeyLayer_Delete(uint8_t layer);



KeyCodeDis_t* KeyLayer_GetCodecDiscribe(keybit_t bit);


void For_Each_Bit_KeyCode(keybit_t bit,ForEachCodeFunc_f func,uint8_t pressed);

#endif



