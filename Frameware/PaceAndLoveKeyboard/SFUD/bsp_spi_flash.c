
#include "flash.h"
//��Ҫ������Ӵ���
#include "stm32f1xx_hal_gpio.h"
#include "spi.h"
#include "keyconfig.h"
#include "scheduler.h"

//#include "board.h"
uint16_t W25QXX_TYPE=W25Q128;	//Ĭ����W25Q256
extern SPI_HandleTypeDef hspi3;


uint8_t W25QXX_ReadSR(uint8_t regno);             //??????????
void W25QXX_4ByteAddr_Enable(void);     //???4???????
void W25QXX_Write_SR(uint8_t regno,uint8_t sr);   //?????????
void W25QXX_Write_Enable(void);  		//?????
void W25QXX_Write_Disable(void);		//??????
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);   //???flash
void W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);//????flash
void W25QXX_Erase_Chip(void);    	  	//???????
void W25QXX_Erase_Sector(uint32_t Dst_Addr);	//????????
void W25QXX_Wait_Busy(void);           	//???????
void W25QXX_PowerDown(void);        	//?????????


void Flash_Init(void)
{
   /*##-1- Configure the SPI peripheral #######################################*/
  /* Set the SPI parameters */
  FLASH_SPI_HANDLER.Instance               = FLASH_SPIX;
  FLASH_SPI_HANDLER.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  FLASH_SPI_HANDLER.Init.Direction         = SPI_DIRECTION_2LINES;
  FLASH_SPI_HANDLER.Init.CLKPhase          = SPI_PHASE_2EDGE;
  FLASH_SPI_HANDLER.Init.CLKPolarity       = SPI_POLARITY_HIGH;
  FLASH_SPI_HANDLER.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  FLASH_SPI_HANDLER.Init.CRCPolynomial     = 7;
  FLASH_SPI_HANDLER.Init.DataSize          = SPI_DATASIZE_8BIT;
  FLASH_SPI_HANDLER.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  FLASH_SPI_HANDLER.Init.NSS               = SPI_NSS_SOFT;
  FLASH_SPI_HANDLER.Init.TIMode            = SPI_TIMODE_DISABLE;
  
  FLASH_SPI_HANDLER.Init.Mode = SPI_MODE_MASTER;

  HAL_SPI_Init(&FLASH_SPI_HANDLER); 
  
  __HAL_SPI_ENABLE(&FLASH_SPI_HANDLER);     
}

uint8_t Flash_SPI_Valid()
{

	return true;
}


//SPI1 ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
uint8_t SPI2_ReadWriteByte(uint8_t TxData)
{
    uint8_t Rxdata;
    HAL_SPI_TransmitReceive(&FLASH_SPI_HANDLER,&TxData,&Rxdata,1, 1000);
    return Rxdata;          		    //�����յ�������
}
#define FLASH_USE_DMA 1
HAL_StatusTypeDef SPI2_Read(uint8_t* pBuffer,uint16_t NumByteToRead)
{
#if FLASH_USE_DMA
	HAL_StatusTypeDef ret = HAL_SPI_Receive_DMA(&FLASH_SPI_HANDLER,pBuffer,NumByteToRead);
	while(HAL_SPI_GetState(&FLASH_SPI_HANDLER) == HAL_SPI_STATE_BUSY_RX)
	{
		do_ms_schedule();
	};
	return ret;
#else
	return HAL_SPI_Receive(&FLASH_SPI_HANDLER,pBuffer,NumByteToRead,1000);        		    //�����յ�������
#endif
}

HAL_StatusTypeDef SPI2_Write(uint8_t* pBuffer,uint16_t NumByteToWrite)
{
#if FLASH_USE_DMA

	HAL_StatusTypeDef ret = HAL_SPI_Transmit_DMA(&FLASH_SPI_HANDLER,pBuffer,NumByteToWrite);
	while(HAL_SPI_GetState(&FLASH_SPI_HANDLER) == HAL_SPI_STATE_BUSY_TX)
	{
		do_ms_schedule();
	};
	return ret;
#else
	return HAL_SPI_Transmit(&FLASH_SPI_HANDLER,pBuffer,NumByteToWrite,1000);        		    //�����յ�������
#endif
}

void SPI2_SetSpeed(uint8_t SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//�ж���Ч��
    __HAL_SPI_DISABLE(&FLASH_SPI_HANDLER);            //�ر�SPI
    FLASH_SPI_HANDLER.Instance->CR1&=0XFFC7;          //λ3-5���㣬�������ò�����
    FLASH_SPI_HANDLER.Instance->CR1|=SPI_BaudRatePrescaler;//����SPI�ٶ�
    __HAL_SPI_ENABLE(&FLASH_SPI_HANDLER);             //ʹ��SPI

}

/**
 * @brief  �ȴ�WIP(BUSY)��־����0�����ȴ���FLASH�ڲ�����д�����
 * @param  none
 * @retval none
 */
void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;

    /* ѡ�� FLASH: CS �� */
    W25QXX_CS_0;

    /* ���� ��״̬�Ĵ��� ���� */
    SPI2_ReadWriteByte(W25X_ReadStatusReg1);

    /* ��FLASHæµ����ȴ� */
    do
    {
        /* ��ȡFLASHоƬ��״̬�Ĵ��� */
        FLASH_Status = SPI2_ReadWriteByte(0xFF);
    }
    while ((FLASH_Status & 0x01) == SET);  /* ����д���־ */

    /* ֹͣ�ź�  FLASH: CS �� */
    W25QXX_CS_1;
}


//��ȡW25QXX��״̬�Ĵ�����W25QXXһ����3��״̬�Ĵ���
//״̬�Ĵ���1��
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
//״̬�Ĵ���2��
//BIT7  6   5   4   3   2   1   0
//SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
//״̬�Ĵ���3��
//BIT7      6    5    4   3   2   1   0
//HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
//regno:״̬�Ĵ����ţ���:1~3
//����ֵ:״̬�Ĵ���ֵ
uint8_t W25QXX_ReadSR(uint8_t regno)
{
    uint8_t byte=0,command=0;
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;    //��״̬�Ĵ���1ָ��
            break;
        case 2:
            command=W25X_ReadStatusReg2;    //��״̬�Ĵ���2ָ��
            break;
        case 3:
            command=W25X_ReadStatusReg3;    //��״̬�Ĵ���3ָ��
            break;
        default:
            command=W25X_ReadStatusReg1;
            break;
    }
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(command);            //���Ͷ�ȡ״̬�Ĵ�������
    byte=SPI2_ReadWriteByte(0Xff);          //��ȡһ���ֽ�
    W25QXX_CS_1;                            //ȡ��Ƭѡ
    return byte;
}
//дW25QXX״̬�Ĵ���
void W25QXX_Write_SR(uint8_t regno,uint8_t sr)
{
    uint8_t command=0;
    switch(regno)
    {
        case 1:
            command=W25X_WriteStatusReg1;    //д״̬�Ĵ���1ָ��
            break;
        case 2:
            command=W25X_WriteStatusReg2;    //д״̬�Ĵ���2ָ��
            break;
        case 3:
            command=W25X_WriteStatusReg3;    //д״̬�Ĵ���3ָ��
            break;
        default:
            command=W25X_WriteStatusReg1;
            break;
    }
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(command);            //����дȡ״̬�Ĵ�������
    SPI2_ReadWriteByte(sr);                 //д��һ���ֽ�
    W25QXX_CS_1;                            //ȡ��Ƭѡ
}
//W25QXXдʹ��
//��WEL��λ
void W25QXX_Write_Enable(void)
{
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(W25X_WriteEnable);   //����дʹ��
    W25QXX_CS_1;                            //ȡ��Ƭѡ
}
//W25QXXд��ֹ
//��WEL����
void W25QXX_Write_Disable(void)
{
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(W25X_WriteDisable);  //����д��ָֹ��
    W25QXX_CS_1;                            //ȡ��Ƭѡ
}

//��ȡоƬID
//����ֵ����:
//0XEF13,��ʾоƬ�ͺ�ΪW25Q80
//0XEF14,��ʾоƬ�ͺ�ΪW25Q16
//0XEF15,��ʾоƬ�ͺ�ΪW25Q32
//0XEF16,��ʾоƬ�ͺ�ΪW25Q64
//0XEF17,��ʾоƬ�ͺ�ΪW25Q128
//0XEF18,��ʾоƬ�ͺ�ΪW25Q256
uint16_t W25QXX_ReadID(void)
{
    uint16_t Temp = 0;
    W25QXX_CS_0;
    SPI2_ReadWriteByte(0x90);//���Ͷ�ȡID����
    SPI2_ReadWriteByte(0x00);
    SPI2_ReadWriteByte(0x00);
    SPI2_ReadWriteByte(0x00);
    Temp|=SPI2_ReadWriteByte(0xFF)<<8;
    Temp|=SPI2_ReadWriteByte(0xFF);
    W25QXX_CS_1;
    return Temp;
}
//��ȡSPI FLASH
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)
{
//    uint16_t i;
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(W25X_ReadData);      //���Ͷ�ȡ����
//    if(W25QXX_TYPE==W25Q64)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
//    {
//        SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>24));
//    }
    SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>16));   //����24bit��ַ
    SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    SPI2_ReadWriteByte((uint8_t)ReadAddr);
    //for(i=0;i<NumByteToRead;i++)
    //{
     //   pBuffer[i]=SPI2_ReadWriteByte(0XFF);    //ѭ������
   // }
	SPI2_Read(pBuffer,NumByteToRead);
    W25QXX_CS_1;
}

//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!
void W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
//    uint16_t i;
    W25QXX_Write_Enable();                  //SET WEL
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(W25X_PageProgram);   //����дҳ����
//    if(W25QXX_TYPE==W25Q64)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
//    {
//        SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>24));
//    }
    SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>16)); //����24bit��ַ
    SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>8));
    SPI2_ReadWriteByte((uint8_t)WriteAddr);
    //for(i=0;i<NumByteToWrite;i++)SPI2_ReadWriteByte(pBuffer[i]);//ѭ��д��
    SPI2_Write(pBuffer,NumByteToWrite);
    W25QXX_CS_1;                            //ȡ��Ƭѡ
    SPI_FLASH_WaitForWriteEnd();					   //�ȴ�д�����
}



//�޼���дSPI FLASH
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ����
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
    while(1)
    {
        W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
        if(NumByteToWrite==pageremain)break;//д�������
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;

            NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
            if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
            else pageremain=NumByteToWrite; 	  //����256���ֽ���
        }
    };
}
//дSPI FLASH
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
uint8_t W25QXX_BUFFER[4096];
void W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t * W25QXX_BUF;
    W25QXX_BUF=W25QXX_BUFFER;
    secpos=WriteAddr/4096;//������ַ
    secoff=WriteAddr%4096;//�������ڵ�ƫ��
    secremain=4096-secoff;//����ʣ��ռ��С
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
    if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
    while(1)
    {
        W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//������������������
        for(i=0;i<secremain;i++)//У������
        {
            if(W25QXX_BUF[secoff+i]!=0XFF)break;//��Ҫ����
        }
        if(i<secremain)//��Ҫ����
        {
            W25QXX_Erase_Sector(secpos);//�����������
            for(i=0;i<secremain;i++)	   //����
            {
                W25QXX_BUF[i+secoff]=pBuffer[i];
            }
            W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//д����������

        }else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������.
        if(NumByteToWrite==secremain)break;//д�������
        else//д��δ����
        {
            secpos++;//������ַ��1
            secoff=0;//ƫ��λ��Ϊ0

            pBuffer+=secremain;  //ָ��ƫ��
            WriteAddr+=secremain;//д��ַƫ��
            NumByteToWrite-=secremain;				//�ֽ����ݼ�
            if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
            else secremain=NumByteToWrite;			//��һ����������д����
        }
    };
}
//��������оƬ
//�ȴ�ʱ�䳬��...
void W25QXX_Erase_Chip(void)
{
    W25QXX_Write_Enable();                  //SET WEL
    W25QXX_Wait_Busy();
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(W25X_ChipErase);        //����Ƭ��������
    W25QXX_CS_1;                            //ȡ��Ƭѡ
    W25QXX_Wait_Busy();   				   //�ȴ�оƬ��������
}
//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ������������ʱ��:150ms
void W25QXX_Erase_Sector(uint32_t Dst_Addr)
{
    //����falsh�������,������
    //printf("fe:%x\r\n",Dst_Addr);
    Dst_Addr*=4096;
    W25QXX_Write_Enable();                  //SET WEL
    W25QXX_Wait_Busy();
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(W25X_SectorErase);   //������������ָ��
    SPI2_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //����24bit��ַ
    SPI2_ReadWriteByte((uint8_t)((Dst_Addr)>>8));
    SPI2_ReadWriteByte((uint8_t)Dst_Addr);
    W25QXX_CS_1;                            //ȡ��Ƭѡ
    W25QXX_Wait_Busy();   				    //�ȴ��������
}

//�ȴ�����
void W25QXX_Wait_Busy(void)
{
    while((W25QXX_ReadSR(1)&0x01)==0x01);   // �ȴ�BUSYλ���
}
//�������ģʽ
void W25QXX_PowerDown(void)
{
    W25QXX_CS_0;                            //ʹ������
    SPI2_ReadWriteByte(W25X_PowerDown);     //���͵�������
    W25QXX_CS_1;                            //ȡ��Ƭѡ
    //delay_us(3);                            //�ȴ�TPD
}
//����
void W25QXX_WAKEUP(void)
{
    W25QXX_CS_0;                                //ʹ������
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);  //  send W25X_PowerDown command 0xAB
    W25QXX_CS_1;                                //ȡ��Ƭѡ
    //delay_us(3);                                //�ȴ�TRES1
}




#ifdef TEST_NORFLASH
#include <string.h>
#include "tracelog.h"
uint8_t testbuffer[] = {"Hello World! I'm Marspacecraft."};
uint8_t testbufferrx[] = {"Hello World! I'm Marspacecraft."};
uint8_t testspeedbuf[1024*16] = {0};
extern uint16_t TIM_GetCounnt_MS(void);
void testcase_testspeed()
{
	uint16_t bef = TIM_GetCounnt_MS();
	uint16_t aft = TIM_GetCounnt_MS();
	TRACELOG_DEBUG("\ttestcase_testspeed...");

	Flash_Write(0,sizeof(testspeedbuf),testspeedbuf);
	bef = TIM_GetCounnt_MS();
	Flash_EraseSecotor(0);
	Flash_EraseSecotor(1);
	Flash_EraseSecotor(2);
	Flash_EraseSecotor(3);
	aft = TIM_GetCounnt_MS();
	TRACELOG_DEBUG_PRINTF("\t\tErase speed [%dms/16KB]",aft-bef);


	bef = TIM_GetCounnt_MS();
	Flash_Write(0,sizeof(testspeedbuf),testspeedbuf);
	aft = TIM_GetCounnt_MS();
	TRACELOG_DEBUG_PRINTF("\t\tWrite speed [%dms/16KB]",aft-bef);
	

	bef = TIM_GetCounnt_MS();
	Flash_Read(0,sizeof(testspeedbuf),testspeedbuf);
	aft = TIM_GetCounnt_MS();
	TRACELOG_DEBUG_PRINTF("\t\tRead speed [%dms/16KB]",aft-bef);
	
	memset(testspeedbuf,0x0f,sizeof(testspeedbuf));
	bef = TIM_GetCounnt_MS();
	Flash_Write_Erz(0,sizeof(testspeedbuf),testspeedbuf);
	aft = TIM_GetCounnt_MS();
	TRACELOG_DEBUG_PRINTF("\t\tWrite erz speed [%dms/16KB]",aft-bef);
	
	
	TRACELOG_DEBUG("\ttestcase_testspeed end...");
}


void testcase_rw()
{
	TRACELOG_DEBUG("\ttestcase_rw...");
	
	Flash_EraseSecotor(0);
	
	memset(testbufferrx,0,sizeof(testbufferrx));
	Flash_Read(0,sizeof(testbufferrx),testbufferrx);
	testbufferrx[sizeof(testbufferrx)-1] = 0;
	TRACELOG_DEBUG_PRINTF("\t\tErase get string from flash:%s",testbufferrx);

	
	Flash_Write(0,sizeof(testbuffer),testbuffer);
	
	memset(testbufferrx,0,sizeof(testbufferrx));
	Flash_Read(0,sizeof(testbufferrx),testbufferrx);
	testbufferrx[sizeof(testbufferrx)-1] = 0;
	TRACELOG_DEBUG_PRINTF("\t\tWrite get string from flash:%s",testbufferrx);


	memset(testbufferrx,0,sizeof(testbufferrx));
	Flash_Write_Erz(0,sizeof(testbufferrx),testbufferrx);
	testbuffer[sizeof(testbuffer)-2] = 0;
	Flash_Write_Erz(0,sizeof(testbuffer),testbuffer);
	
	memset(testbufferrx,0,sizeof(testbufferrx));
	Flash_Read(0,sizeof(testbufferrx),testbufferrx);
	testbufferrx[sizeof(testbufferrx)-1] = 0;
	TRACELOG_DEBUG_PRINTF("\t\tWrite erz get string from flash:%s",testbufferrx);
	
	TRACELOG_DEBUG("\ttestcase_rw end...");
}


void testcase_norflash()
{
	TRACELOG_DEBUG("Running testcase_norflash...");
	testcase_rw();

	Flash_Read(0,sizeof(testbufferrx),testbufferrx);
	testbufferrx[sizeof(testbufferrx)-1] = 0;
	
	TRACELOG_DEBUG_PRINTF("\t\t-------------------Erase get string from flash:%s",testbufferrx);
	testcase_testspeed();

	TRACELOG_DEBUG("testcase_norflash end...");
}


#endif


