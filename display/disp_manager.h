#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include <video_manager.h>

/* 图片的象素数据 */
typedef struct PixelDatas {
	int width;   /* 宽度: 一行有多少个象素 */
	int height;  /* 高度: 一列有多少个象素 */
	int Bpp;     /* 一个象素用多少位来表示 */
	int lineBytes;  /* 一行数据有多少字节 */
	int totalBytes; /* 所有字节数 */ 
	unsigned char *aucPixelDatas;  /* 象素数据存储的地方 */
}T_PixelDatas, *PT_PixelDatas;


/* 显示区域,含该区域的左上角/右下角座标
 * 如果是图标,还含有图标的文件名
 */
typedef struct Layout {
	int topLeftX;
	int topLeftY;
	int botRightX;
	int botRightY;
	char *strIconName;
}T_Layout, *PT_Layout;


/* VideoMem的状态:
 * 空闲/用于预先准备显示内容/用于当前线程
 */
typedef enum {
	VMS_FREE = 0,
	VMS_USED_FOR_PREPARE,
	VMS_USED_FOR_CUR,	
}E_VideoMemState;

/* VideoMem中内存里图片的状态:
 * 空白/正在生成/已经生成
 */
typedef enum {
	PS_BLANK = 0,
	PS_GENERATING,
	PS_GENERATED,	
}E_PicState;


typedef struct VideoMem {
	int iID;                        /* ID值,用于标识不同的页面 */
	int bDevFrameBuffer;            /* 1: 这个VideoMem是显示设备的显存; 0: 只是一个普通缓存 */
	E_VideoMemState eVideoMemState; /* 这个VideoMem的状态 */
	E_PicState ePicState;           /* VideoMem中内存里图片的状态 */
	T_PixelDatas tPixelDatas;       /* 内存: 用来存储图像 */
	struct VideoMem *ptNext;        /* 链表 */
}T_VideoMem, *PT_VideoMem;

typedef struct DispOpr {
	char *name;              /* 显示模块的名字 */
	int Xres;               /* X分辨率 */
	int Yres;               /* Y分辨率 */
	int Bpp;                /* 一个象素用多少位来表示 */
	int lineWidth;          /* 一行数据占据多少字节 */
	unsigned char *pucDispMem;   /* 显存地址 */
	int (*DeviceInit)(void);     /* 设备初始化函数 */
	int (*ShowPixel)(int iPenX, int iPenY, unsigned int dwColor);    /* 把指定座标的象素设为某颜色 */
	int (*CleanScreen)(unsigned int dwBackColor);                    /* 清屏为某颜色 */
	int (*ShowPage)(PT_PixelDatas ptPixelDatas);                         /* 显示一页,数据源自ptVideoMem */
	struct DispOpr *ptNext;      		/* 链表 */
}T_DispOpr, *PT_DispOpr;



int RegisterDispOpr(PT_DispOpr ptDispOpr);

void ShowDispOpr(void);

int DisplayInit(void);

void SelectAndInitDefaultDispDev(char *name);

PT_DispOpr GetDefaultDispDev(void);

int GetDispResolution(int *piXres, int *piYres, int *piBpp);

int AllocVideoMem(int iNum);

PT_VideoMem GetDevVideoMem(void);

PT_VideoMem GetVideoMem(int iID, int bCur);


void PutVideoMem(PT_VideoMem ptVideoMem);

void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor);

void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor);

int FBInit(void);

int GetVideoBufForDisplay(PT_VideoBuf ptFrameBuf);
void FlushPixelDatasToDev(PT_PixelDatas ptPixelDatas);

#endif /* _DISP_MANAGER_H */

