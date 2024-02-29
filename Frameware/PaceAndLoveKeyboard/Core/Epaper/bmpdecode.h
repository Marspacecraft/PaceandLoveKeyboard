
#ifndef _BMPDECODE_H_
#define _BMPDECODE_H_
#include "main.h"

#pragma pack(push)
#pragma pack (1)

typedef struct 
{
	uint16_t	bfType;                  // λͼ�ļ������ͣ�����ΪBMP (2���ֽ�)
    uint32_t	bfSize;                  // λͼ�ļ��Ĵ�С�����ֽ�Ϊ��λ (4���ֽ�)
    uint16_t	bfReserved1;             // λͼ�ļ������֣�����Ϊ0 (2���ֽ�)
    uint16_t	bfReserved2;             // λͼ�ļ������֣�����Ϊ0 (2���ֽ�)
    uint32_t	bfOffBits;               // λͼ���ݵ���ʼλ�ã��������λͼ (4���ֽ�)
} bmpfilehead_t;

typedef struct 
{
	uint32_t 	biSize;     		// ���ṹ��ռ���ֽ���  (4���ֽ�)
	uint32_t 	biWidth;      		// λͼ�Ŀ�ȣ�������Ϊ��λ(4���ֽ�)
	uint32_t 	biHeight;     		// λͼ�ĸ߶ȣ�������Ϊ��λ(4���ֽ�)
	uint16_t 	biPlanes;    		// Ŀ���豸�ļ��𣬱���Ϊ1(2���ֽ�)
	uint16_t 	biBitCount; 		// ÿ�����������λ����������1(˫ɫ)��4(16ɫ)��8(256ɫ)��24(���ɫ)��32(��ǿ���ɫ)֮һ (2���ֽ�)
	uint32_t	biCompression; 		// λͼѹ�����ͣ������� 0(��ѹ��)�� 1(BI_RLE8ѹ������)��2(BI_RLE4ѹ������)֮һ ) (4���ֽ�)
	uint32_t 	biSizeImage;     	// λͼ�Ĵ�С�����ֽ�Ϊ��λ(4���ֽ�)
	uint32_t 	biXPelsPerMeter;  	// λͼˮƽ�ֱ��ʣ�ÿ��������(4���ֽ�)
	uint32_t 	biYPelsPerMeter;	// λͼ��ֱ�ֱ��ʣ�ÿ��������(4���ֽ�)
	uint32_t 	biClrUsed;        	// λͼʵ��ʹ�õ���ɫ���е���ɫ��(4���ֽ�)
	uint32_t 	biClrImportant;   	// λͼ��ʾ��������Ҫ����ɫ��(4���ֽ�)
} bmphead_t;

typedef struct  
{
	uint8_t rgbBlue;          // ��ɫ������(ֵ��ΧΪ0-255)
	uint8_t rgbGreen;         // ��ɫ������(ֵ��ΧΪ0-255)
	uint8_t rgbRed;           // ��ɫ������(ֵ��ΧΪ0-255)
	uint8_t rgbReserved;      // ����������Ϊ0
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



