#include <unistd.h>
#include <scsi/scsi_ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IT8951_CMD.h"

SG_IO_HDR* init_io_hdr() {

	SG_IO_HDR *p_scsi_hdr = (SG_IO_HDR*)malloc(sizeof(SG_IO_HDR));
	memset(p_scsi_hdr, 0, sizeof(SG_IO_HDR));
	if (p_scsi_hdr) {
		p_scsi_hdr->interface_id = 'S';
		p_scsi_hdr->flags = SG_FLAG_LUN_INHIBIT;
	}

	return p_scsi_hdr;
}

void destroy_io_hdr(SG_IO_HDR * p_hdr) {
	if (p_hdr) {
		free(p_hdr);
	}
}

void set_xfer_data(SG_IO_HDR * p_hdr, void * data, unsigned int length) {
	if (p_hdr) {
		p_hdr->dxferp = data;
		p_hdr->dxfer_len = length;
	}
}

void set_sense_data(SG_IO_HDR * p_hdr, unsigned char * data, unsigned int length) {
	if (p_hdr) {
		p_hdr->sbp = data;
		p_hdr->mx_sb_len = length;
	}
}


Word SWAP_16(Word input){
	Word output;
	output = ((input&0xFF)<<8)|((input>>8)&0xFF);
	
	return output;
}

DWord SWAP_32(DWord input){
	DWord output;
	output = ((input&0xFF)<<24)|((input>>8&0xFF)<<16)|((input>>16&0xFF)<<8)|(input>>24&0xFF);

	return output;
}


// 0x12: inquiry
int IT8951_CMD_INQ(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr) {
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];

	/* set the cdb format */
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i]=IT8951_USB_INQUIRY;
				break;
			default:
				cdb[i]=0x00;
				break;
		}
	}
	
	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;

	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_INQUIRY);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}

// 0x80: get system information
int IT8951_CMD_SYSTEM_INFO(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr) {
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	/* set the cdb format */
    int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 2:
				cdb[i] = 0x38;
				break;
			case 3:
				cdb[i] = 0x39;
				break;
			case 4:
				cdb[i] = 0x35;
				break;
			case 5:
				cdb[i] = 0x31;
				break;
			case 6:
				cdb[i] = IT8951_USB_OP_GET_SYS;
				break;
			case 8:
				cdb[i] = 0x01;
				break;
			case 10:
				cdb[i] = 0x02;
				break;
			default:
				cdb[i] = 0x00;
		}
	}

	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_GET_SYS);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}

// 0x81: read memory
int IT8951_CMD_READ_MEM(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr, DWord memAddr, Word length){
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	/* set the cdb format */
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 2:
				cdb[i] = (Byte)((memAddr>>24)&0xFF);
				break;
			case 3:
				cdb[i] = (Byte)((memAddr>>16)&0xFF);
				break;
			case 4:
				cdb[i] = (Byte)((memAddr>>8)&0xFF);
				break;
			case 5:
				cdb[i] = (Byte)(memAddr&0xFF);
				break;
			case 6:
				cdb[i] = IT8951_USB_OP_READ_MEM;
				break;
			case 7:
				cdb[i] = (Byte)((length>>8)&0xFF);
				break;
			case 8:
				cdb[i] = (Byte)(length&0xFF);
				break;
			default:
				cdb[i] = 0x00;
		}
	}

	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_READ_MEM);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}

// 0x82: write memory
int IT8951_CMD_WRITE_MEM(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr, DWord memAddr, Word length){
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 2:
				cdb[i] = (Byte)((memAddr>>24)&0xFF);
				break;
			case 3:
				cdb[i] = (Byte)((memAddr>>16)&0xFF);
				break;
			case 4:
				cdb[i] = (Byte)((memAddr>>8)&0xFF);
				break;
			case 5:
				cdb[i] = (Byte)(memAddr&0xFF);
				break;
			case 6:
				#if defined(EN_FAST_WRITE_MEM)
					cdb[i] = IT8951_USB_OP_FAST_WRITE_MEM;
				#else
					cdb[i] = IT8951_USB_OP_WRITE_MEM;
				#endif //} EN_FAST_WRITE_MEM
				break;
			case 7:
				cdb[i] = (Byte)((length>>8)&0xFF);
				break;
			case 8:
				cdb[i] = (Byte)(length&0xFF);
				break;
			default:
				cdb[i] = 0x00;
		}
	}
	
	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_WRITE_MEM);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}
	
// 0x83: read register
int IT8951_CMD_READ_REG(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr, DWord regAddr){
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 2:
				cdb[i] = (Byte)((regAddr>>24)&0xFF);
				break;
			case 3:
				cdb[i] = (Byte)((regAddr>>16)&0xFF);
				break;
			case 4:
				cdb[i] = (Byte)((regAddr>>8)&0xFF);
				break;
			case 5:
				cdb[i] = (Byte)(regAddr&0xFF);
				break;
			case 6:
				cdb[i] = IT8951_USB_OP_READ_REG;
				break;
			case 8:
				cdb[i] = 0x04;
				break;
			default:
				cdb[i] = 0x00;
		}
	}
	
	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_READ_REG);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}
	
// 0x84: write register
int IT8951_CMD_WRITE_REG(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr, DWord regAddr){
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 2:
				cdb[i] = (Byte)((regAddr>>24)&0xFF);
				break;
			case 3:
				cdb[i] = (Byte)((regAddr>>16)&0xFF);
				break;
			case 4:
				cdb[i] = (Byte)((regAddr>>8)&0xFF);
				break;
			case 5:
				cdb[i] = (Byte)(regAddr&0xFF);
				break;
			case 6:
				cdb[i] = IT8951_USB_OP_WRITE_REG;
				break;
			case 8:
				cdb[i] = 0x04;
				break;
			default:
				cdb[i] = 0x00;
		}
	}
	
	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_WRITE_REG);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}

// 0x94: display area
int IT8951_CMD_DPY_AREA(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr){
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 6:
				cdb[i] = IT8951_USB_OP_DPY_AREA;
				break;
			default:
				cdb[i] = 0x00;
		}
	}
	
	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_DPY_AREA);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}

// 0xA2: load image
int IT8951_CMD_LOAD_IMG(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr){

	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 6:
				cdb[i] = IT8951_USB_OP_LD_IMG_AREA;
				break;
			default:
				cdb[i] = 0x00;
		}
	}
	
	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_LD_IMG_AREA);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}

// 0xA4: set temperature
int IT8951_CMD_SET_TEMP(int fd, int page_code, int evpd, SG_IO_HDR *p_hdr, Byte TempOption, Byte TempValue) {
	
	p_hdr->cmd_len = 16;
	Byte cdb[(p_hdr->cmd_len)];
	
	/* set the cdb format */
	int i;
	for(i=0; i<(p_hdr->cmd_len); i++){
		switch(i){
			case 0:
				cdb[i] = 0xFE;
				break;
			case 6:
				cdb[i] = IT8951_USB_OP_FSET_TEMP;
				break;
			case 7:
				cdb[i] = (Byte)TempOption;
				break;
			case 8:
				cdb[i] = (Byte)TempValue;
				break;	
			default:
				cdb[i] = 0x00;
		}
	}

	p_hdr->cmdp = cdb;
	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	
	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		printf("Sending SCSI Command failed: 0x%X\r\n", IT8951_USB_OP_FSET_TEMP);
		close(fd);
		exit(1);
	}

	return p_hdr->status;
}

