#include <string.h>
#include "miniGUI.h"

extern uint8_t* gpFrameBuf; //Host Source Frame buffer
extern SystemInfo Sys_info;	// SystemInfo structure

void EPD_Clear(uint8_t Color)
{
	memset(gpFrameBuf, Color, Sys_info.uiWidth * Sys_info.uiHeight);
}

void EPD_DrawRGBPixel(int16_t x0, int16_t y0, uint8_t color)
{
    int32_t x = x0, y = y0;

	x = Sys_info.uiWidth - y0;
	y = x0;
    if(x < 0 || x >= (uint16_t)Sys_info.uiWidth
            || y < 0 || y >= (uint16_t)Sys_info.uiHeight)
		return ;	

	gpFrameBuf[y*Sys_info.uiWidth + x] = color;
}
//uint8_t  bmp_BitCount;

static void DrawRGBdata(uint16_t Xpos, uint16_t Ypos,uint16_t Width, uint16_t High,const uint8_t* Matrix)
{
	uint16_t i,j,x,y;
	uint8_t R= 0,G = 0,B = 0;
//	uint8_t temp1,temp2;
	uint8_t Gray;
	uint32_t *pbuf;

	pbuf = (uint32_t* )Matrix;
	for (y=0,j=Ypos;y<High;y++,j++)
	{
 		for (x=0,i=Xpos;x<Width;x++,i++)
		{
/*			switch(bmp_BitCount)
			{
				case 1:
				case 4:
				case 8:
					R = palette[Matrix[(y*Width+x)]].red;
					G = palette[Matrix[(y*Width+x)]].green;
					B = palette[Matrix[(y*Width+x)]].blue;
				break;

				case 16:
					temp1 = Matrix[(y*Width+x)*2];
					temp2 = Matrix[(y*Width+x)*2+1];
					R = (temp1 & 0x7c)<<1;
					G = (((temp1 & 0x03) << 3 ) | ((temp2&0xe0) >> 5))<<3;
					B = (temp2 & 0x1f)<<3;
				break;

				case 24:
					R = Matrix[(y*Width+x)*3];
					G = Matrix[(y*Width+x)*3+1];
					B = Matrix[(y*Width+x)*3+2];
				break;
				//ARGB_8888
				case 32:
					B = (pbuf[(y*Width+x)] & 0x000000FF)>>0;
					G = (pbuf[(y*Width+x)] & 0x0000FF00) >> 8;
					R = (pbuf[(y*Width+x)] & 0x00FF0000) >> 16;
				break;
				
				default:
				break;
			}*/
		
			B = (pbuf[(y*Width+x)] & 0x000000FF)>>0;
			G = (pbuf[(y*Width+x)] & 0x0000FF00) >> 8;
			R = (pbuf[(y*Width+x)] & 0x00FF0000) >> 16;

		//	Gray = (R*299 + G*587 + B*114 + 500) / 1000;
			Gray = (R*38 + G*75 + B*15) >> 7;
			if(Gray >= 0x80) {
			    Gray = 0xf0;
			} else {
			    Gray = 0x00;
			}

			EPD_DrawRGBPixel(i, j, Gray);	
		}
	}
}

uint8_t Show_linuxfb(uint32_t x, uint32_t y) {
    static int fp=0;
    static int flag = 0;
    static struct fb_var_screeninfo vinfo;
    static struct fb_fix_screeninfo finfo;
    static unsigned char * old_buf = NULL;
    static void *bits = NULL;
    if(flag == 0)
        fp = open ("/dev/fb0",O_RDWR);

    if (fp < 0){
        printf("Error : Can not open framebuffer device\n");
        return -1;
    }

    if(flag == 0) {
        if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo)){
            printf("Error reading fixed information\n");
            return -1;
        }

        if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo)){
            printf("Error reading variable information\n");
            return -1;
        }

        bits = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
        if (bits == MAP_FAILED) {
            perror("failed to mmap framebuffer");
            close(fp);
            return -1;
        }

        if(old_buf == NULL) {
            old_buf = (unsigned char *)calloc(1, finfo.smem_len/2);
            if(old_buf == NULL) {
                printf("calloc mem faile.\n");
                return -1;
            }
        }
        }
//    bmp_BitCount = vinfo.bits_per_pixel;

/////    DrawRGBdata(x, y, vinfo.xres, vinfo.yres, (uint8_t*)bits);

    flag = 1;
    if(strncmp((int8_t*)bits, (int8_t*)old_buf, finfo.smem_len/2) != 0) {
        strncpy((int8_t*)old_buf, (int8_t*)bits, finfo.smem_len/2);
//        munmap(bits,finfo.smem_len);
        //close (fp);
        return 0;  // 刷新
    }
//    munmap(bits,finfo.smem_len);
   // close (fp);
    return 1;  // 不刷新
}
