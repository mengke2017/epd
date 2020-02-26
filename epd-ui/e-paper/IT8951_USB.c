#include <math.h>
#include "IT8951_USB.h"
#include "miniGUI.h"

//#define _TEST_SW_ 0

unsigned char sense_buffer[SENSE_LEN];
unsigned char data_buffer[BLOCK_LEN*256];

Byte* gpFrameBuf;
SystemInfo Sys_info;	// SystemInfo structure
DWord gulPanelW, gulPanelH;

char *path = "/dev/sg0";
int evpd = 0, page_code = 0;

void show_sense_buffer(struct sg_io_hdr * hdr) {
	unsigned char * buffer = hdr->sbp;

	int i;
	for (i=0; i<hdr->mx_sb_len; ++i) {
		putchar(buffer[i]);
	}
}

void IT8951_Cmd_Inquiry(void) {

	struct sg_io_hdr * p_hdr = init_io_hdr();

	set_xfer_data(p_hdr, data_buffer, BLOCK_LEN*256);
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);

	int status = 0;
	int fd = open(path, O_RDWR);
	
	if (fd>0) {
		printf("\nCommand: Inquiry\n");
		
		status = IT8951_CMD_INQ(fd, page_code, evpd, p_hdr);
		if (status!=0) {
			show_sense_buffer(p_hdr);
		} 
		else{
//			#if defined(_TEST_SW_)
			int i;
			for(i=8; i<36; i++){
			  printf("%c", data_buffer[i]);
			}
			printf("\n");
//			#endif
		}
	} 
	else {
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void IT8951_Cmd_SysInfo(SystemInfo* Sys_info) {

	int i;
	int status = 0;
	int fd = open(path, O_RDWR);

	SG_IO_HDR *p_hdr = init_io_hdr();
	
	set_xfer_data(p_hdr, Sys_info, (sizeof(SystemInfo)/sizeof(DWord)));
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);
	
	if (fd > 0) {
		
		status = IT8951_CMD_SYSTEM_INFO(fd, page_code, evpd, p_hdr);
		if (status!=0) {
			show_sense_buffer(p_hdr);
		} else {
			SystemInfo *temp = p_hdr->dxferp;

			Sys_info->uiStandardCmdNo = SWAP_32(temp->uiStandardCmdNo);
			Sys_info->uiExtendCmdNo = SWAP_32(temp->uiExtendCmdNo);
			Sys_info->uiSignature = SWAP_32(temp->uiSignature);
			Sys_info->uiVersion = SWAP_32(temp->uiVersion);
			Sys_info->uiWidth = SWAP_32(temp->uiWidth);
			Sys_info->uiHeight = SWAP_32(temp->uiHeight);
			Sys_info->uiUpdateBufBase = SWAP_32(temp->uiUpdateBufBase);
			Sys_info->uiImageBufBase = SWAP_32(temp->uiImageBufBase);
			Sys_info->uiTemperatureNo = SWAP_32(temp->uiTemperatureNo);
			Sys_info->uiModeNo = SWAP_32(temp->uiModeNo);
			
			for(i=0; i<8; i++){
				Sys_info->uiFrameCount[i] = SWAP_32(temp->uiFrameCount[i]);
			}

			Sys_info->uiNumImgBuf = SWAP_32(temp->uiNumImgBuf);
			
			Sys_info->uiWbfSFIAddr = SWAP_32(temp->uiWbfSFIAddr);
			
			for(i=0; i<9; i++){
				Sys_info->uiReserved[i] = SWAP_32(temp->uiReserved[i]);
			}
			
			gulPanelW = Sys_info->uiWidth;
			gulPanelH = Sys_info->uiHeight;

			//#if defined(_TEST_SW_)
			printf("Panel Width: %d\n", Sys_info->uiWidth);
			printf("Panel Height: %d\n", Sys_info->uiHeight);
			printf("Image buffer address: %X\n", Sys_info->uiImageBufBase);
			printf("Update buffer address: %X\n", Sys_info->uiUpdateBufBase);		
	        printf("uiNumImgBuf: %d\n", Sys_info->uiNumImgBuf);
			//#endif
		}
	} 
	else{
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void IT8951_Cmd_MemRead(DWord memAddr, Word length, Byte* revBuf) {

	SG_IO_HDR *p_hdr = init_io_hdr();
	
	//Byte *rev = (Byte*)malloc(sizeof(Byte)*length);
	
	set_xfer_data(p_hdr, revBuf, length);
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);

	int status = 0;
	int fd = open(path, O_RDWR);
	
	if (fd>0) {
//		printf("\nCommand: Read memory\n");
		
		// Notice that the memAddr should be 0x format
		status = IT8951_CMD_READ_MEM(fd, page_code, evpd, p_hdr, memAddr, length);
		if (status!=0) {
			show_sense_buffer(p_hdr);
		} 
		else{
			#if defined(_Test_SW_)
			printf("Read memory 0x%X: \r\n", memAddr);
			int i;
			for(i=0; i<length; i++){
				printf("%X \r", revBuf[i]);
			}
			printf("\n");	
			#endif
		}
	} 
	else{
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void IT8951_Cmd_MemWrite(DWord memAddr, Word length, Byte* srcBuf) {

	SG_IO_HDR *p_hdr = init_io_hdr();
	
	set_xfer_data(p_hdr, srcBuf, length);
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);

	int status = 0;
	int fd = open(path, O_RDWR);
	
	if (fd>0) {
//		printf("\nCommand: Write memory\n");
		
		status = IT8951_CMD_WRITE_MEM(fd, page_code, evpd, p_hdr, memAddr, length);
		if (status!=0) {
			show_sense_buffer(p_hdr);
	//		printf("Write memory status!=0");
		} 
		else{
			#if defined(_TEST_SW_)
			printf("Write memory 0x%X: \r\n", memAddr);
			
			int i;
			for(i=0; i<length; i++){
				printf("%X \r", srcBuf[i]);
			}
			printf("\n");	
			#endif
		}
	} 
	else{
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void IT8951_Cmd_RegRead(DWord regAddr, DWord *rev) {

	SG_IO_HDR *p_hdr = init_io_hdr();
	
	set_xfer_data(p_hdr, rev, sizeof(DWord));
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);

	int status = 0;
	int fd = open(path, O_RDWR);
	
	if (fd>0) {
	//	printf("\nCommand: Read register\n");
		
		status = IT8951_CMD_READ_REG(fd, page_code, evpd, p_hdr, regAddr);
		if (status!=0) {
			show_sense_buffer(p_hdr);
		} 
		else{
			*rev = SWAP_32(*rev);
			
			#if defined(_Test_SW_)
			printf("Read register 0x%X: \r\n", regAddr);
			printf("%X \r", *rev);
			#endif
		}
	} 
	else{
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void IT8951_Cmd_RegWrite(DWord regAddr, DWord *src) {

	SG_IO_HDR *p_hdr = init_io_hdr();
	
	DWord srcIn = SWAP_32(*src);
	
	set_xfer_data(p_hdr, &srcIn, sizeof(DWord));
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);

	int status = 0;
	int fd = open(path, O_RDWR);
	
	if (fd>0) {
	//	printf("\nCommand: Write register\n");
		
		status = IT8951_CMD_WRITE_REG(fd, page_code, evpd, p_hdr, regAddr);
		if (status!=0) {
			show_sense_buffer(p_hdr);
		} 
		else{
			#if defined(_TEST_SW_)
			printf("Write register 0x%X: \r\n", regAddr);
			#endif
		}
	} 
	else{
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void IT8951_Cmd_DisplayArea(DWord dpyX, DWord dpyY, DWord dpyW, DWord dpyH, DWord dpyMode, DWord memAddr, DWord enWaitRdy){
	
	SG_IO_HDR *p_hdr = init_io_hdr();
	
	DisplayArg displayArg;
	
	displayArg.iPosX          = SWAP_32(dpyX); //Conver Little to Big for IT8951/61
    displayArg.iPosY          = SWAP_32(dpyY);
    displayArg.iWidth         = SWAP_32(dpyW);
    displayArg.iHeight        = SWAP_32(dpyH);
    displayArg.iEngineIndex   = SWAP_32(enWaitRdy);
    displayArg.iMemAddr       = SWAP_32(memAddr);
    displayArg.iWavMode       = SWAP_32(dpyMode);
	
	set_xfer_data(p_hdr, &displayArg, sizeof(DisplayArg));
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);

	int status = 0;
	int fd = open(path, O_RDWR);
	
	if (fd>0) {
	//	printf("\nCommand: Display\n");
		
		status = IT8951_CMD_DPY_AREA(fd, page_code, evpd, p_hdr);
		if (status!=0) {
			show_sense_buffer(p_hdr);
		} 
		else{
	//		printf("Refreshing (%d, %d, %d, %d) with mode %d ... \r\n", dpyX, dpyY, dpyW, dpyH, dpyMode);
		}
	} 
	else{
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void IT8951_Cmd_LoadImageArea(DWord memAddr, DWord ldX, DWord ldY, DWord ldW, DWord ldH){

//	printf("\nCommand: Load image\n");
	
	if(ldW <= 2048 && ldW != gulPanelW){
		SG_IO_HDR *p_hdr = init_io_hdr();
			
		//Using IT8951 New USB Command for Loading Image - it Needs IT8951 F/W support
		DWord counter = (SPT_BUF_SIZE/ldW);
		LoadArg loadArg;
			
		int status = 0;
		int fd = open(path, O_RDWR);

		DWord i;
		for(i=0; i<ldH; i+=counter){
			if(counter > (ldH-i)){
				counter = (ldH-i);
			}
				
			Byte* tempBuf = (Byte*)malloc((sizeof(LoadArg)+(ldW*counter)));
				
			loadArg.iX = SWAP_32(ldX);
			loadArg.iY = SWAP_32((ldY + i));
			loadArg.iW = SWAP_32(ldW);
			loadArg.iH = SWAP_32(counter);

			//Set Image Buffer Start Address
			loadArg.iAddress = SWAP_32(memAddr);

			memcpy(tempBuf, &loadArg, sizeof(LoadArg));
			memcpy((tempBuf+sizeof(LoadArg)), (gpFrameBuf+(i*ldW)), (ldW*counter));
						
			set_xfer_data(p_hdr, tempBuf, (sizeof(LoadArg)+(ldW*counter)));
			set_sense_data(p_hdr, sense_buffer, SENSE_LEN);
				
			if (fd>0) {
				status = IT8951_CMD_LOAD_IMG(fd, page_code, evpd, p_hdr);
				if (status!=0) {
					show_sense_buffer(p_hdr);
				} 
				else{
					// Show some information of loaded data?
				}
			} 
			else{
				printf("failed to open sg file %s\n", path);
			}	
			free(tempBuf);
			tempBuf = NULL;
		}
			
		close(fd);
		destroy_io_hdr(p_hdr);
	}
	else{
		//Full Image - Fast Method => Send Multi Lines for each transfer
		DWord counter = (SPT_BUF_SIZE/ldW);
		
		DWord i;
		for(i=0; i<ldH; i+=counter){
			if(counter > (ldH-i)){
				counter = (ldH-i);
			}
			
			//We Send Multi Lines for each Bulk Transfer
			IT8951_Cmd_MemWrite((memAddr+ldX+((ldY+i)*gulPanelW)), (Word)(ldW*counter), (gpFrameBuf+(i*ldW)));
		}
	}	
}

void IT8951_Cmd_SetTemp(TempArg SetTemp) {

	SG_IO_HDR *p_hdr = init_io_hdr();

	Byte flag = SetTemp.ucSetTemp;
	Byte value = SetTemp.ucTempVal;
	
	set_xfer_data(p_hdr, &SetTemp, sizeof(TempArg)/sizeof(Byte));
	set_sense_data(p_hdr, sense_buffer, SENSE_LEN);

	int status = 0;
	int fd = open(path, O_RDWR);
	
	if (fd>0) {
//		printf("\nCommand: Set temperature\n");
		
		status = IT8951_CMD_SET_TEMP(fd, page_code, evpd, p_hdr, SetTemp.ucSetTemp, SetTemp.ucTempVal);
		if (status!=0) {
			show_sense_buffer(p_hdr);
		} 
		else{
			if((flag)==0x00){
				TempArg *temp = (p_hdr->dxferp);
				Byte TempByte;
				
				// will return a word , need to swap
				TempByte = temp->ucSetTemp;
				temp->ucSetTemp = temp->ucTempVal;
				temp->ucTempVal = TempByte;
				
				printf("Get temperature: %X\n", temp->ucTempVal);
			}
			else{
				printf("Set temperature, now forced as %d\n", value);
				printf("Please read temperature again to check it.\n");							
			}
		}
	} 
	else{
		printf("failed to open sg file %s\n", path);
	}
	
	close(fd);
	destroy_io_hdr(p_hdr);
}

void EPD_display_Area(DWord starX, DWord starY, DWord width, DWord High, int mode) {
    IT8951_Cmd_LoadImageArea((Sys_info.uiImageBufBase), starX, starY, gulPanelW, gulPanelH);  //
    IT8951_Cmd_DisplayArea(starX, starY, width, High, mode, (Sys_info.uiImageBufBase), 1);
}
int dis_epd(uint8_t mode) {
    static int i = -1;
    if(Show_linuxfb(0, 0) == 0) {  // 不同
        usleep(10000);
        while(Show_linuxfb(0, 0) == 0) {
            int c = 0;
            c++;
          //  printf("112233");
            usleep(10000);
            if(c > 10)
                break;
        }
//        i ++;
//        if(mode == GLOBAL_REF || mode == PART_REF) {
//            EPD_display_Area(0,0,gulPanelW,gulPanelH,mode);
//        } else {
//            if(i >= 10 || i == -1) {
//                EPD_display_Area(0,0,gulPanelW,gulPanelH,GLOBAL_REF);
//                i = 0;
//            } else {
//                EPD_display_Area(0,0,gulPanelW,gulPanelH,PART_REF);
//            }
//        }
        return 0;
    }
    return 1;
}
void init_device(void) {

    // 0x12: get device name
    IT8951_Cmd_Inquiry();

    // 0x80: get system info
    IT8951_Cmd_SysInfo(&Sys_info);
    gulPanelW = Sys_info.uiWidth;
    gulPanelH = Sys_info.uiHeight;

    // Mode 0: initial
    IT8951_Cmd_DisplayArea(0, 0, gulPanelW, gulPanelH, PART_REF, (Sys_info.uiImageBufBase), 1);

    // set full white
    gpFrameBuf = (unsigned char*)malloc(sizeof(unsigned char)*(gulPanelW*gulPanelH));
    EPD_Clear(0xF0);

    EPD_display_Area(0, 0, gulPanelW, gulPanelH, GLOBAL_REF);
}
