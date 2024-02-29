
#include "fonts.h"
#include "keyconfig.h"
#include "flash.h"
#include "memery.h"

typedef struct
{
	uint8_t 		type;
	uint8_t			enable;
#define FONTS_MSG_INITED 0x7dfb
	uint16_t		initfg;
	uint32_t 		flash_addr;
	uint32_t 		size;
}FlashFont_t;

typedef struct
{
	uint8_t 	Width;
	uint8_t 	FontWidth;
  	uint8_t 	Height;
	uint32_t 	len;
}font_info;

const static font_info csg_fontsinfo[FONT_MAX] =
{
	{8,6,8,8},//FONT_ASCII_5x8
	{8,7,12,12},//FONT_ASCII_7x12
	{16,10,16,32},//FONT_ASCII_11x16
	{16,11,20,40},//FONT_ASCII_14x20
	{24,14,24,72},//FONT_ASCII_17x24
	{16,10,40,80},//FONT_KEY_10x40
	{16,16,16,32},//FONT_EXT_16x616
};

static FlashFont_t sg_FontMsg[FONT_MAX];
static font_t sg_FontCache ;


uint8_t Fonts_Width(fonts_e type)
{
	if(type < FONT_MAX)
		return 0;
	return csg_fontsinfo[type].Width;
}

uint8_t Fonts_FontWidth(fonts_e type)
{
	if(type < FONT_MAX)
		return 0;
	return csg_fontsinfo[type].FontWidth;
}


uint8_t Fonts_Height(fonts_e type)
{
	if(type < FONT_MAX)
		return 0;
	return csg_fontsinfo[type].Height;
}


void Fonts_Init()
{
	IntFlash_Read(CONFIG_FONTMSG_ADDR, (uint8_t*)&sg_FontMsg,sizeof(sg_FontMsg));
}

font_t * Fonts_GetASCII(fonts_e type,unsigned char c)
{
	if(c > 0x7f || type >= FONT_MAX || c == 0)
	{
		return NULL;
	}

	if((0 == sg_FontMsg[type].enable) || (FONTS_MSG_INITED != sg_FontMsg[type].initfg))
	{
		return NULL;
	}

	if(c >= 0x20)
	{
		c -= 0x20;
	}
	else
	{
		c = 0x7f;
	}
	
	IntFlash_Read(sg_FontMsg[type].flash_addr+c*csg_fontsinfo[type].len, sg_FontCache.Font,csg_fontsinfo[type].len);
	
	sg_FontCache.Type = type;
	sg_FontCache.Width = csg_fontsinfo[type].Width;
	sg_FontCache.FontWidth = csg_fontsinfo[type].FontWidth;
	sg_FontCache.Height = csg_fontsinfo[type].Height;
	return &sg_FontCache;
}

font_t * Fonts_GeKey(fonts_e type,unsigned char c)
{
	if(type != FONT_KEY_10x40 || c == 0 || c>3)
	{
		return NULL;
	}

	if((0 == sg_FontMsg[type].enable) || (FONTS_MSG_INITED != sg_FontMsg[type].initfg))
	{
		return NULL;
	}

	c--;
	IntFlash_Read(sg_FontMsg[type].flash_addr+c*csg_fontsinfo[type].len, sg_FontCache.Font,csg_fontsinfo[type].len);
	
	sg_FontCache.Type = type;
	sg_FontCache.Width = csg_fontsinfo[type].Width;
	sg_FontCache.FontWidth = csg_fontsinfo[type].FontWidth;
	sg_FontCache.Height = csg_fontsinfo[type].Height;
	return &sg_FontCache;
}

font_t * Fonts_GeExt(fonts_e type,unsigned char c)
{
	if(type != FONT_EXT_16x16 || c > 0x54 || c < 0x4f)
	{
		return NULL;
	}

	if((0 == sg_FontMsg[type].enable) || (FONTS_MSG_INITED != sg_FontMsg[type].initfg))
	{
		return NULL;
	}

	c -= 0x4f;
	
	IntFlash_Read(sg_FontMsg[type].flash_addr+c*csg_fontsinfo[type].len, sg_FontCache.Font,csg_fontsinfo[type].len);
	
	sg_FontCache.Type = type;
	sg_FontCache.Width = csg_fontsinfo[type].Width;
	sg_FontCache.FontWidth = csg_fontsinfo[type].FontWidth;
	sg_FontCache.Height = csg_fontsinfo[type].Height;
	return &sg_FontCache;
}



font_t * Fonts_Get(fonts_e type,fonthandler* handler)
{
	font_t * ret = NULL;
	
	if(type <= FONT_ASCII_MAX)
	{
		ret = Fonts_GetASCII(type,handler->string[handler->startpos]);
		if(ret)
		{
			handler->startpos++;
		}
	}
	else
	{
		switch(type)
		{
			case FONT_KEY_10x40:
				ret = Fonts_GeKey(type,handler->string[handler->startpos]);
				if(ret)
				{
					handler->startpos++;
				}
				break;
			case FONT_EXT_16x16:
				ret = Fonts_GeExt(type,handler->string[handler->startpos]);
				if(ret)
				{
					handler->startpos++;
				}
				break;
		}
	}
	
	return ret;
}





#ifdef TEST_FONT
#include <string.h>
#include "flash.h"
#include "tracelog.h"
#include "usart.h"
#include "menu.h"

static FlashFont_t sg_WorkingFont = {0};

#define ASCII_5x8_SIZE 		768
#define ASCII_7x12_SIZE 	1152
#define ASCII_11x16_SIZE 	3072
#define ASCII_14x20_SIZE 	3840
#define ASCII_17x24_SIZE 	6912
#define ASCII_KEY_SIZE 		240
#define ASCII_EXT_SIZE 		192


#define ASCII_5x8_ADDR 		CONFIG_FONTS_ADDR
#define ASCII_7x12_ADDR 	(ASCII_5x8_ADDR+ASCII_5x8_SIZE)
#define ASCII_11x16_ADDR 	(ASCII_7x12_ADDR+ASCII_7x12_SIZE)
#define ASCII_14x20_ADDR 	(ASCII_11x16_ADDR+ASCII_11x16_SIZE)
#define ASCII_17x24_ADDR 	(ASCII_14x20_ADDR+ASCII_14x20_SIZE)

#define ASCII_KEY_ADDR 		(ASCII_17x24_ADDR+ASCII_17x24_SIZE)
#define ASCII_EXT_ADDR 		(ASCII_KEY_ADDR+ASCII_KEY_SIZE)


static FlashFont_t sg_FontMsgcfg[FONT_MAX] = 
{
	[FONT_ASCII_5x8] = 		{.flash_addr = ASCII_5x8_ADDR,.size = ASCII_5x8_SIZE},
	[FONT_ASCII_7x12] = 	{.flash_addr = ASCII_7x12_ADDR,.size = ASCII_7x12_SIZE},
	[FONT_ASCII_11x16] = 	{.flash_addr = ASCII_11x16_ADDR,.size = ASCII_11x16_SIZE},
	[FONT_ASCII_14x20] = 	{.flash_addr = ASCII_14x20_ADDR,.size = ASCII_14x20_SIZE},
	[FONT_ASCII_17x24] = 	{.flash_addr = ASCII_17x24_ADDR,.size = ASCII_17x24_SIZE},
	[FONT_KEY_10x40] = 		{.flash_addr = ASCII_KEY_ADDR,.size = ASCII_KEY_SIZE},
	[FONT_EXT_16x16] = 		{.flash_addr = ASCII_EXT_ADDR,.size = ASCII_EXT_SIZE},
};

uint8_t Check_FontByArra(fonts_e type, unsigned char * font)
{
	uint32_t i;
	uint32_t addr = sg_FontMsg[type].flash_addr;
	uint8_t data;
	uint32_t size = sg_FontMsg[type].size;
	TRACELOG_DEBUG_PRINTF("\tCheck_FontByArra.get type[%d] size[%d]!",type,size);
	for(i=0;i<size;i++)
	{
		 IntFlash_Read( i+addr, &data,1);
		 if(data != font[i])
		 {
			TRACELOG_DEBUG_PRINTF("check the flash error.get %dth char[%x] should[%x]!",i,data,font[i]);
			return 0;
		 }
	}
	TRACELOG_DEBUG("\tCHECK SUCCESS");
	return 1;
}

void SaveFontInfo2IntFlash(uint8_t type,uint32_t addr,uint32_t size)
{
	sg_WorkingFont.flash_addr = addr;
	sg_WorkingFont.size = size;
	sg_WorkingFont.type = type;
	sg_WorkingFont.enable = 1;
	sg_WorkingFont.initfg = FONTS_MSG_INITED;
	IntFlash_Write(CONFIG_FONTMSG_ADDR + type*sizeof(FlashFont_t),(uint8_t*)&sg_WorkingFont,sizeof(FlashFont_t));
}

void Font_Save(fonts_e type, uint8_t * data,uint32_t size)
{


	TRACELOG_DEBUG_PRINTF("\tSave font[%d] size[%d] flashaddr[%d]!",type,	size,sg_FontMsgcfg[type].flash_addr);
	if(sg_FontMsgcfg[type].size != size)
	{
		TRACELOG_DEBUG_PRINTF("Save font[%d] size[%d] not match[%d]!",type,	size,sg_FontMsgcfg[type].size);
	}

	IntFlash_Write(sg_FontMsgcfg[type].flash_addr,data,size);


	SaveFontInfo2IntFlash(type,sg_FontMsgcfg[type].flash_addr,size);

	Check_FontByArra(type,data);

	TRACELOG_DEBUG("\tSave font finish!");
	
}

#include "font8.c"
#include "font12.c"
#include "font16.c"
#include "font20.c"
#include "font24.c"
#include "fontext.c"

void test_fontsave()
{
	Font_Save(FONT_ASCII_5x8,Font8_Table,(uint32_t)sizeof(Font8_Table));
	Font_Save(FONT_ASCII_7x12,Font12_Table,(uint32_t)sizeof(Font12_Table));
	Font_Save(FONT_ASCII_11x16,Font16_Table,(uint32_t)sizeof(Font16_Table));
	Font_Save(FONT_ASCII_14x20,Font20_Table,(uint32_t)sizeof(Font20_Table));
	Font_Save(FONT_ASCII_17x24,Font24_Table,(uint32_t)sizeof(Font24_Table));
	Font_Save(FONT_KEY_10x40,key40,(uint32_t)sizeof(key40));	
	Font_Save(FONT_EXT_16x16,direct16,(uint32_t)sizeof(direct16));	
}

uint8_t * fontsarra[] = 
{
	Font8_Table,
	Font12_Table,
	Font16_Table,
	Font20_Table,
	Font24_Table,
	key40,
	direct16
};


void test_checkfont()
{
	uint8_t type = 0;
	uint32_t addrstart = ASCII_5x8_ADDR;
	TRACELOG_DEBUG("Running test_checkfont");
	Fonts_Init();

	for(type = 0;type<FONT_MAX;type++)
	{
		TRACELOG_DEBUG_PRINTF("\t---Font[%d] addr[%d] size[%d]",type,sg_FontMsg[type].flash_addr,sg_FontMsg[type].size);
		if((0 == sg_FontMsg[type].enable) || (FONTS_MSG_INITED != sg_FontMsg[type].initfg))
		{
			TRACELOG_DEBUG_PRINTF("Font[%d] not enable!enable[%d] init[%x]",type,sg_FontMsg[type].enable,sg_FontMsg[type].initfg);
			return;
		}

		if(addrstart != sg_FontMsg[type].flash_addr)
		{
			TRACELOG_DEBUG_PRINTF("Font[%d] addr[%d] not match[%x]",type,sg_FontMsg[type].flash_addr,addrstart);
			return;	
		}

		if(0 == Check_FontByArra((fonts_e)type,fontsarra[type]))
		{
			TRACELOG_DEBUG_PRINTF("Font[%d] check error",type);
			return;
		}
		addrstart += sg_FontMsg[type].size;
	}
	addrstart -= ASCII_5x8_ADDR;
	TRACELOG_DEBUG_PRINTF("\tCheck Success!Totalsize[%dB]-[%dKB][%dB]",addrstart,addrstart/1024,addrstart%1024);

}

void test_downloadchinese()
{
	Main_Menu();
}

void testcase_font()
{
	
	//test_fontsave();
	//test_checkfont();
	test_downloadchinese();
}


#endif




