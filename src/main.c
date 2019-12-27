#include <stdlib.h>
#include <stdio.h>

#include "psge.h"

void test_cd_reading(char* test_name, int index, int testCount) {
	char* test_string = "TEST STRING";
	psDATA* data;
	data = psReadFileDisc("\\TEST.DAT;1", 201600, 11);
	if (data == NULL) {
		printf("(%d/%d) %s: Error! (file not found)\n", index, testCount, test_name);
		return;
	}
	while (psWaitDisc() > 0);
	if (memcmp(data->data, test_string, data->length) != 0) {
		printf("(%d/%d) %s: Error! (read data is invalid)\n", index, testCount, test_name);
		return;
	}
	printf("(%d/%d) %s: OK.\n", index, testCount, test_name);
}

void test_memory_card(char* test_name, int index, int testCount) {
	psMEMCARDHEADER header, test_header;
	char* file_name = "BIPNTS-69696KAWAII";
	char* title = "Pantsu";
	unsigned char sjis_title[64];
	int result;
	psDATA* pantsu;
	pantsu = psReadFileDisc("\\PANTSU.TIM;1", 0, 0);
	if (pantsu == NULL) {
		printf("(%d/%d) %s: Error! (CD read failure)\n", index, testCount, test_name);
		return;
	}
	while (psWaitDisc() > 0);
	psStartMemCardMode();
	result = psCheckMemCardStatus(MEMCARD_CHANNEL_A);
	if (result == MEMCARD_STATUS_UNFORMATTED) {
		if (psFormatMemCard(MEMCARD_CHANNEL_A) != MEMCARD_STATUS_CONNECTED) {
			printf("(%d/%d) %s: Error! (memory card format failed)\n", index, testCount, test_name);
			psStopMemCardMode();
			return;
		}
	} else if ((result != MEMCARD_STATUS_CONNECTED) && (result != MEMCARD_STATUS_NEW_CARD)) {
		printf("(%d/%d) %s: Error! (memory card not found)\n", index, testCount, test_name);
		psStopMemCardMode();
		return;
	}
	psASCIIToShiftJIS(title, sjis_title, 64);
	psGenerateMemCardHeader(&header, MEMCARD_TYPE_1_FRAME, 1, sjis_title, MEMCARD_TIM4BPP_CLUT(pantsu->data), MEMCARD_TIM4BPP_IMAGE(pantsu->data), NULL, NULL);
	result = psOpenMemCardSave(MEMCARD_CHANNEL_A, file_name, MEMCARD_MODE_READ);
	if (result == MEMCARD_STATUS_NOT_FOUND) {
		if (psCreateMemCardSave(MEMCARD_CHANNEL_A, file_name, 1) != MEMCARD_STATUS_CONNECTED) {
			printf("(%d/%d) %s: Error! (memory card save creation failure)\n", index, testCount, test_name);
			psStopMemCardMode();
			return;
		}
		result = psOpenMemCardSave(MEMCARD_CHANNEL_A, file_name, MEMCARD_MODE_WRITE);
		if (result != MEMCARD_STATUS_CONNECTED) {
			printf("(%d/%d) %s: Error! (memory card open failure)\n", index, testCount, test_name);
			psCloseMemCardSave();
			psStopMemCardMode();
			return;
		}
		result = psWriteMemCardData((unsigned char*)&header, 0, 256);
		if (result != MEMCARD_STATUS_CONNECTED) {
			printf("(%d/%d) %s: Error! (memory card write failure)\n", index, testCount, test_name);
			psCloseMemCardSave();
			psStopMemCardMode();
			return;
		}
		printf("(%d/%d) %s: OK.\n", index, testCount, test_name);
		psCloseMemCardSave();
		psStopMemCardMode();
		return;
	} else if (result != MEMCARD_STATUS_CONNECTED) {
		printf("(%d/%d) %s: Error! (memory card open failure)\n", index, testCount, test_name);
		psStopMemCardMode();
		return;
	}
	result = psReadMemCardData((unsigned char*)&test_header, 0, 256);
	if (result != MEMCARD_STATUS_CONNECTED) {
		printf("(%d/%d) %s: Error! (memory card read failure)\n", index, testCount, test_name);
		psCloseMemCardSave();
		psStopMemCardMode();
		return;
	}
	if (memcmp(&test_header, &header, 256) != 0) {
		printf("(%d/%d) %s: Error! (memory card data failure)\n", index, testCount, test_name);
		psCloseMemCardSave();
		psStopMemCardMode();
		return;
	}
	printf("(%d/%d) %s: OK.\n", index, testCount, test_name);
	psCloseMemCardSave();
	psStopMemCardMode();
}

void test_controllers(char* test_name, int index, int testCount) {
	int stateA = psGetControllerState(CONTROLLER_A);
	int stateB = psGetControllerState(CONTROLLER_B);
	int use = -1;
	if (stateA == CONTROLLER_STATE_CONNECTED) {
		printf("(%d/%d) %s: Info! (controller A found)\n", index, testCount, test_name);
		use = CONTROLLER_A;
	} else {
		printf("(%d/%d) %s: Warning! (controller A not found)\n", index, testCount, test_name);
	}
	if (stateB == CONTROLLER_STATE_CONNECTED) {
		printf("(%d/%d) %s: Info! (controller B found)\n", index, testCount, test_name);
		if (use == -1) {
			use = CONTROLLER_B;
		}
	} else {
		printf("(%d/%d) %s: Warning! (controller B not found)\n", index, testCount, test_name);
	}
	if (use == -1) {
		printf("(%d/%d) %s: Error! (no controllers found)\n", index, testCount, test_name);
		return;
	}
}

int main() {
	int testCount = 3;
	psInit();
	test_cd_reading("Testing CD reading", 1, testCount);
	test_memory_card("Testing Memory Card", 2, testCount);
	test_controllers("Testing controllers", 3, testCount);
	return 0;
}
