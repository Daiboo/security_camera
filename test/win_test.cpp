#include <stdio.h>
#include <QApplication>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <jpeglib.h>
#include <stdlib.h>
#include "config.h"
#include "video_manager.h"
#include "convert_manager.h"

void handle_sigint(int sig);
T_VideoDevice g_tVideoDevice;
T_VideoBuf tConvertBuf;


class MyWindow : public QWidget
{
    Q_OBJECT

public:
    MyWindow(QWidget *parent = nullptr);
    ~MyWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
};



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QLabel label;
    PT_VideoBuf ptVideoBufCur;
    T_VideoBuf tVideoBuf;
 
    PT_VideoConvert ptVideoConvertor;
    int ret;


    if(argc < 2)
    {
        printf("Usage: %s /dev/video0\n", argv[0]);
        return -1;
    }


    signal(SIGINT, handle_sigint);

    
    // 初始化摄像头
    constructV4l2(&g_tVideoDevice, 1280, 720);
    ret = g_tVideoDevice.InitDevice(argv[1], &g_tVideoDevice);  // 初始化
    if(ret != 0)
    {
        printf("InitDevice error\n");
        return -1;
    }
    
    label.resize(g_tVideoDevice.width, g_tVideoDevice.height);

    // 初始化videoconvert
    int iPixelFormatOfVideo = g_tVideoDevice.GetFormat(&g_tVideoDevice);
    int iPixelFormatOfDisp = V4L2_PIX_FMT_RGB24;
    // videoconvert
    VideoConvertInit();
    ptVideoConvertor = GetVideoConvertForFormats(iPixelFormatOfVideo, iPixelFormatOfDisp);
    if (NULL == ptVideoConvertor)
    {
        DBG_PRINTF("can not support this format convert\n");
        return -1;
    }

    // 打开摄像头
    ret = g_tVideoDevice.StartDevice(&g_tVideoDevice);         // 开启设备
    if(ret != 0)
    {
        printf("StartDevice error\n");
        return -1;
    }

    memset(&tVideoBuf, 0, sizeof(tVideoBuf));
    memset(&tConvertBuf, 0, sizeof(tConvertBuf));
    tConvertBuf.pixelFormat     = iPixelFormatOfDisp;
    tConvertBuf.Bpp = 24;

    label.show();

    while(1)
    {
        if(g_tVideoDevice.GetFrame(&g_tVideoDevice, &tVideoBuf))
        {
            printf("GetFrame error\n");
            continue;
        }
        ptVideoBufCur = &tVideoBuf;
        if(iPixelFormatOfVideo != iPixelFormatOfDisp)
        {
            // 转换为RGB
            ret = ptVideoConvertor->Convert(&tVideoBuf, &tConvertBuf);
            printf("Convert %s, ret = %d\n", ptVideoConvertor->name, ret);
            if(ret)
            {
                printf("Convert error\n");
                continue;
            }
            ptVideoBufCur = &tConvertBuf;
        }
        
        // 显示在QLabel上
        QImage image(ptVideoBufCur->aucPixelDatas,tConvertBuf.width,tConvertBuf.height,QImage::Format_RGB888);
        label.setPixmap(QPixmap::fromImage(image));

        g_tVideoDevice.PutFrame(&g_tVideoDevice, &tVideoBuf);
        QApplication::processEvents();
    }

    return app.exec();

}

void handle_sigint(int sig)
{
    deconstructV4l2(&g_tVideoDevice);
    if(tConvertBuf.aucPixelDatas)
    {
        free(tConvertBuf.aucPixelDatas);
        tConvertBuf.aucPixelDatas = NULL;
    }
    exit(0);
}