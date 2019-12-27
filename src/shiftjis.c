#include "shiftjis.h"

unsigned short code_table[] = {
	0x8140, 0x8149, 0x8168, 0x8194, 0x8190, 0x8193, 0x8195, 0x8166, 0x8169, 0x816A, 0x8196, 0x817B, 0x8143, 0x817C, 0x8144, 0x815E, 0x824F, 0x8250, 0x8251, 0x8252, 0x8253, 0x8254, 0x8255, 0x8256, 0x8257, 0x8258, 0x8146, 0x8147, 0x8171, 0x8181, 0x8172, 0x8148, 0x8197, 0x8260, 0x8261, 0x8262, 0x8263, 0x8264, 0x8265, 0x8266, 0x8267, 0x8268, 0x8269, 0x826A, 0x826B, 0x826C, 0x826D, 0x826E, 0x826F, 0x8270, 0x8271, 0x8272, 0x8273, 0x8274, 0x8275, 0x8276, 0x8277, 0x8278, 0x8279, 0x816D, 0x818F, 0x816E, 0x814F, 0x8151, 0x8165, 0x8281, 0x8282, 0x8283, 0x8284, 0x8285, 0x8286, 0x8287, 0x8288, 0x8289, 0x828A, 0x828B, 0x828C, 0x828D, 0x828E, 0x828F, 0x8290, 0x8291, 0x8292, 0x8293, 0x8294, 0x8295, 0x8296, 0x8297, 0x8298, 0x8299, 0x829A, 0x816F, 0x8162, 0x8170, 0x8150
};

char inv_code_table_1[] = {
	' ', '\0', '\0', ',', '.', '\0', ':', ';', '?', '!', '\0', '\0', '\0', '\0', '\0', '^', '~', '_'
};

char inv_code_table_2[] = {
	'/', '\0', '\0', '\0', '|', '\0', '\0', '`', '\'', '\0', '"', '(', ')', '\0', '\0', '[', ']', '{', '}', '<', '>'
};

char inv_code_table_3[] = {
	'\\', '$', '\0', '\0', '%', '#', '&', '*', '@'
};

unsigned short ascii2sjis(unsigned char character) {
	if ((character >= 0x20) && (character <= 0x7E)) {
		return code_table[character - 0x20];
	}
	return 0;
}

unsigned char sjis2ascii(unsigned short character) {
	if (character >= 0x8281) {
		if (character <= 0x829A) {
			return (unsigned char)'a' + (character - 0x8281);
		}
		return 0;
	} else if (character >= 0x8260) {
		if (character <= 0x8279) {
			return (unsigned char)'A' + (character - 0x8260);
		}
		return 0;
	} else if (character >= 0x824F) {
		if (character <= 0x8258) {
			return (unsigned char)'0' + (character - 0x824F);
		}
		return 0;
	} else if (character >= 0x818F) {
		if (character <= 0x8197) {
			return (unsigned char)(inv_code_table_3[character - 0x818F]);
		}
		return 0;
	} else if (character >= 0x817B) {
		switch (character) {
		case 0x817B:
			return (unsigned char)'+';
		case 0x817C:
			return (unsigned char)'-';
		case 0x8181:
			return (unsigned char)'=';
		}
		return 0;
	} else if (character >= 0x815E) {
		if (character <= 0x8172) {
			return (unsigned char)(inv_code_table_2[character - 0x815E]);
		}
		return 0;
	} else if ((character >= 0x8140) && (character <= 0x8151)) {
		return (unsigned char)(inv_code_table_1[character - 0x8140]);
	}
	return 0;
}
