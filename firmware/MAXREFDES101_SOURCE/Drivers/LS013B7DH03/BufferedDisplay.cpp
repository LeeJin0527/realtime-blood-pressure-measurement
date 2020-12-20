/***************************************************************************//**
 * @file BufferedDisplay.cpp
 * @brief Buffered version of GraphicsDisplay
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

#include "BufferedDisplay.h"

#define SWAP8(a) ((((a) & 0x80) >> 7) | (((a) & 0x40) >> 5) | (((a) & 0x20) >> 3) | (((a) & 0x10) >> 1) | (((a) & 0x08) << 1) | (((a) & 0x04) << 3) | (((a) & 0x02) << 5) | (((a) & 0x01) << 7))

namespace silabs {

	BufferedDisplay::BufferedDisplay(const char *name) : GraphicsDisplay(name) {
		memset((uint8_t*)_pixelBuffer, White, sizeof(_pixelBuffer));	// init full frame buffer
		memset((uint8_t*)_dirtyRows, 0xFF, sizeof(_dirtyRows)); 		// init dirty status
	}

	/**
	 * Override of GraphicsDisplay's pixel()
	 */

	void BufferedDisplay::pixel(int x, int y, int colour) {
	    /* Apply constraint to x and y */
	    if(x < 0 || y < 0) return;
	    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
	    
	    /***************************************************************************************************************** 
	     * The display expects LSB input, while the SPI is configured for 8bit MSB transfers. Therefore, we should  
	     * construct the framebuffer accordingly, so that an MSB transmission will put pixel 0 first on the wire.
	     *
	     * So the actual pixel layout in framebuffer (for 128x128) is as follows:
	     * {                                                    //Framebuffer
	     *	{                                                   //Line 0
	     *	 {p0, p1, p2, p3, p4, p5, p6, p7},                  //Line 0 byte 0 (byte 0)
	     *   {p8, p9,p10,p11,p12,p13,p14,p15},                  //Line 0 byte 1 (byte 1)
	     *   ...
	     *   {p120,p121,p122,p123,p124,p125,p126,p127}          //Line 0 byte 15 (byte 15)
	     *  },        
	     *  {													//Line 1
	     *	 {p128,p129,p130,p131,p132,p133,p134,p135},         //Line 1 byte 0 (byte 16)
	     *   ...
	     *  },
	     *  ...
	     *  {													//Line 127
	     *	 {...},              								//Line 127 byte 0 (byte 2032)
	     *   ...
	     *   {...}												//Line 127 byte 15 (byte 2047) = 128*128 bits
	     *	}
	     * }
	     *
	     * This means that to calculate the actual bit position in the framebuffer byte, we need to swap the bit 
	     * order of the lower three bits. So pixel 7 becomes bit offset 0, 6 -> 1, 5 -> 2, 4->3, 3->4, 2->5, 1->6 and 0->7.
	     *****************************************************************************************************************/
	    uint8_t swapx = 7 - ((unsigned int)x & 0x07);
	    x = ((unsigned int)x & 0xFFFFFFF8) | swapx;

	    /* Since we are dealing with 1-bit pixels, we can avoid having to do bitshift and comparison operations twice.
	     * Basically, do the comparison with the requested state and current state, and if it changed, do an XOR on the framebuffer pixel and set the line to dirty.
	     */
	    bool change = ((_pixelBuffer[((y * DISPLAY_WIDTH) + x) / DISPLAY_BUFFER_TYPE_SIZE] & (1 << (x & DISPLAY_BUFFER_TYPE_MASK))) != ((colour & 0x01) << (x & DISPLAY_BUFFER_TYPE_MASK)));
		if(change) {
			/* Pixel's value and requested value are different, so since it's binary, we can simply do an XOR */
            _pixelBuffer[((y * DISPLAY_WIDTH) + x) / DISPLAY_BUFFER_TYPE_SIZE] ^= (1 << (x & DISPLAY_BUFFER_TYPE_MASK));

            /* notify dirty status of this line */
            _dirtyRows[y / DISPLAY_BUFFER_TYPE_SIZE] |= (1 << (y & DISPLAY_BUFFER_TYPE_MASK));
		}
	}

	int BufferedDisplay::width() {
		return DISPLAY_WIDTH;
	}
	int BufferedDisplay::height() {
		return DISPLAY_HEIGHT;
	}

	/**
	 * Function to move bitmap into frame buffer
	 * arguments:
	 * 	* bitmap: pointer to uint8 array containing horizontal pixel data
	 * 	* bmpWidth: width of the bitmap in pixels (must be multiple of 8)
	 * 	* bmpHeight: height of the bitmap in pixels
	 * 	* startX: starting position to apply bitmap in horizontal direction (0 = leftmost) (must be multiple of 8)
	 * 	* startY: starting position to apply bitmap in vertical direction (0 = topmost)
	 */
	void BufferedDisplay::showBMP(const uint8_t* bitmap, const uint32_t bmpWidth, const uint32_t bmpHeight, const uint32_t startX, const uint32_t startY) {
		uint32_t bmpLine = 0, y = startY, bytesPerLine = ((bmpWidth >= (DISPLAY_WIDTH - startX)) ? (DISPLAY_WIDTH - startX) : bmpWidth) / 8;

		/* Apply constraints */
		if((bmpWidth & 0x07) != 0) return;
		if((startX & 0x07) != 0) return;
		if(startX >= DISPLAY_WIDTH) return;
		
		//Superflouous due to for-loop check
		//if((startY >= DISPLAY_HEIGHT) return;

		/* Copy over bytes to the framebuffer, do not write outside framebuffer boundary */
		for(; y < DISPLAY_HEIGHT; y++) {
			/* Check that we are not writing more than the BMP height */
			if(bmpLine >= bmpHeight) break;
			
			/* Copy over one line (bmpLine) from the BMP file to the corresponding line (y) in the pixel buffer */
			memcpy( (void*) &(((uint8_t*)_pixelBuffer)[((y * DISPLAY_WIDTH) + startX) / 8]),
					(const void*) &(bitmap[bmpLine * (bmpWidth / 8)]),
					bytesPerLine);

			/* Set dirty status for the line we just overwrote */
			_dirtyRows[y / DISPLAY_BUFFER_TYPE_SIZE] |= (1 << (y % DISPLAY_BUFFER_TYPE_SIZE));
			bmpLine++;
		}

		return;
	}
}
