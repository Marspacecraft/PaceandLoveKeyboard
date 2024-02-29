
#ifndef _BMPDECODE_H_
#define _BMPDECODE_H_
#include "main.h"

#pragma pack(push)
#pragma pack (1)

typedef struct 
{
	uint16_t	bfType;                  // 位图文件的类型，必须为BMP (2个字节)
    uint32_t	bfSize;                  // 位图文件的大小，以字节为单位 (4个字节)
    uint16_t	bfReserved1;             // 位图文件保留字，必须为0 (2个字节)
    uint16_t	bfReserved2;             // 位图文件保留字，必须为0 (2个字节)
    uint32_t	bfOffBits;               // 位图数据的起始位置，以相对于位图 (4个字节)
} bmpfilehead_t;

typedef struct 
{
	uint32_t 	biSize;     		// 本结构所占用字节数  (4个字节)
	uint32_t 	biWidth;      		// 位图的宽度，以像素为单位(4个字节)
	uint32_t 	biHeight;     		// 位图的高度，以像素为单位(4个字节)
	uint16_t 	biPlanes;    		// 目标设备的级别，必须为1(2个字节)
	uint16_t 	biBitCount; 		// 每个像素所需的位数，必须是1(双色)、4(16色)、8(256色)、24(真彩色)或32(增强真彩色)之一 (2个字节)
	uint32_t	biCompression; 		// 位图压缩类型，必须是 0(不压缩)、 1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一 ) (4个字节)
	uint32_t 	biSizeImage;     	// 位图的大小，以字节为单位(4个字节)
	uint32_t 	biXPelsPerMeter;  	// 位图水平分辨率，每米像素数(4个字节)
	uint32_t 	biYPelsPerMeter;	// 位图垂直分辨率，每米像素数(4个字节)
	uint32_t 	biClrUsed;        	// 位图实际使用的颜色表中的颜色数(4个字节)
	uint32_t 	biClrImportant;   	// 位图显示过程中重要的颜色数(4个字节)
} bmphead_t;

typedef struct  
{
	uint8_t rgbBlue;          // 蓝色的亮度(值范围为0-255)
	uint8_t rgbGreen;         // 绿色的亮度(值范围为0-255)
	uint8_t rgbRed;           // 红色的亮度(值范围为0-255)
	uint8_t rgbReserved;      // 保留，必须为0
} bmpcolor_t;

typedef struct
{
	bmpfilehead_t 	fhead;
	bmphead_t 		ihead;
	bmpcolor_t		rgbquad[2];
}bmp_t;
#pragma pack (pop)

#define BMP_WIDTH 	264
#define BMP_HEIGHT  176
#define BMP_DATA_BUFFER_SIZE   	(BMP_WIDTH*BMP_HEIGHT/8)
#define BMP_DATA_OFFSET 		sizeof(bmp_t)
#define BMP_FILE_SIZE 			(BMP_DATA_OFFSET+BMP_DATA_BUFFER_SIZE)

bool bmp_check(uint8_t* buffer);
uint32_t bmp_width(uint8_t* buffer);
uint32_t bmp_height(uint8_t* buffer);

#endif



