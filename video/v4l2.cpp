#include "video_manager.h"
#include "config.h"

#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

static int g_aiSupportedFormats[] = {V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB565};


static bool isSupportThisFormat(int iPixelFormat)
{
    int i;
    for (i = 0; i < sizeof(g_aiSupportedFormats) / sizeof(g_aiSupportedFormats[0]); i++)
    {
        if (g_aiSupportedFormats[i] == iPixelFormat)
            return 1;
    }
    return 0;
}

/* open
 * VIDIOC_QUERYCAP 确定它是否视频捕捉设备,支持哪种接口(streaming/read,write)
 * VIDIOC_ENUM_FMT 查询支持哪种格式
 * VIDIOC_S_FMT    设置摄像头使用哪种格式
 * VIDIOC_REQBUFS  申请buffer
 对于 streaming:
 * VIDIOC_QUERYBUF 确定每一个buffer的信息 并且 mmap
 * VIDIOC_QBUF     放入队列
 * VIDIOC_STREAMON 启动设备
 * poll            等待有数据
 * VIDIOC_DQBUF    从队列中取出
 * 处理....
 * VIDIOC_QBUF     放入队列
 * ....
 对于read,write:
    read
    处理....
    read
 * VIDIOC_STREAMOFF 停止设备
 *
 */

static int V4l2InitDevice(char *strDevName, PT_VideoDevice ptVideoDevice)
{
    int i;
    int fd;
    int error;
    struct v4l2_capability tV4l2Cap;                    // 设备信息
	struct v4l2_fmtdesc tFmtDesc;                       // 设备支持的格式
    struct v4l2_format  tV4l2Fmt;                       // 设备当前使用的格式
    struct v4l2_requestbuffers tV4l2ReqBuffs;           // 申请buffer
    struct v4l2_buffer tV4l2Buf;            


    fd = open(strDevName, O_RDWR);
    if (fd < 0)
    {
        DBG_PRINTF("can not open %s\n", strDevName);
        return -1;
    }
    ptVideoDevice->fd = fd;

    memset(&tV4l2Cap, 0, sizeof(struct v4l2_capability));
    error = ioctl(fd, VIDIOC_QUERYCAP, &tV4l2Cap);
    if(error < 0)
    {
        DBG_PRINTF("can not get capability of %s\n", strDevName);
        return -1;
    }

    if (!(tV4l2Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        DBG_PRINTF("%s is not a video capture device\n", strDevName);
        goto err_exit;
    }

    if (!(tV4l2Cap.capabilities & V4L2_CAP_STREAMING))
    {
        DBG_PRINTF("%s does not support streaming i/o\n", strDevName);
        goto err_exit;
    }

    memset(&tFmtDesc, 0, sizeof(struct v4l2_fmtdesc));
    tFmtDesc.index = 0;
	tFmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while((error = ioctl(fd, VIDIOC_ENUM_FMT, &tFmtDesc)) == 0)
    {
        if (isSupportThisFormat(tFmtDesc.pixelformat))
        {
            ptVideoDevice->pixelFormat = tFmtDesc.pixelformat;
            printf("选择的格式为: %s\n", tFmtDesc.description);
            break;
        }
        tFmtDesc.index++;
    }

    if(!ptVideoDevice->pixelFormat)
    {
        DBG_PRINTF("can not find a suitable format for %s\n", strDevName);
        goto err_exit;
    }

    /* set format in */

    memset(&tV4l2Fmt, 0, sizeof(struct v4l2_format));
    tV4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Fmt.fmt.pix.pixelformat = ptVideoDevice->pixelFormat;
    tV4l2Fmt.fmt.pix.width       = ptVideoDevice->width;
    tV4l2Fmt.fmt.pix.height      = ptVideoDevice->height;
    tV4l2Fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    /* 如果驱动程序发现无法某些参数(比如分辨率),
     * 它会调整这些参数, 并且返回给应用程序
     */
    error = ioctl(fd, VIDIOC_S_FMT, &tV4l2Fmt); 
    if (error) 
    {
    	DBG_PRINTF("Unable to set format\n");
        goto err_exit;        
    }
    ptVideoDevice->width = tV4l2Fmt.fmt.pix.width;
    ptVideoDevice->height = tV4l2Fmt.fmt.pix.height;

    /* request buffers */
    memset(&tV4l2ReqBuffs, 0, sizeof(struct v4l2_requestbuffers));
    tV4l2ReqBuffs.count = NB_BUFFER;
    tV4l2ReqBuffs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2ReqBuffs.memory = V4L2_MEMORY_MMAP;

    error = ioctl(fd, VIDIOC_REQBUFS, &tV4l2ReqBuffs);
    if (error)
    {
        DBG_PRINTF("Unable to allocate buffers\n");
        goto err_exit;
    }
    ptVideoDevice->VideoBufCnt = tV4l2ReqBuffs.count;

    if (tV4l2Cap.capabilities & V4L2_CAP_STREAMING)
    {
        // mmap buffers
        for (i = 0; i < ptVideoDevice->VideoBufCnt; i++)
        {
            memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
            tV4l2Buf.index = i;
            tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            tV4l2Buf.memory = V4L2_MEMORY_MMAP;

            error = ioctl(fd, VIDIOC_QUERYBUF, &tV4l2Buf);
            if (error)
            {
                DBG_PRINTF("Unable to query buffer\n");
                goto err_exit;
            }


            ptVideoDevice->VideoBufLen = tV4l2Buf.length;
            ptVideoDevice->pucVideBuf[i] = (unsigned char*)mmap(NULL, tV4l2Buf.length, 
                                                PROT_READ, MAP_SHARED, fd, 
                                                tV4l2Buf.m.offset);
            if (ptVideoDevice->pucVideBuf[i] == MAP_FAILED)
            {
                DBG_PRINTF("Unable to mmap buffer\n");
                goto err_exit;
            }
        }

        // queue buffers
        for (i = 0; i < ptVideoDevice->VideoBufCnt; i++)
        {
            memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
            tV4l2Buf.index = i;
            tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            tV4l2Buf.memory = V4L2_MEMORY_MMAP;

            error = ioctl(fd, VIDIOC_QBUF, &tV4l2Buf);
            if (error)
            {
                DBG_PRINTF("Unable to queue buffer\n");
                goto err_exit;
            }
        }
    }

    return 0;
err_exit:
    close(fd);
    return -1;
}


static int V4l2ExitDevice(PT_VideoDevice ptVideoDevice)
{
    int i;
    int error;

    if (ptVideoDevice->fd)
    {
        for (i = 0; i < ptVideoDevice->VideoBufCnt; i++)
        {
            if(ptVideoDevice->pucVideBuf[i])
            {
                munmap(ptVideoDevice->pucVideBuf[i], ptVideoDevice->VideoBufLen);
                ptVideoDevice->pucVideBuf[i] = NULL;
            }
        }

        if (ptVideoDevice->StopDevice(ptVideoDevice))
        {
            return -1;
        }

        close(ptVideoDevice->fd);
    }

    return 0;
}

static int V4l2GetFrameForStreaming(PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf)
{
    struct pollfd tFds[1];
    int iRet;
    struct v4l2_buffer tV4l2Buf;

    /* poll */
    tFds[0].fd     = ptVideoDevice->fd;
    tFds[0].events = POLLIN;
    iRet = poll(tFds, 1, -1);
    if (iRet <= 0)
    {
        DBG_PRINTF("poll error!\n");
        return -1;
    }

    /* VIDIOC_DQBUF */
    memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
    tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Buf.memory = V4L2_MEMORY_MMAP;
    iRet = ioctl(ptVideoDevice->fd, VIDIOC_DQBUF, &tV4l2Buf);
    if (iRet < 0) 
    {
    	DBG_PRINTF("Unable to dequeue buffer.\n");
    	return -1;
    }

    ptVideoDevice->VideoBufCurIndex = tV4l2Buf.index;

    ptVideoBuf->width    = ptVideoDevice->width;
    ptVideoBuf->height   = ptVideoDevice->height;
    ptVideoBuf->Bpp      =  (ptVideoDevice->pixelFormat == V4L2_PIX_FMT_YUYV) ? 16 : \
                            (ptVideoDevice->pixelFormat == V4L2_PIX_FMT_MJPEG) ? 0 :  \
                            (ptVideoDevice->pixelFormat == V4L2_PIX_FMT_RGB565) ? 16 :  \
                            0;
    ptVideoBuf->lineBytes = ptVideoDevice->width * ptVideoBuf->Bpp / 8;
    ptVideoBuf->totalBytes = tV4l2Buf.bytesused;
    ptVideoBuf->aucPixelDatas = ptVideoDevice->pucVideBuf[tV4l2Buf.index]; 
    
    return 0;
}

static int V4l2PutFrameForStreaming(PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf)
{
    /* VIDIOC_QBUF */
    struct v4l2_buffer tV4l2Buf;
    int error;
    
	memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
	tV4l2Buf.index  = ptVideoDevice->VideoBufCurIndex;
	tV4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Buf.memory = V4L2_MEMORY_MMAP;
	error = ioctl(ptVideoDevice->fd, VIDIOC_QBUF, &tV4l2Buf);
	if (error) 
    {
	    DBG_PRINTF("Unable to queue buffer.\n");
	    return -1;
	}
    return 0;
}


static int V4l2StartDevice(PT_VideoDevice ptVideoDevice)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int error;

    error = ioctl(ptVideoDevice->fd, VIDIOC_STREAMON, &type);
    if (error) 
    {
    	DBG_PRINTF("Unable to start capture.\n");
    	return -1;
    }
    return 0;
}

static int V4l2StopDevice(PT_VideoDevice ptVideoDevice)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int error;

    error = ioctl(ptVideoDevice->fd, VIDIOC_STREAMOFF, &type);
    if (error) 
    {
    	DBG_PRINTF("Unable to stop capture.\n");
    	return -1;
    }
    return 0;
}

static int V4l2GetFormat(PT_VideoDevice ptVideoDevice)
{
    return ptVideoDevice->pixelFormat;
}


int constructV4l2(PT_VideoDevice ptVideoDevice)
{
    ptVideoDevice->InitDevice = V4l2InitDevice;
    ptVideoDevice->ExitDevice = V4l2ExitDevice;
    ptVideoDevice->GetFormat = V4l2GetFormat;
    ptVideoDevice->GetFrame = V4l2GetFrameForStreaming;
    ptVideoDevice->PutFrame = V4l2PutFrameForStreaming;
    ptVideoDevice->StartDevice = V4l2StartDevice;
    ptVideoDevice->StopDevice = V4l2StopDevice;

    return 0;
}

/* 构造这个对象 */
int constructV4l2(PT_VideoDevice ptVideoDevice, int width, int height)
{
    ptVideoDevice->InitDevice = V4l2InitDevice;
    ptVideoDevice->ExitDevice = V4l2ExitDevice;
    ptVideoDevice->GetFormat = V4l2GetFormat;
    ptVideoDevice->GetFrame = V4l2GetFrameForStreaming;
    ptVideoDevice->PutFrame = V4l2PutFrameForStreaming;
    ptVideoDevice->StartDevice = V4l2StartDevice;
    ptVideoDevice->StopDevice = V4l2StopDevice;

    ptVideoDevice->width = width;
    ptVideoDevice->height = height;

    return 0;
}


int deconstructV4l2(PT_VideoDevice ptVideoDevice)
{   
    if(ptVideoDevice->ExitDevice)
        ptVideoDevice->ExitDevice(ptVideoDevice);
    return 0;
}

