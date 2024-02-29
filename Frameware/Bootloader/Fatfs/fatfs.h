#ifndef _FATFS_H_
#define _FATFS_H_

/************************************************************************

mscĿ¼�ṹ
	-boot  	bootloader�ļ����Ŀ¼
	-bmp	īˮ��ͼƬĿ¼
	-pic	oled��ͼƬĿ¼
	-bank	�ֿ�Ŀ¼
	-log	��־Ŀ¼

flashĿ¼�ṹ
	-boot	bootloader�ļ����Ŀ¼
	-bmp	īˮ��ͼƬĿ¼
	-pic	oled��ͼƬĿ¼
	-bank	�ֿ�Ŀ¼
	-log	��־Ŀ¼

************************************************************************/




void Flash_Mount(void);
void Flash_Unmount(void);

uint8_t IntFlash_Mount(void);
void IntFlash_Unmount(void);


#ifdef TEST_FATFS
	void testcase_fatfs(void);
#endif

#endif

