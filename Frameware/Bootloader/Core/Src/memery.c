
#include "memery.h"
#include "tracelog.h"

uint32_t flash_read_word(uint32_t raddr)
{
    uint32_t word;
    word = *(volatile uint32_t *)raddr;
	return word;
}

void  flash_write_word(uint32_t waddr, uint32_t word)
{
	HAL_FLASH_Unlock();   
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, waddr,(uint64_t)word);
	HAL_FLASH_Lock(); 
}

void  flash_erase(uint32_t eaddr)
{
	uint32_t erase_addr; 
	FLASH_EraseInitTypeDef flash_eraseop;
	uint32_t offaddr =  eaddr - STM32_FLASH_START; 
   	uint32_t secpos = offaddr / STM32_SECTOR_SIZE;
	
	HAL_FLASH_Unlock(); 
	flash_eraseop.TypeErase = FLASH_TYPEERASE_PAGES;       
	flash_eraseop.Banks = FLASH_BANK_1;
	flash_eraseop.NbPages = 1;
	flash_eraseop.PageAddress = secpos * STM32_SECTOR_SIZE + STM32_FLASH_START;  
	HAL_FLASHEx_Erase( &flash_eraseop, &erase_addr);
	HAL_FLASH_Lock();
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
        pbuf[i] = stmflash_read_halfword(raddr);    /* ?����?2??��??�� */
        raddr += 2; /* ??��?2??��??�� */
    }
}
void IntFlash_Read(uint32_t raddr, uint8_t *pbuf, uint16_t length)
{
	stmflash_read(raddr,(uint16_t*)pbuf,(length)>>1);
	if(length & 0x0001)
	{
		pbuf[length-1] = (uint8_t)(stmflash_read_halfword(raddr + length-1)&0xff00);
	}
}

HAL_StatusTypeDef InitFlash_PageErase(uint32_t eindex,uint32_t num)
{
	uint32_t SECTORError = 0;
	HAL_StatusTypeDef ret;
	static FLASH_EraseInitTypeDef EraseInitStruct;
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
        waddr += 2; /* ???��??��???��?��? */
    }
	HAL_FLASH_Lock(); /* ��??? */

}

void IntFlash_Write_NoCheck(uint32_t waddr, uint8_t *pbuf, uint16_t length)
{
	uint16_t value;
	
	stmflash_write_nocheck(waddr,(uint16_t*)pbuf,(length)>>1);
	if(length & 0x0001)
	{
		value = stmflash_read_halfword(waddr + length-1);		
		value = (value&~0xff00)|((uint16_t)pbuf[length - 1]);
		stmflash_write_nocheck(waddr + length-1 , &value,1);
	}

}



void stmflash_write(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
	uint16_t g_intflashbuf[STM32_SECTOR_SIZE/2]; /* ��??����?2K��??�� */
    uint32_t secpos;    /* ����??��??�� */
    uint16_t secoff;    /* ����???��??��?��??��(16??��?????) */
    uint16_t secremain; /* ����???�����ꨮ����??��(16??��?????) */
    uint16_t i;
    uint32_t offaddr;   /* ������?0X08000000o����?��??�� */
    FLASH_EraseInitTypeDef flash_eraseop;
    uint32_t erase_addr;   /* 2��3y�䨪?����??a???��?a���騦���䨪?����?����??��??�� */

    if ( (waddr + length * 2) >= (INTFLASH_ADDR_END - INTFLASH_ADDR_START))
    {
        return; /* ��?������??�� */
    }

    offaddr = waddr;       /* ����?��??��?��??��. */
    secpos = offaddr / STM32_SECTOR_SIZE;       /* ����??��??��  0~127 for STM32F103RBT6 */
    secoff = (offaddr % STM32_SECTOR_SIZE) / 2; /* ?������???����???��?(2??��??��?a?����?�̣�??.) */
    secremain = STM32_SECTOR_SIZE / 2 - secoff; /* ����??���ꨮ��????�䨮D? */
    if (length <= secremain)
    {
        secremain = length; /* 2?�䨮����??����??��??�� */
    }

    while (1)
    {
        stmflash_read(secpos * STM32_SECTOR_SIZE, g_intflashbuf, STM32_SECTOR_SIZE / 2); /* ?��3?????����??��??����Y */
        for (i = 0; i < secremain; i++)                                                              /* D��?����y?Y */
        {
            if (g_intflashbuf[secoff + i] != 0XFFFF)
            {
                break; /* D����a2��3y */
            }
        }
        if (i < secremain) /* D����a2��3y */
        { 
        	HAL_FLASH_Unlock(); 
            flash_eraseop.TypeErase = FLASH_TYPEERASE_PAGES;        /* ??????2��3y */
            flash_eraseop.Banks = FLASH_BANK_1;
            flash_eraseop.NbPages = 1;
            flash_eraseop.PageAddress = secpos * STM32_SECTOR_SIZE + INTFLASH_ADDR_START;  /* ��a2��3y��?����?? */
            HAL_FLASHEx_Erase( &flash_eraseop, &erase_addr);
			HAL_FLASH_Lock(); 
			
            for (i = 0; i < secremain; i++)                               /* ?��?? */
            {
                g_intflashbuf[i + secoff] = pbuf[i];
            }
            stmflash_write_nocheck(secpos * STM32_SECTOR_SIZE, g_intflashbuf, STM32_SECTOR_SIZE / 2); /* D�䨨?????����?? */
        }
        else
        {
            stmflash_write_nocheck(waddr, pbuf, secremain); /* D�䨰??-2��3y��?��?,?��?��D�䨨?����??���ꨮ��????. */
        }
        if (length == secremain)
        {
            break; /* D�䨨??����?��? */
        }
        else       /* D�䨨??��?����? */
        {
            secpos++;               /* ����??��??��??1 */
            secoff = 0;             /* ??��??????a0 */
            pbuf += secremain;      /* ??????��? */
            waddr += secremain * 2; /* D���??��??��?(16??��y?Y��??��,D����a*2) */
            length -= secremain;    /* ��??��(16??)��y��Y?? */
            if (length > (STM32_SECTOR_SIZE / 2))
            {
                secremain = STM32_SECTOR_SIZE / 2; /* ??��???����???1��?D��2?���� */
            }
            else
            {
                secremain = length; /* ??��???����???����?D�䨪����? */
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
		value = (value&~0xff00)|((uint16_t)pbuf[length - 1]);
		stmflash_write(waddr + length-1,&value,1);		
	}
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



