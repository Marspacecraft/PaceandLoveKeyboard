
#include "memery.h"
#include "tracelog.h"
#include "keyconfig.h"



static uint16_t sg_LastChar_endian = 0xff00;


void IntFlash_Init()
{
	uint8_t lastchar = ((uint8_t*)&sg_LastChar_endian)[0];
	if(0x00 == lastchar)
		sg_LastChar_endian = 0x00ff;
}

static uint16_t stmflash_read_halfword(uint32_t faddr)
{
	faddr += INTFLASH_ADDR_START;
    return *(volatile uint16_t *)faddr;
}

static uint32_t stmflash_read_word(uint32_t faddr)
{
	faddr += INTFLASH_ADDR_START;
    return *(volatile uint32_t *)faddr;
}


void stmflash_read(uint32_t raddr, uint16_t *pbuf, uint16_t length)
{
    uint16_t i;

    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_halfword(raddr);    /* ?¨¢¨¨?2??¡Á??¨² */
        raddr += 2; /* ??¨°?2??¡Á??¨² */
    }
}
void IntFlash_Read(uint32_t raddr, uint8_t *pbuf, uint16_t length)
{
	stmflash_read(raddr,(uint16_t*)pbuf,(length)>>1);
	if(length & 0x0001)
	{
		pbuf[length-1] = (uint8_t)(stmflash_read_halfword(raddr + length-1)&sg_LastChar_endian);
	}
}

HAL_StatusTypeDef InitFlash_PageErase(uint32_t eindex,uint32_t num)
{
	uint32_t SECTORError = 0;
	HAL_StatusTypeDef ret;
	static FLASH_EraseInitTypeDef EraseInitStruct;
	TRACELOG_INFO("InitFlash_PageErase.");
	if((num+eindex)>=(INTFLASH_SIZE / FLASH_PAGE_SIZE))
	{
		TRACELOG_INFO("Erase addr larger than flash size!");
		return HAL_ERROR;
	}

	EraseInitStruct.TypeErase	  = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.NbPages 	  = num;
	EraseInitStruct.PageAddress   = eindex * STM32_SECTOR_SIZE + INTFLASH_ADDR_START;
	HAL_FLASH_Unlock();
	ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
	if(HAL_OK != ret)
	{
		TRACELOG_DEBUG("HAL_FLASHEx_Erase error!");
	}
	HAL_FLASH_Lock();
	return ret;
}


void stmflash_write_nocheck(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
    uint16_t i;

	waddr += INTFLASH_ADDR_START;
	HAL_FLASH_Unlock(); 
    for (i = 0; i < length; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, waddr, pbuf[i]);
        waddr += 2; /* ???¨°??¨°???¡ã?¡Á? */
    }
	HAL_FLASH_Lock(); /* ¨¦??? */

}

void IntFlash_Write_NoCheck(uint32_t waddr, uint8_t *pbuf, uint16_t length)
{
	uint16_t value;
	
	stmflash_write_nocheck(waddr,(uint16_t*)pbuf,(length)>>1);
	if(length & 0x0001)
	{
		value = stmflash_read_halfword(waddr + length-1);		
		value = (value&~sg_LastChar_endian)|((uint16_t)pbuf[length - 1]);
		stmflash_write_nocheck(waddr + length-1 , &value,1);
	}

}


uint16_t g_intflashbuf[STM32_SECTOR_SIZE/2]; /* ¡Á??¨¤¨º?2K¡Á??¨² */
void stmflash_write(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
    uint32_t secpos;    /* ¨¦¨¨??¦Ì??¡¤ */
    uint16_t secoff;    /* ¨¦¨¨???¨²??¨°?¦Ì??¡¤(16??¡Á?????) */
    uint16_t secremain; /* ¨¦¨¨???¨²¨º¡ê¨®¨¤¦Ì??¡¤(16??¡Á?????) */
    uint16_t i;
    uint32_t offaddr;   /* ¨¨£¤¦Ì?0X08000000o¨®¦Ì?¦Ì??¡¤ */
    FLASH_EraseInitTypeDef flash_eraseop;
    uint32_t erase_addr;   /* 2¨¢3y¡ä¨ª?¨®¡ê??a???¦Ì?a¡¤¡é¨¦¨²¡ä¨ª?¨®¦Ì?¨¦¨¨??¦Ì??¡¤ */

    if ( (waddr + length * 2) >= (INTFLASH_ADDR_END - INTFLASH_ADDR_START))
    {
        return; /* ¡¤?¡¤¡§¦Ì??¡¤ */
    }

    offaddr = waddr;       /* ¨º¦Ì?¨º??¨°?¦Ì??¡¤. */
    secpos = offaddr / STM32_SECTOR_SIZE;       /* ¨¦¨¨??¦Ì??¡¤  0~127 for STM32F103RBT6 */
    secoff = (offaddr % STM32_SECTOR_SIZE) / 2; /* ?¨²¨¦¨¨???¨²¦Ì???¨°?(2??¡Á??¨²?a?¨´¡À?¦Ì£¤??.) */
    secremain = STM32_SECTOR_SIZE / 2 - secoff; /* ¨¦¨¨??¨º¡ê¨®¨¤????¡ä¨®D? */
    if (length <= secremain)
    {
        secremain = length; /* 2?¡ä¨®¨®¨²??¨¦¨¨??¡¤??¡ì */
    }

    while (1)
    {
        stmflash_read(secpos * STM32_SECTOR_SIZE, g_intflashbuf, STM32_SECTOR_SIZE / 2); /* ?¨¢3?????¨¦¨¨??¦Ì??¨²¨¨Y */
        for (i = 0; i < secremain; i++)                                                              /* D¡ê?¨¦¨ºy?Y */
        {
            if (g_intflashbuf[secoff + i] != 0XFFFF)
            {
                break; /* D¨¨¨°a2¨¢3y */
            }
        }
        if (i < secremain) /* D¨¨¨°a2¨¢3y */
        { 
        	HAL_FLASH_Unlock(); 
            flash_eraseop.TypeErase = FLASH_TYPEERASE_PAGES;        /* ??????2¨¢3y */
            flash_eraseop.Banks = FLASH_BANK_1;
            flash_eraseop.NbPages = 1;
            flash_eraseop.PageAddress = secpos * STM32_SECTOR_SIZE + INTFLASH_ADDR_START;  /* ¨°a2¨¢3y¦Ì?¨¦¨¨?? */
            HAL_FLASHEx_Erase( &flash_eraseop, &erase_addr);
			HAL_FLASH_Lock(); 
			
            for (i = 0; i < secremain; i++)                               /* ?¡ä?? */
            {
                g_intflashbuf[i + secoff] = pbuf[i];
            }
            stmflash_write_nocheck(secpos * STM32_SECTOR_SIZE, g_intflashbuf, STM32_SECTOR_SIZE / 2); /* D¡ä¨¨?????¨¦¨¨?? */
        }
        else
        {
            stmflash_write_nocheck(waddr, pbuf, secremain); /* D¡ä¨°??-2¨¢3y¨¢?¦Ì?,?¡À?¨®D¡ä¨¨?¨¦¨¨??¨º¡ê¨®¨¤????. */
        }
        if (length == secremain)
        {
            break; /* D¡ä¨¨??¨¢¨º?¨¢? */
        }
        else       /* D¡ä¨¨??¡ä?¨¢¨º? */
        {
            secpos++;               /* ¨¦¨¨??¦Ì??¡¤??1 */
            secoff = 0;             /* ??¨°??????a0 */
            pbuf += secremain;      /* ??????¨°? */
            waddr += secremain * 2; /* D¡ä¦Ì??¡¤??¨°?(16??¨ºy?Y¦Ì??¡¤,D¨¨¨°a*2) */
            length -= secremain;    /* ¡Á??¨²(16??)¨ºy¦ÌY?? */
            if (length > (STM32_SECTOR_SIZE / 2))
            {
                secremain = STM32_SECTOR_SIZE / 2; /* ??¨°???¨¦¨¨???1¨º?D¡ä2?¨ª¨º */
            }
            else
            {
                secremain = length; /* ??¨°???¨¦¨¨???¨¦¨°?D¡ä¨ª¨º¨¢? */
            }
        }
    }

}

void IntFlash_Write_Check(uint32_t waddr, uint8_t *pbuf, uint16_t length)
{
	uint16_t value;
	stmflash_write(waddr,(uint16_t*)pbuf,(length)>>1);
	
	if(length & 0x0001)
	{
		value = stmflash_read_halfword(waddr + length-1);		
		value = (value&~sg_LastChar_endian)|((uint16_t)pbuf[length - 1]);
		stmflash_write(waddr + length-1,&value,1);		
	}
}

uint8_t IntFlash_WritePosEmpty(uint32_t addr, uint16_t datalen)
{
	uint16_t len,i;
	uint8_t* pbuf = (uint8_t*)g_intflashbuf;

	do
	{
		if((STM32_SECTOR_SIZE)<datalen)
		{
			
			len = STM32_SECTOR_SIZE;
			datalen -= STM32_SECTOR_SIZE;
		}
		else
			len = datalen;
		IntFlash_Read(addr,pbuf,len);
		addr += len;
		
		for(i=0;i<len;i++)
		{
			if(0xff != pbuf[i])
				return 0;
		}
	}while(len != datalen);

	return 1;
}

void IntFlash_Write(uint32_t waddr, uint8_t *pbuf, uint16_t length)
{
	if(IntFlash_WritePosEmpty(waddr,length))
	{
		IntFlash_Write_NoCheck(waddr,pbuf,length);
	}
	else
	{
		IntFlash_Write_Check(waddr,pbuf,length);
	}

}

void MEM_FlagBit_Set(uint32_t flagbit)
{
	uint32_t flag = stmflash_read_word(CONFIG_FLAGBIT_ADDR);
	flag &= ~flagbit;
	IntFlash_Write_Check(CONFIG_FLAGBIT_ADDR,(uint8_t*)&flag,CONFIG_FLAGBIT_SIZE);
}

void MEM_FlagBit_Reset(uint32_t flagbit)
{
	uint32_t flag = stmflash_read_word(CONFIG_FLAGBIT_ADDR);
	flag |= flagbit;
	IntFlash_Write_Check(CONFIG_FLAGBIT_ADDR,(uint8_t*)&flag,CONFIG_FLAGBIT_SIZE);
}

bool MEM_FlagBit_IsEnabled(uint32_t flagbit)
{
	uint32_t flag = stmflash_read_word(CONFIG_FLAGBIT_ADDR);
	return !(flag & flagbit);
}


bool IntFlash_MSC_Read(uint32_t addr, uint16_t size, uint8_t *data)
{
	if((addr+size)>= INTFLASH_MSC_SIZE)
		return false;
	
	addr += (INTFLASH_MSC_ADDR_START-INTFLASH_ADDR_START);
	IntFlash_Read(addr,data,size);
	
	return true;
}

bool IntFlash_MSC_Write(uint32_t addr, uint16_t size, uint8_t *data)
{

	if((addr+size)>= INTFLASH_MSC_SIZE)
		return false;
	
	addr += (INTFLASH_MSC_ADDR_START-INTFLASH_ADDR_START);
	IntFlash_Write_Check(addr,data,size);
	
	return true;
}


#ifdef TEST_INTFLASH
#include "string.h"

#define INTFLASH_BUFFER_SIZE  (2*STM32_SECTOR_SIZE)
uint8_t intflash_rx[INTFLASH_BUFFER_SIZE] = {0};
uint8_t intflash_tx[INTFLASH_BUFFER_SIZE] = {0};
uint8_t intflash_tx_zero[INTFLASH_BUFFER_SIZE] = {0};

#define TEST_INTFLASH_CHAR		0xc8


bool check_Intbuffer(uint16_t len)
{
	uint16_t i = 0;
	for(i=0;i<len;i++)
	{
		if(intflash_rx[i]!=intflash_tx[i])
			return false;
	}
	return true;
}



void test_intflash_page_rw_nocheck()
{

	TRACELOG_DEBUG("\ttest_intflash_page_rw_nocheck...");
	
	TRACELOG_DEBUG("\t\ttest aligned.");
	InitFlash_PageErase(0,1);

	IntFlash_Write(0,intflash_tx,STM32_SECTOR_SIZE);

	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,STM32_SECTOR_SIZE);
	if(check_Intbuffer(STM32_SECTOR_SIZE))
	{
		TRACELOG_DEBUG("\t\ttest aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttest aligned fail.");
		TRACELOG_DEBUG("\ttest_intflash_page_rw_nocheck fail.");
		return;
	}

	
	TRACELOG_DEBUG("\t\ttest not aligned.");
	InitFlash_PageErase(0,1);
	IntFlash_Write(0,intflash_tx,STM32_SECTOR_SIZE-1);
	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,STM32_SECTOR_SIZE);
	if(check_Intbuffer(STM32_SECTOR_SIZE-1)&&(0xff == intflash_rx[STM32_SECTOR_SIZE-1]))
	{
		TRACELOG_DEBUG("\t\ttest not aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\ttest_intflash_page_rw_nocheck fail.");
		return;
	}

	TRACELOG_DEBUG("\ttest_intflash_page_rw_nocheck success.");


}

void test_intflash_page_rw_check()
{
	TRACELOG_DEBUG("\ttest_intflash_page_rw_check...");
		
	TRACELOG_DEBUG("\t\ttest aligned.");

	IntFlash_Write(0,intflash_tx,STM32_SECTOR_SIZE);
	IntFlash_Write(0,intflash_tx,STM32_SECTOR_SIZE);
	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,STM32_SECTOR_SIZE);
	if(check_Intbuffer(STM32_SECTOR_SIZE))
	{
		TRACELOG_DEBUG("\t\ttest aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttest aligned fail.");
		TRACELOG_DEBUG("\ttest_intflash_page_rw_check fail.");
		return;
	}
	TRACELOG_DEBUG("\t\ttest not aligned.");
	//InitFlash_PageErase(0);

	IntFlash_Write(0,intflash_tx_zero,STM32_SECTOR_SIZE);
	IntFlash_Write(0,intflash_tx,STM32_SECTOR_SIZE-1);
	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,STM32_SECTOR_SIZE);
	if(check_Intbuffer(STM32_SECTOR_SIZE-1)&&(0 == intflash_rx[STM32_SECTOR_SIZE-1]))
	{
		TRACELOG_DEBUG("\t\ttest not aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttest not aligned fail.");
		TRACELOG_DEBUG("\ttest_intflash_page_rw_check fail.");
		return;
	}

	TRACELOG_DEBUG("\ttest_intflash_page_rw_check success.");

}

void test_intflash_multipage_rw_nocheck()
{

	TRACELOG_DEBUG("\ttest_intflash_multipage_rw_nocheck...");
	
	TRACELOG_DEBUG("\t\ttest aligned.");
	InitFlash_PageErase(0,1);
	InitFlash_PageErase(1,1);
	IntFlash_Write(0,intflash_tx,INTFLASH_BUFFER_SIZE);

	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,INTFLASH_BUFFER_SIZE);
	if(check_Intbuffer(INTFLASH_BUFFER_SIZE))
	{
		TRACELOG_DEBUG("\t\ttest aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttest aligned fail.");
		TRACELOG_DEBUG("\ttest_intflash_multipage_rw_nocheck fail.");
		return;
	}
	TRACELOG_DEBUG("\t\ttest not aligned.");
	InitFlash_PageErase(0,1);
	InitFlash_PageErase(1,1);

	IntFlash_Write(0,intflash_tx,INTFLASH_BUFFER_SIZE-1);
	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,INTFLASH_BUFFER_SIZE);
	if(check_Intbuffer(INTFLASH_BUFFER_SIZE-1)&&(0xff == intflash_rx[INTFLASH_BUFFER_SIZE-1]))
	{
		TRACELOG_DEBUG("\t\ttest not aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttest not aligned fail.");
		TRACELOG_DEBUG("\ttest_intflash_multipage_rw_nocheck fail.");
		return;
	}

	TRACELOG_DEBUG("\ttest_intflash_multipage_rw_nocheck success.");


}

void test_intflash_multipage_rw_check()
{
	TRACELOG_DEBUG("\ttest_intflash_multipage_rw_check...");
		
	TRACELOG_DEBUG("\t\ttest aligned.");
	//InitFlash_PageErase(0);

	IntFlash_Write(0,intflash_tx,INTFLASH_BUFFER_SIZE);
	IntFlash_Write(0,intflash_tx,INTFLASH_BUFFER_SIZE);
	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,INTFLASH_BUFFER_SIZE);
	if(check_Intbuffer(INTFLASH_BUFFER_SIZE))
	{
		TRACELOG_DEBUG("\t\ttest aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttest aligned fail.");
		TRACELOG_DEBUG("\ttest_intflash_multipage_rw_check fail.");
		return;
	}
	TRACELOG_DEBUG("\t\ttest not aligned.");
	//InitFlash_PageErase(0);

	IntFlash_Write(0,intflash_tx_zero,INTFLASH_BUFFER_SIZE);
	IntFlash_Write(0,intflash_tx,INTFLASH_BUFFER_SIZE-1);
	memset(intflash_rx,0,sizeof(intflash_rx));
	IntFlash_Read(0,intflash_rx,INTFLASH_BUFFER_SIZE);
	if(check_Intbuffer(INTFLASH_BUFFER_SIZE-1)&&(0 == intflash_rx[INTFLASH_BUFFER_SIZE-1]))
	{
		TRACELOG_DEBUG("\t\ttest not aligned success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttest not aligned fail.");
		TRACELOG_DEBUG("\ttest_intflash_multipage_rw_check fail.");
		return;
	}

	TRACELOG_DEBUG("\ttest_intflash_multipage_rw_check success.");

}


void testcase_msc_secort()
{
	TRACELOG_DEBUG("\ttestcase_msc_secort.");
	memset(intflash_tx,0x0f,INTFLASH_BUFFER_SIZE);
	IntFlash_MSC_Write(0, INTFLASH_BUFFER_SIZE, intflash_tx);
	IntFlash_MSC_Read(0, INTFLASH_BUFFER_SIZE, intflash_rx);
	if(check_Intbuffer(INTFLASH_BUFFER_SIZE))
	{
		TRACELOG_DEBUG("\t\ttestcase_msc_secort success.");
	}
	else
	{
		TRACELOG_DEBUG("\t\ttestcase_msc_secort fail.");
	}
		
	TRACELOG_DEBUG("\ttestcase_msc_secort end.");
}

void test_easerintflash()
{
	TRACELOG_DEBUG("\tRunning test_easerintflash");
	uint32_t i;
	uint8_t data[4*1024];
	memset(data,0xff,4096);
	for(i=0;i<INTFLASH_MEM_SIZE/4096;i++)
	{
	
			TRACELOG_DEBUG_PRINTF("Easer [%d].",i);
			IntFlash_Write(CONFIG_FLAGBIT_ADDR+i*4096, data, 4096);

	}
	for(i=0;i<INTFLASH_MEM_SIZE/4;i++)
	{
		uint32_t rddata;
		TRACELOG_DEBUG_PRINTF("check [%d].",i);
		IntFlash_Read(CONFIG_FLAGBIT_ADDR+i*4, (uint8_t*)&rddata, 4);
		if(0xffffffff != rddata)
		{
			TRACELOG_DEBUG_PRINTF("Easer [%d-%d] error.",i,rddata);
			return;
		}
	}
	TRACELOG_DEBUG("\ttest_easerintflash success");
}

void testcase_intflash()
{
	TRACELOG_DEBUG("Running intflash testcase.");
	
	memset(intflash_tx,TEST_INTFLASH_CHAR,INTFLASH_BUFFER_SIZE);
	memset(intflash_tx_zero,0,INTFLASH_BUFFER_SIZE);

	IntFlash_Init();

	//test_intflash_page_rw_nocheck();
	//test_intflash_page_rw_check();
	//test_intflash_multipage_rw_nocheck();
	//test_intflash_multipage_rw_check();
	#ifdef TEST_USB_MSC
	//testcase_msc_secort();
	#endif

	test_easerintflash();
	TRACELOG_DEBUG("Intflash testcase end.");
}

#endif







