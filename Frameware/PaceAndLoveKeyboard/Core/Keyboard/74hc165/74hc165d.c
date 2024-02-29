

#include "main.h"
#include "spi.h"
#include "keyconfig.h"
#include "tracelog.h"

extern SPI_HandleTypeDef hspi1;

void KEY_74HC165D_Scan(uint8_t *buffer,uint8_t len)
{
	uint8_t i;
	HAL_GPIO_WritePin(KEY_PL_GPIO_Port, KEY_PL_Pin, GPIO_PIN_SET);
	hspi1.pRxBuffPtr = buffer;
    hspi1.RxXferCount = len;
	__HAL_SPI_ENABLE(&hspi1);
	//HAL_Delay(1);
 	if (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_RXNE))
    {
        (*(uint8_t*) hspi1.pRxBuffPtr) = *(__IO uint8_t*) &hspi1.Instance->DR;
    }
	
	while (hspi1.RxXferCount > 0U)
    {
        if (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_RXNE))
        {
            /* read the received data */
            (*(uint8_t*) hspi1.pRxBuffPtr) = *(__IO uint8_t*) &hspi1.Instance->DR;
            hspi1.pRxBuffPtr += sizeof(uint8_t);
            hspi1.RxXferCount--;
        }
		
    }
	
	__HAL_SPI_DISABLE(&hspi1);
	HAL_GPIO_WritePin(KEY_PL_GPIO_Port, KEY_PL_Pin, GPIO_PIN_RESET);

	for(i=1;i<len;i++)
	{
		if(i%2)
		{
			buffer[i] = ~(buffer[i]);
		}
	}
    return ;

}


#ifdef TEST_74HC165
#include "gpio.h"

void testcase_74hc165()
{

	TRACELOG_DEBUG("Running testcase_74HC165!...");
	uint32_t value[4] = {0};
	uint8_t* v = (uint8_t*)value;
		
	while(1)
	{
		KEY_74HC165D_Scan((uint8_t*)value,14);
		TRACELOG_DEBUG_PRINTF("VALUE[%02x%02x%02x%02x][%02x%02x%02x%02x][%02x%02x%02x%02x][%02x%02x%02x%02x]!"
			,v[0],v[1],v[2],v[3]
			,v[4],v[5],v[6],v[7]
			,v[8],v[9],v[10],v[11]
			,v[12],v[13],v[14],v[15]);
		extern void testcase_led_toggle(void);
		testcase_led_toggle();
		HAL_Delay(1000);
	}
	TRACELOG_DEBUG("testcase_74HC165 end");
}

#endif


