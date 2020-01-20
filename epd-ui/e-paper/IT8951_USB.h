#ifndef IT8951_USB_H
#define IT8951_USB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IT8951_CMD.h"

#define GLOBAL_REF   0x02
#define PART_REF     0x01
#define AUTO_MODE    0x03
#define INIT_REF     0x00

void show_sense_buffer(SG_IO_HDR*);

void IT8951_Cmd_Inquiry(void);
void IT8951_Cmd_SysInfo(SystemInfo*);
void IT8951_Cmd_MemRead(DWord, Word, Byte*);
void IT8951_Cmd_MemWrite(DWord, Word, Byte*);
void IT8951_Cmd_RegRead(DWord, DWord*);
void IT8951_Cmd_RegWrite(DWord, DWord*);
void IT8951_Cmd_DisplayArea(DWord, DWord, DWord, DWord, DWord, DWord, DWord);
void IT8951_Cmd_LoadImageArea(DWord, DWord, DWord, DWord, DWord);
void IT8951_Cmd_SetTemp(TempArg);
void EPD_display_Area(DWord starX, DWord starY, DWord width, DWord High, int mode);
int dis_epd(uint8_t);
void init_device(void);
#ifdef __cplusplus
}
#endif

#endif  // IT8951_USB_H
