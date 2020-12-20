#ifndef LCDSETTINGS_H
#define LCDSETTINGS_H

/** MemoryLCD width in pixels */ 
#define DISPLAY_WIDTH				(128)

/** MemoryLCD height in pixels */
#define DISPLAY_HEIGHT				(128)

/** Data type for storing buffer the pixel buffer */
#if	((DISPLAY_WIDTH % 32) == 0)
#define	DISPLAY_BUFFER_TYPE			uint32_t
#define DISPLAY_BUFFER_TYPE_MASK    (0x1F)
#else
#define DISPLAY_BUFFER_TYPE			uint8_t
#define DISPLAY_BUFFER_TYPE_MASK    (0x07)
#endif

#define DISPLAY_BUFFER_TYPE_SIZE	(sizeof(DISPLAY_BUFFER_TYPE) * 8)
#define DISPLAY_BUFFER_ELEMENTS 	((DISPLAY_WIDTH*DISPLAY_HEIGHT)/DISPLAY_BUFFER_TYPE_SIZE)

/** Maximum length of a printf to the display */
#define MAX_PRINTF_CHARS			40

/** Color definitions */
#define White						0xFFFFFFFF
#define Black						0x00000000

#endif
