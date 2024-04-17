#include "config.h"
#include "video_manager.h"
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

void handle_sigint(int sig);
T_VideoDevice g_tVideoDevice;



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QLabel label;
    
    label.show();


    if(argc < 2)
    {
        printf("Usage: %s /dev/video0\n", argv[0]);
        return -1;
    }

    signal(SIGINT, handle_sigint);

    
    T_VideoBuf g_tVideoBuf;
    constructV4l2(&g_tVideoDevice, 1280, 720);
    g_tVideoDevice.InitDevice(argv[1], &g_tVideoDevice);  // 初始化
    g_tVideoDevice.StartDevice(&g_tVideoDevice);         // 开启设备

    label.resize(g_tVideoDevice.width, g_tVideoDevice.height);


    while(1)
    {
        if(g_tVideoDevice.GetFrame(&g_tVideoDevice, &g_tVideoBuf) == 0)
        {
            printf("1\n");
            // 创建一个JPEG解压对象
            jpeg_decompress_struct cinfo;
            jpeg_error_mgr jerr;
            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_decompress(&cinfo);

            printf("1\n");
            // 设置JPEG数据源
            jpeg_mem_src(&cinfo, g_tVideoBuf.aucPixelDatas, g_tVideoBuf.totalBytes);
            // 打印g_tVideoBuf.aucPixelDatas前两个字节
            printf("0x%02x 0x%02x\n", g_tVideoBuf.aucPixelDatas[0], g_tVideoBuf.aucPixelDatas[1]);
            printf("2\n");
            jpeg_read_header(&cinfo, TRUE);
            // 开始解压
            jpeg_start_decompress(&cinfo);
    
        
            // 输出图像的宽度和高度
            printf("output_width: %d\n", cinfo.output_width);
            printf("output_height: %d\n", cinfo.output_height);
            printf("output_components: %d\n", cinfo.output_components);
            printf("3\n");
            // 读取像素数据
            unsigned char *rawData = new unsigned char[cinfo.output_width * cinfo.output_height * cinfo.output_components];
            printf("4\n");
            while (cinfo.output_scanline < cinfo.output_height) {
                unsigned char *bufferArray[1];
                bufferArray[0] = rawData + (cinfo.output_scanline) * cinfo.output_width * cinfo.output_components;
                jpeg_read_scanlines(&cinfo, bufferArray, 1);
            }
            printf("5\n");
            QImage image(rawData,cinfo.output_width,cinfo.output_height,QImage::Format_RGB888);
            // QImage image = QImage::fromData(rawData, cinfo.output_width * cinfo.output_height * cinfo.output_components, "RGB");
            printf("6\n");
            label.setPixmap(QPixmap::fromImage(image));
            printf("7\n");
            delete[] rawData;
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
            g_tVideoDevice.PutFrame(&g_tVideoDevice, &g_tVideoBuf);
            QApplication::processEvents();
        }

    }

    // return app.exec();
    return 0;

}

void handle_sigint(int sig)
{
    deconstructV4l2(&g_tVideoDevice);
    
    exit(0);
}