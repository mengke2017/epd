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
    if(x < 0 || x >= (uint16_t)Sys_info.uiWidth || y < 0 || y >= (uint16_t)Sys_info.uiHeight)
		return ;	

	gpFrameBuf[y*Sys_info.uiWidth + x] = color;
}


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
	int fp=0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	unsigned char * buf; 
	fp = open ("/dev/fb0",O_RDWR);


	if (fp < 0){
		printf("Error : Can not open framebuffer device\n");
		return -1;
	}


	if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo)){
		printf("Error reading fixed information\n");
		return -1;
	}

	if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo)){
		printf("Error reading variable information\n");
		return -1;
	}
	buf = (unsigned char *)calloc(1, finfo.smem_len/2);
	
	if(buf == NULL) {
    	printf("calloc mem faile.\n");
	    return -1;
	}
//	printf("reading screen...\n");
	int l = read(fp, buf, finfo.smem_len/2);
//	printf("read len: %d\n", l);
//	unsigned long frame_address= finfo.smem_start;
//    bmp_BitCount = vinfo.bits_per_pixel;

	if (l <= 0) {
    	printf("read /dev/fb0 faile.\n");
	    return -1;
	}
	DrawRGBdata(x, y, vinfo.xres, vinfo.yres, buf);

//	printf("The mem star is :%ld\n", finfo.smem_start);
//	printf("The mem len is :%d\n",finfo.smem_len);
//	printf("The line_length is :%d\n",finfo.line_length);
//	printf("The xres is :%d\n",vinfo.xres);
//	printf("The yres is :%d\n",vinfo.yres);
//	printf("bits_per_pixel is :%d\n",vinfo.bits_per_pixel);
	if(buf != NULL) {	
    	free(buf);
		buf = NULL;
	}
	close (fp);
	return 0;
}
