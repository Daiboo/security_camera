#include "config.h"
#include "video_manager.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* /main /dev/video0 */
int main(int argc, char *argv[])
{   
    
    if(argc < 2)
    {
        printf("Usage: %s /dev/video0\n", argv[0]);
        return -1;
    }


    T_VideoDevice g_tVideoDevice;
    T_VideoBuf g_tVideoBuf;
    constructV4l2(&g_tVideoDevice, 1280, 720);
    g_tVideoDevice.InitDevice(argv[1], &g_tVideoDevice);  // 初始化
    g_tVideoDevice.StartDevice(&g_tVideoDevice);         // 开启设备

    int cnt = 0;
    char nameprifix[32];
    while(1){
        if(cnt == 5) break;
        if(g_tVideoDevice.GetFrame(&g_tVideoDevice, &g_tVideoBuf) == 0)
        {
            sprintf(nameprifix, "frame%d.jpg", cnt++);
            int fd = open(nameprifix, O_RDWR | O_CREAT, 0666);
            ssize_t len = write(fd, g_tVideoBuf.aucPixelDatas, g_tVideoBuf.totalBytes);
            if(len < 0) printf("write error\n");
            close(fd);

            g_tVideoDevice.PutFrame(&g_tVideoDevice, &g_tVideoBuf);
        }

    }
    deconstructV4l2(&g_tVideoDevice);

    return 0;
}