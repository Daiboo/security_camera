#include "config.h"
#include "video_manager.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jpeglib.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <jpeglib.h> // 记得编译的时候-ljpeg

// convert mjpeg frame to RGB24
int MJPEG2RGB(uint8_t *data_frame, int bytesused, int *cnt)
{
    // variables:

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned int width, height;
    // data points to the mjpeg frame received from v4l2.
    unsigned char *data = data_frame;
    size_t data_size = bytesused;

    // all the pixels after conversion to RGB.
    unsigned char *pixels; // to store RBG 存放RGB结果
    int pixel_size = 0;    // size of one pixel
    if (data == NULL || data_size <= 0)
    {
        printf("Empty data!\n");
        return -1;
    }
    uint8_t h1 = 0xFF;
    uint8_t h2 = 0xD8; // jpg的头部两个字节
    printf("0x%02x 0x%02x\n", data[0], data[1]);
    if(data[0] != h1 || data[1] != h2)
    {
        // error header
        printf("wrong header %d\n ",*cnt);
        return -2;
    }
    // ... In the initialization of the program:
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, data_size);
    int rc = jpeg_read_header(&cinfo, TRUE);
    if (!(1 == rc))
    {
        printf("Not a jpg frame.\n");
        return -2;
    }
    jpeg_start_decompress(&cinfo);
    width = cinfo.output_width;
    height = cinfo.output_height;
    pixel_size = cinfo.output_components; // 3
    int bmp_size = width * height * pixel_size;
    pixels = (unsigned char *)malloc(bmp_size);

    // ... Every frame:

    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *temp_array[] = {pixels + (cinfo.output_scanline) * width * pixel_size};
        jpeg_read_scanlines(&cinfo, temp_array, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // Write the decompressed bitmap out to a ppm file, just to make sure
    // 保存为PPM6格式（P6	Pixmap	Binary）

    char fname[25] = {0}; // file name

    sprintf(fname, "output_%d.ppm", (*cnt)++); // cnt 是用来计算的全局变量

    char buf[50]; // for header
    rc = sprintf(buf, "P6 %d %d 255\n", width, height);
    FILE *fd = fopen(fname, "w");
    fwrite(buf, rc, 1, fd);
    fwrite(pixels, bmp_size, 1, fd);
    fflush(fd);
    fclose(fd);

    free(pixels); // free

    return 0;
}

/* /main /dev/video0 */
int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Usage: %s /dev/video0\n", argv[0]);
        return -1;
    }

    T_VideoDevice g_tVideoDevice;
    T_VideoBuf g_tVideoBuf;
    constructV4l2(&g_tVideoDevice, 1280, 720);
    g_tVideoDevice.InitDevice(argv[1], &g_tVideoDevice); // 初始化
    g_tVideoDevice.StartDevice(&g_tVideoDevice);         // 开启设备

    int cnt = 0;
    char fname[25] = {0}; // file nam
    while (1)
    {
        if (cnt == 5)
            break;
        if (g_tVideoDevice.GetFrame(&g_tVideoDevice, &g_tVideoBuf) == 0)
        {

            MJPEG2RGB(g_tVideoBuf.aucPixelDatas, g_tVideoBuf.totalBytes, &cnt);

            g_tVideoDevice.PutFrame(&g_tVideoDevice, &g_tVideoBuf);
        }
    }
    deconstructV4l2(&g_tVideoDevice);

    return 0;
}