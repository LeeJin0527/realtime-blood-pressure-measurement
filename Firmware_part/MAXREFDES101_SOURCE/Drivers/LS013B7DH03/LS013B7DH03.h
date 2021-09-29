/***************************************************************************//**
 * @file LS013B7DH03.h
 * @brief Driver class for the Sharp LS013B7DH03 memory LCD on some kits.
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

#ifndef SILABS_LS013B7DH03_H
#define SILABS_LS013B7DH03_H

#include "platform.h"
#include <mbed.h>
#include "LCDSettings.h"
#include "BufferedDisplay.h"
#include "Peripherals.h"

typedef void (*cbptr_t)(void);

#define LS013B7DH03_ERROR_BUSY		-1
#define LS013B7DH03_ERROR_SPI_BUSY	-2
#define LS013B7DH03_NO_ACTION		-3
#define LS013B7DH03_ERROR_ARGUMENT	-4
#define	LS013B7DH03_OK				0

typedef enum {
	IDLE,			// No operation currently ongoing
	CLEARING,		// In the process of clearing the display
	WRITING,		// In the process of sending a display update
	WAIT_CLEAR,		// Going to clear after CS pin timeout
	WAIT_WRITE,		// Going to write after CS pin timeout
	TRANSFERS_DONE, // Last transfer in progress
	DONE			// Done with transmission, waiting for CS pin to become high
} LS013B7DH03_state_t;

namespace silabs {
class LS013B7DH03 : public BufferedDisplay {

public:

	LS013B7DH03(SPI * spi, DigitalOut * CS,  const char *name=NULL);

	/**
	 * Call this function to push all changes to the display
	 */
	int update( cbptr_t callback = NULL );

	/**
	 * Immediately clear the display: set pixel buffer to zero and clear out the display.
	 */
	int clearImmediate( cbptr_t callback = NULL );

	/**
	 * Function to test display buffer
	 */
	int showDemo();



	/**
	 * Function to get internal refresh counter
	 */
	uint32_t getRefreshTicks();

	/**
	 * Function to get internal refresh counter
	 */
	int executeOnceLCD();


protected:
	mbed::SPI *_spi;
	//mbed::DigitalOut *_EXTCOM;
	mbed::DigitalOut *_CS;

	mbed::Ticker _displayToggler;
	mbed::Timeout _csTimeout;

	event_callback_t _internalEventCallback;
	volatile uint32_t _refreshCount;
	uint8_t _lcdPolarity;
	LS013B7DH03_state_t _state;
	cbptr_t _completionCallbackPtr;
	volatile uint32_t _rowCount;
	uint8_t _cmd[2 + (DISPLAY_WIDTH / DISPLAY_BUFFER_TYPE_SIZE * sizeof(DISPLAY_BUFFER_TYPE))];

	/**
	 * Callback handler for internal SPI transfers.
	 */
	void _cbHandler( int event );

	/**
	 * Callback handler for internal SPI transfers triggered by timeout.
	 */
	void _cbHandlerTimeout( void );

	/**
	 * Call this function at 55 ~ 65 Hz to keep the display from losing contrast.
	 */
	void toggle();

	/**
	 *
	 */
	bool interrupt_received;
};

} // namespace silabs

#endif //SILABS_LS013B7DH03_H
