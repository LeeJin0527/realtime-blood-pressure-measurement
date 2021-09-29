/** 128*128 1bpp version of the mbed-enabled logo */
#if 0
extern const unsigned char mbed_enabled_logo[];
#endif
extern const unsigned char maxim_integrated_logo[];
extern const unsigned char maxim128Bitmaps[];
extern const unsigned char BLELogoBitmap[];
extern const unsigned char USBLogoBitmap[];

#ifndef MBED_LOGO_H
#define MBED_LOGO_H

typedef struct
{
	const unsigned char width;
	const unsigned char height;
	const unsigned short offset;

} FONT_CHAR_INFO;

typedef struct
{
	const unsigned char height;
	const char startCh;
	const char endCh;
	const FONT_CHAR_INFO*	charDesc;
	const unsigned char* bitmap;

} FONT_INFO;

// Font data for Ubuntu Condensed 28pt
extern const unsigned char ubuntuCondensed_28ptBitmaps[];
extern const FONT_INFO ubuntuCondensed_28ptFontInfo;
extern const FONT_CHAR_INFO ubuntuCondensed_28ptDescriptors[];
// Bitmap info for maxim128

//extern const unsigned char maxim128WidthPages;
//extern const unsigned char maxim128HeightPixels;

// Font data for Microsoft Sans Serif 8pt
extern const unsigned char AM_Bitmap[];
extern const unsigned char PM_Bitmap[];


#endif // MBED_LOGO_H
