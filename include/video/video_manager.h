#ifndef __VIDEO_MANAGER_H__
#define __VIDEO_MANAGER_H__


#define NB_BUFFER 4

struct VideoDevice;
typedef struct VideoDevice T_VideoDevice, *PT_VideoDevice;
typedef struct VideoBuf T_VideoBuf, *PT_VideoBuf;

struct VideoBuf {
    int width;                       /* 宽度: 一行有多少个象素 */
	int height;                      /* 高度: 一列有多少个象素 */
	int Bpp;                         /* 一个象素用多少位来表示 */
	int lineBytes;                   /* 一行数据有多少字节 */
	int totalBytes;                  /* 所有字节数 */ 
    int pixelFormat;
	unsigned char *aucPixelDatas;    /* 象素数据存储的地方 */

};

struct VideoDevice {
    int fd;            
    int pixelFormat;                        // 帧格式
    int width;                              // 帧 宽
    int height;                             // 帧 高
    unsigned int VideoBufCnt;               // buffer count
    int VideoBufLen;
    int VideoBufCurIndex;
    unsigned char *pucVideBuf[NB_BUFFER];   // streaming mmap mem

    int (*InitDevice)(char *strDevName, PT_VideoDevice ptVideoDevice);
    int (*ExitDevice)(PT_VideoDevice ptVideoDevice);
    int (*GetFrame)(PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf);
    int (*GetFormat)(PT_VideoDevice ptVideoDevice);
    int (*PutFrame)(PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf);
    int (*StartDevice)(PT_VideoDevice ptVideoDevice);
    int (*StopDevice)(PT_VideoDevice ptVideoDevice);
};


int constructV4l2(PT_VideoDevice ptVideoDevice);
int constructV4l2(PT_VideoDevice ptVideoDevice, int width, int height);
int deconstructV4l2(PT_VideoDevice ptVideoDevice);

#endif // __VIDEO_MANAGER_H__