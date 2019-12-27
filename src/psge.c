#include "psge.h"
#include "shiftjis.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <libcd.h>
#include <libmcrd.h>
#include <libpad.h>

static unsigned char ctrl_port_a[34];
static unsigned char ctrl_port_b[34];

void psInit() {
	ResetCallback();
	InitHeap3((void*)0x800F8000, 0x00100000);
	PadInitMtap(ctrl_port_a, ctrl_port_b);
	PadStartCom();
	CdInit();
}

psDATA* psReadFileDisc(char* filename, unsigned int offset, unsigned int length) {
	CdlFILE file;
	if (CdSearchFile(&file, filename) != NULL) {
		psDATA* data;
		unsigned int first_sector_offset, first_sector_index, sector_count;
		int cd_mode = CdlModeSpeed;
		if (offset >= file.size) {
			return NULL;
		}
		if (length == 0) {
			length = file.size - offset;
		} else if (length > file.size - offset) {
			return NULL;
		}
		first_sector_offset = offset % DISC_SECTOR_SIZE;
		sector_count = (length + first_sector_offset + DISC_SECTOR_SIZE - 1) / DISC_SECTOR_SIZE;
		data = psAllocateData(sector_count * DISC_SECTOR_SIZE);
		if (data == NULL) {
			return NULL;
		}
		first_sector_index = offset / DISC_SECTOR_SIZE;
		CdIntToPos(CdPosToInt(&file.pos) + first_sector_index, &file.pos);
		CdControlB(CdlSetloc, (unsigned char*)&(file.pos), NULL);
		CdControlB(CdlSetmode, (unsigned char*)&cd_mode, NULL);
		CdRead(sector_count, (unsigned long*)data->raw_data, cd_mode);
		data->data += first_sector_offset;
		data->length = length;
		data->sector_count = sector_count;
		return data;
	}
	return NULL;
}

int psWaitDisc() {
	return CdReadSync(1, NULL);
}

psDATA* psAllocateData(unsigned int length) {
	psDATA* data = (psDATA*)malloc3(sizeof(psDATA));
	if (data == NULL) {
		return NULL;
	}
	data->raw_data = (unsigned char*)malloc3(length * sizeof(unsigned char));
	if (data->raw_data == NULL) {
		free3(data);
		return NULL;
	}
	data->data = data->raw_data;
	data->length = length;
	data->sector_count = 0;
	return data;
}

void psClearData(psDATA* data, unsigned char value, unsigned int offset, unsigned int length) {
	if (offset >= data->length) {
		return;
	}
	if (length == 0) {
		length = data->length - offset;
	} else if (length > data->length - offset) {
		return;
	}
	memset(data->data + offset, value, length);
}

psDATA* psCopyData(psDATA* data, unsigned int offset, unsigned int length) {
	psDATA* new_data;
	if (offset >= data->length) {
		return NULL;
	}
	if (length == 0) {
		length = data->length - offset;
	} else if (length > data->length - offset) {
		return NULL;
	}
	new_data = psAllocateData(length);
	if (new_data == NULL) {
		return NULL;
	}
	memcpy(new_data->data, data->data, length);
	return new_data;
}

void psFreeData(psDATA* data) {
	free3(data->raw_data);
	free3(data);
}

int psCheckMemCardStatus(int channel) {
	long result;
	MemCardAccept(channel);
	MemCardSync(0, NULL, &result);
	return result;
}

int psFormatMemCard(int channel) {
	return MemCardFormat(channel);
}

int psCheckMemCardInfo(int channel, char* file_filter, psMEMCARDINFO* out_info, int max_info) {
	struct DIRENTRY direntries[MEMCARD_MAX_BLOCKS];
	long result, entry_count;
	if ((result = MemCardGetDirentry(channel, file_filter, direntries, &entry_count, 0, max_info)) == MEMCARD_STATUS_CONNECTED) {
		int i;
		for (i = 0; i < entry_count; i++) {
			strcpy(out_info[i].name, direntries[i].name);
			out_info[i].block_count = direntries[i].size / MEMCARD_BLOCK_SIZE;
		}
		for (i = entry_count; i < max_info; i++) {
			out_info[i].name[0] = '\0';
			out_info[i].block_count = 0;
		}
		return MEMCARD_STATUS_CONNECTED;
	}
	return result;
}

int psCreateMemCardSave(int channel, char* file_name, unsigned char block_count) {
	if ((block_count == 0) || (block_count > 15)) {
		return MEMCARD_STATUS_INVALID;
	}
	return MemCardCreateFile(channel, file_name, block_count);
}

int psDeleteMemCardSave(int channel, char* file_name) {
	return MemCardDeleteFile(channel, file_name);
}

int psASCIIToShiftJIS(char* src, unsigned char* dest, unsigned int max_length) {
	unsigned int length = strlen(src);
	if (2 * (length + 1) > max_length) {
		return 1;
	}
	while (*src != '\0') {
		unsigned short character = ascii2sjis((unsigned char)*src);
		if (character == 0) {
			return 1;
		}
		dest[0] = (character >> 8) & 0xFF;
		dest[1] = character & 0xFF;
		dest += 2;
		src++;
	}
	dest[0] = 0;
	dest[1] = 0;
	return 0;
}

int psShiftJISToASCII(unsigned char* src, char* dest, unsigned int max_length) {
	unsigned int i, length;
	for (length = 0; ((src[length] != 0) || (src[length + 1] != 0)); length += 2);
	if (length / 2 + 1 > max_length) {
		return 1;
	}
	for (i = 0; i < length; i += 2) {
		unsigned short character = ((unsigned short)(src[0]) << 8) | src[1];
		*dest = (char)sjis2ascii(character);
		if (*dest == '\0') {
			return 1;
		}
		src += 2;
		dest++;
	}
	*dest = '\0';
	return 0;
}

int psOpenMemCardSave(int channel, char* file_name, int mode) {
	return MemCardOpen(channel, file_name, mode);
}

void psCloseMemCardSave() {
	MemCardClose();
	MemCardSync(0, NULL, NULL);
}

int psGenerateMemCardHeader(psMEMCARDHEADER* out_header, unsigned char type, unsigned char block_count, unsigned char* title_shift_jis, unsigned char* clut, unsigned char* image1, unsigned char* image2, unsigned char* image3) {
	unsigned int length;
	if ((type != MEMCARD_TYPE_1_FRAME) && (type != MEMCARD_TYPE_2_FRAMES) && (type != MEMCARD_TYPE_3_FRAMES)) {
		return 1;
	} else if ((block_count == 0) || (block_count > 15)) {
		return 1;
	} else if (title_shift_jis == NULL) {
		return 1;
	} else if ((clut == NULL) || (image1 == NULL)) {
		return 1;
	} else if ((image2 == NULL) && (type != MEMCARD_TYPE_1_FRAME)) {
		return 1;
	} else if ((image3 == NULL) && (type == MEMCARD_TYPE_3_FRAMES)) {
		return 1;
	}
	for (length = 0; ((title_shift_jis[length] != 0) || (title_shift_jis[length + 1] != 0)); length += 2);
	if (length > MEMCARD_TITLE_MAX_LENGTH) {
		return 1;
	}
	out_header->magic_number[0] = 'S';
	out_header->magic_number[1] = 'C';
	out_header->type = type;
	out_header->block_count = block_count;
	memcpy(out_header->title_shift_jis, title_shift_jis, length + 2);
	memset(out_header->pad, 0, 28);
	memcpy(out_header->clut, clut, 32);
	memcpy(out_header->image1, image1, 128);
	if (image2 != NULL) {
		memcpy(out_header->image2, image2, 128);
		if (image3 != NULL) {
			memcpy(out_header->image3, image3, 128);
		}
	}
	return 0;
}

int psWriteMemCardData(unsigned char* data, unsigned int offset, unsigned int length) {
	long result;
	MemCardWriteData((unsigned long*)data, offset, length);
	MemCardSync(0, NULL, &result);
	return result;
}

int psReadMemCardData(unsigned char* data, unsigned int offset, unsigned int length) {
	long result;
	MemCardReadData((unsigned long*)data, offset, length);
	MemCardSync(0, NULL, &result);
	return result;
}

void psStartMemCardMode() {
	PadStopCom();
	MemCardInit(0);
	MemCardStart();
	MemCardSync(0, NULL, NULL);
}

void psStopMemCardMode() {
	MemCardStop();
	MemCardSync(0, NULL, NULL);
	MemCardEnd();
	PadStartCom();
}

int psGetControllerState(int port) {
	return PadGetState(port);
}
