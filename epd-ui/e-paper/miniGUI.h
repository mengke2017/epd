#ifndef _miniGUI_H_
#define _miniGUI_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "IT8951_CMD.h" 

typedef struct 
{
  uint16_t X;
  uint16_t Y;
}Point, *pPoint; 

typedef struct
{
    uint16_t cfType;
    uint32_t cfSize;
    uint32_t cfReserved;
    uint32_t cfoffBits;
}__attribute__((packed)) BITMAPFILEHEADER;

typedef struct
{
    uint32_t ciSize;
    uint32_t ciWidth;
    uint32_t ciHeight;
    uint16_t ciPlanes;
    uint16_t ciBitCount;
    uint32_t ciCompress;
    uint32_t ciSizeImage;
    uint32_t ciXPelsPerMeter;
    uint32_t ciYPelsPerMeter;
    uint32_t ciClrUsed;
    uint32_t ciClrImportant;
}__attribute__((packed)) BITMAPINFOHEADER;

typedef struct
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t reserved;
}__attribute__((packed)) PIXEL;

#define ABS(X)    ((X) > 0 ? (X) : -(X))     

void EPD_Clear(uint8_t Color);
void EPD_DrawPixel(uint16_t x0,uint16_t y0,uint8_t color);
void EPD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint8_t color);
void EPD_DrawRect(uint16_t Xpos,uint16_t Ypos,uint16_t Width,uint16_t Height,uint8_t color);
void EPD_DrawCircle(uint16_t Xpos,uint16_t Ypos,uint16_t Radius,uint8_t color);
void EPD_DrawPolygon(pPoint Points,uint16_t PointCount,uint8_t color);
void EPD_DrawEllipse(uint16_t Xpos,uint16_t Ypos,uint16_t XRadius,uint16_t YRadius,uint8_t color);
void EPD_FillRect(uint16_t Xpos,uint16_t Ypos,uint16_t Width,uint16_t Height,uint8_t color);
void EPD_FillCircle(uint16_t Xpos,uint16_t Ypos,uint16_t Radius,uint8_t color);
void EPD_PutChar(uint16_t Xpos,uint16_t Ypos,uint8_t ASCI,uint8_t charColor,uint8_t bkColor);
void EPD_Text(uint16_t Xpos,uint16_t Ypos,uint8_t *str,uint8_t Color,uint8_t bkColor);
void EPD_DrawBitmap(uint16_t Xpos, uint16_t Ypos,uint16_t *bmp);
void EPD_DrawMatrix(uint16_t Xpos, uint16_t Ypos,uint16_t Width, uint16_t High,const uint16_t* Matrix);
uint8_t Show_bmp(uint32_t x, uint32_t y,char *path);
uint8_t Show_linuxfb(uint32_t x, uint32_t y);
#ifdef __cplusplus
}
#endif
#endif
