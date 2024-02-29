
#include "bmpdecode.h"




static bool bmp_check_filehead(bmpfilehead_t* head)
{
	if(0x4d42 != head->bfType)
		return false;

	if(BMP_FILE_SIZE < head->bfSize)
		return false;
	
	if(head->bfOffBits != (sizeof(bmp_t)-1))
			return false;
	
	return true;
}

static bool bmp_check_infohead(bmphead_t* head)
{
	uint32_t max,min;
	if(head->biSize != sizeof(bmphead_t))
		return false;

	if(head->biWidth > head->biHeight)
	{
		max = head->biWidth;
		min = head->biHeight;
	}
	else
	{
		min = head->biWidth;
		max = head->biHeight;
	}
	
	if(max > BMP_WIDTH)
		return false;

	if(min > BMP_HEIGHT)
		return false;

	if(head->biCompression)
		return false;

	if(1 != head->biBitCount)
		return false;

	return true;
}

bool bmp_check(uint8_t* buffer)
{
	bmp_t* bmp = (bmp_t*)buffer;
	if(bmp_check_filehead(&bmp->fhead))
		return false;
	if(bmp_check_infohead(&bmp->ihead))
		return false;
	return true;
}

uint32_t bmp_width(uint8_t* buffer)
{
	bmp_t* bmp = (bmp_t*)buffer;

	return bmp->ihead.biWidth;
}

uint32_t bmp_height(uint8_t* buffer)
{
	bmp_t* bmp = (bmp_t*)buffer;

	return bmp->ihead.biHeight;
}





