#ifndef __SHIFTJIS_H__
#define __SHIFTJIS_H__

/* convert ASCII character to Shift-JIS */
extern unsigned short ascii2sjis(unsigned char);

/* convert Shift-JIS character to ASCII */
extern unsigned char sjis2ascii(unsigned short);

#endif
