/***************************************************************************//**
 * @file BufferedDisplay.h
 * @brief Framebuffered version of GraphicsDisplay
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

#ifndef SILABS_BUFFEREDDISPLAY_H
#define SILABS_BUFFEREDDISPLAY_H

#include "GraphicsDisplay.h"
#include "LCDSettings.h"

namespace silabs {
/** Framebuffered version of GraphicsDisplay
 * 
 * This has been implemented as part of the MemoryLCD library.
 */
class BufferedDisplay : public GraphicsDisplay {

public:

	BufferedDisplay(const char *name=NULL);

	/**
	 * Override of GraphicsDisplay pixel() function to set a pixel in the buffer
     *
     * @param x      Zero-based x-axis index of pixel to set. 0 = leftmost.
     * @param y      Zero-based y-axis index of pixel to set. 0 = topmost.
     * @param colour Colour value to set pixel to. In this implementation, only LSB is taken into account.
	 */
	virtual void pixel(int x, int y, int colour);
	virtual int width();
	virtual int height();

	/**
	 * Function to move bitmap into frame buffer
	 * 
	 * @param bitmap      pointer to uint8 array containing horizontal pixel data
	 * @param bmpWidth    width of the bitmap in pixels (must be multiple of 8)
	 * @param bmpHeight   height of the bitmap in pixels
	 * @param startX      starting position to apply bitmap in horizontal direction (0 = leftmost) (must be multiple of 8)
	 * @param startY      starting position to apply bitmap in vertical direction (0 = topmost)
	 */
	void showBMP(const uint8_t* bitmap, const uint32_t bmpWidth, const uint32_t bmpHeight, const uint32_t startX, const uint32_t startY);

protected:
	volatile DISPLAY_BUFFER_TYPE _pixelBuffer[DISPLAY_BUFFER_ELEMENTS]; // one full frame buffer
	volatile DISPLAY_BUFFER_TYPE _dirtyRows[DISPLAY_HEIGHT/DISPLAY_BUFFER_TYPE_SIZE]; // 1 bit per row to indicate dirty status
};

} // namespace silabs




#endif //SILABS_BUFFEREDDISPLAY_H
