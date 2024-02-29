
#include "flash.h"
//需要重新添加代码
#include "stm32f1xx_hal_gpio.h"
#include "spi.h"
#include "keyconfig.h"
#include "scheduler.h"

//#include "board.h"
uint16_t W25QXX_TYPE=W25Q128;	//默认是W25Q256
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


//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
uint8_t SPI2_ReadWriteByte(uint8_t TxData)
{
    uint8_t Rxdata;
    HAL_SPI_TransmitReceive(&FLASH_SPI_HANDLER,&TxData,&Rxdata,1, 1000);
    return Rxdata;          		    //返回收到的数据
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
	return HAL_SPI_Receive(&FLASH_SPI_HANDLER,pBuffer,NumByteToRead,1000);        		    //返回收到的数据
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
	return HAL_SPI_Transmit(&FLASH_SPI_HANDLER,pBuffer,NumByteToWrite,1000);        		    //返回收到的数据
#endif
}

void SPI2_SetSpeed(uint8_t SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    __HAL_SPI_DISABLE(&FLASH_SPI_HANDLER);            //关闭SPI
    FLASH_SPI_HANDLER.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    FLASH_SPI_HANDLER.Instance->CR1|=SPI_BaudRatePrescaler;//设置SPI速度
    __HAL_SPI_ENABLE(&FLASH_SPI_HANDLER);             //使能SPI

}

/**
 * @brief  等待WIP(BUSY)标志被置0，即等待到FLASH内部数据写入完毕
 * @param  none
 * @retval none
 */
void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;

    /* 选择 FLASH: CS 低 */
    W25QXX_CS_0;

    /* 发送 读状态寄存器 命令 */
    SPI2_ReadWriteByte(W25X_ReadStatusReg1);

    /* 若FLASH忙碌，则等待 */
    do
    {
        /* 读取FLASH芯片的状态寄存器 */
        FLASH_Status = SPI2_ReadWriteByte(0xFF);
    }
    while ((FLASH_Status & 0x01) == SET);  /* 正在写入标志 */

    /* 停止信号  FLASH: CS 高 */
    W25QXX_CS_1;
}


//读取W25QXX的状态寄存器，W25QXX一共有3个状态寄存器
//状态寄存器1：
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
//状态寄存器2：
//BIT7  6   5   4   3   2   1   0
//SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
//状态寄存器3：
//BIT7      6    5    4   3   2   1   0
//HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
//regno:状态寄存器号，范:1~3
//返回值:状态寄存器值
uint8_t W25QXX_ReadSR(uint8_t regno)
{
    uint8_t byte=0,command=0;
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;    //读状态寄存器1指令
            break;
        case 2:
            command=W25X_ReadStatusReg2;    //读状态寄存器2指令
            break;
        case 3:
            command=W25X_ReadStatusReg3;    //读状态寄存器3指令
            break;
        default:
            command=W25X_ReadStatusReg1;
            break;
    }
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(command);            //发送读取状态寄存器命令
    byte=SPI2_ReadWriteByte(0Xff);          //读取一个字节
    W25QXX_CS_1;                            //取消片选
    return byte;
}
//写W25QXX状态寄存器
void W25QXX_Write_SR(uint8_t regno,uint8_t sr)
{
    uint8_t command=0;
    switch(regno)
    {
        case 1:
            command=W25X_WriteStatusReg1;    //写状态寄存器1指令
            break;
        case 2:
            command=W25X_WriteStatusReg2;    //写状态寄存器2指令
            break;
        case 3:
            command=W25X_WriteStatusReg3;    //写状态寄存器3指令
            break;
        default:
            command=W25X_WriteStatusReg1;
            break;
    }
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(command);            //发送写取状态寄存器命令
    SPI2_ReadWriteByte(sr);                 //写入一个字节
    W25QXX_CS_1;                            //取消片选
}
//W25QXX写使能
//将WEL置位
void W25QXX_Write_Enable(void)
{
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(W25X_WriteEnable);   //发送写使能
    W25QXX_CS_1;                            //取消片选
}
//W25QXX写禁止
//将WEL清零
void W25QXX_Write_Disable(void)
{
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(W25X_WriteDisable);  //发送写禁止指令
    W25QXX_CS_1;                            //取消片选
}

//读取芯片ID
//返回值如下:
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16
//0XEF15,表示芯片型号为W25Q32
//0XEF16,表示芯片型号为W25Q64
//0XEF17,表示芯片型号为W25Q128
//0XEF18,表示芯片型号为W25Q256
uint16_t W25QXX_ReadID(void)
{
    uint16_t Temp = 0;
    W25QXX_CS_0;
    SPI2_ReadWriteByte(0x90);//发送读取ID命令
    SPI2_ReadWriteByte(0x00);
    SPI2_ReadWriteByte(0x00);
    SPI2_ReadWriteByte(0x00);
    Temp|=SPI2_ReadWriteByte(0xFF)<<8;
    Temp|=SPI2_ReadWriteByte(0xFF);
    W25QXX_CS_1;
    return Temp;
}
//读取SPI FLASH
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)
{
//    uint16_t i;
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(W25X_ReadData);      //发送读取命令
//    if(W25QXX_TYPE==W25Q64)                //如果是W25Q256的话地址为4字节的，要发送最高8位
//    {
//        SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>24));
//    }
    SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>16));   //发送24bit地址
    SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    SPI2_ReadWriteByte((uint8_t)ReadAddr);
    //for(i=0;i<NumByteToRead;i++)
    //{
     //   pBuffer[i]=SPI2_ReadWriteByte(0XFF);    //循环读数
   // }
	SPI2_Read(pBuffer,NumByteToRead);
    W25QXX_CS_1;
}

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
void W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
//    uint16_t i;
    W25QXX_Write_Enable();                  //SET WEL
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(W25X_PageProgram);   //发送写页命令
//    if(W25QXX_TYPE==W25Q64)                //如果是W25Q256的话地址为4字节的，要发送最高8位
//    {
//        SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>24));
//    }
    SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>16)); //发送24bit地址
    SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>8));
    SPI2_ReadWriteByte((uint8_t)WriteAddr);
    //for(i=0;i<NumByteToWrite;i++)SPI2_ReadWriteByte(pBuffer[i]);//循环写数
    SPI2_Write(pBuffer,NumByteToWrite);
    W25QXX_CS_1;                            //取消片选
    SPI_FLASH_WaitForWriteEnd();					   //等待写入结束
}



//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain=256-WriteAddr%256; //单页剩余的字节数
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
    while(1)
    {
        W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
        if(NumByteToWrite==pageremain)break;//写入结束了
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;

            NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
            if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
            else pageremain=NumByteToWrite; 	  //不够256个字节了
        }
    };
}
//写SPI FLASH
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
uint8_t W25QXX_BUFFER[4096];
void W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t * W25QXX_BUF;
    W25QXX_BUF=W25QXX_BUFFER;
    secpos=WriteAddr/4096;//扇区地址
    secoff=WriteAddr%4096;//在扇区内的偏移
    secremain=4096-secoff;//扇区剩余空间大小
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
    if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
    while(1)
    {
        W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
        for(i=0;i<secremain;i++)//校验数据
        {
            if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除
        }
        if(i<secremain)//需要擦除
        {
            W25QXX_Erase_Sector(secpos);//擦除这个扇区
            for(i=0;i<secremain;i++)	   //复制
            {
                W25QXX_BUF[i+secoff]=pBuffer[i];
            }
            W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区

        }else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间.
        if(NumByteToWrite==secremain)break;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff=0;//偏移位置为0

            pBuffer+=secremain;  //指针偏移
            WriteAddr+=secremain;//写地址偏移
            NumByteToWrite-=secremain;				//字节数递减
            if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
            else secremain=NumByteToWrite;			//下一个扇区可以写完了
        }
    };
}
//擦除整个芯片
//等待时间超长...
void W25QXX_Erase_Chip(void)
{
    W25QXX_Write_Enable();                  //SET WEL
    W25QXX_Wait_Busy();
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(W25X_ChipErase);        //发送片擦除命令
    W25QXX_CS_1;                            //取消片选
    W25QXX_Wait_Busy();   				   //等待芯片擦除结束
}
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个扇区的最少时间:150ms
void W25QXX_Erase_Sector(uint32_t Dst_Addr)
{
    //监视falsh擦除情况,测试用
    //printf("fe:%x\r\n",Dst_Addr);
    Dst_Addr*=4096;
    W25QXX_Write_Enable();                  //SET WEL
    W25QXX_Wait_Busy();
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(W25X_SectorErase);   //发送扇区擦除指令
    SPI2_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //发送24bit地址
    SPI2_ReadWriteByte((uint8_t)((Dst_Addr)>>8));
    SPI2_ReadWriteByte((uint8_t)Dst_Addr);
    W25QXX_CS_1;                            //取消片选
    W25QXX_Wait_Busy();   				    //等待擦除完成
}

//等待空闲
void W25QXX_Wait_Busy(void)
{
    while((W25QXX_ReadSR(1)&0x01)==0x01);   // 等待BUSY位清空
}
//进入掉电模式
void W25QXX_PowerDown(void)
{
    W25QXX_CS_0;                            //使能器件
    SPI2_ReadWriteByte(W25X_PowerDown);     //发送掉电命令
    W25QXX_CS_1;                            //取消片选
    //delay_us(3);                            //等待TPD
}
//唤醒
void W25QXX_WAKEUP(void)
{
    W25QXX_CS_0;                                //使能器件
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);  //  send W25X_PowerDown command 0xAB
    W25QXX_CS_1;                                //取消片选
    //delay_us(3);                                //等待TRES1
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


