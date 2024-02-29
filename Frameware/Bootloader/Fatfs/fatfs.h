#ifndef _FATFS_H_
#define _FATFS_H_

/************************************************************************

msc目录结构
	-boot  	bootloader文件存放目录
	-bmp	墨水屏图片目录
	-pic	oled屏图片目录
	-bank	字库目录
	-log	日志目录

flash目录结构
	-boot	bootloader文件存放目录
	-bmp	墨水屏图片目录
	-pic	oled屏图片目录
	-bank	字库目录
	-log	日志目录

************************************************************************/




void Flash_Mount(void);
void Flash_Unmount(void);

uint8_t IntFlash_Mount(void);
void IntFlash_Unmount(void);


#ifdef TEST_FATFS
	void testcase_fatfs(void);
#endif

#endif

