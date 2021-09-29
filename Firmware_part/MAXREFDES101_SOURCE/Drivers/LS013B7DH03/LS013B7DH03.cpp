/***************************************************************************//**
 * @file LS013B7DH03.cpp
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

#include <mbed.h>
#include "LS013B7DH03.h"
#include "SPI.h"
#include "Peripherals.h"

/* LS013B7DH03 SPI commands */
#define LS013B7DH03_CMD_UPDATE     (0x01)
#define LS013B7DH03_CMD_ALL_CLEAR  (0x04)

/* Macro to switch endianness on char value */
#define SWAP8(a) ((((a) & 0x80) >> 7) | (((a) & 0x40) >> 5) | (((a) & 0x20) >> 3) | (((a) & 0x10) >> 1) | (((a) & 0x08) << 1) | (((a) & 0x04) << 3) | (((a) & 0x02) << 5) | (((a) & 0x01) << 7))

namespace silabs {


LS013B7DH03::LS013B7DH03(mbed::SPI * spi, DigitalOut * CS, const char *name) : BufferedDisplay( name )  {
	//Save pointer to ChipSelect pin
	_CS = CS;
	_CS->write(0);
	DigitalOut DISP(P6_6);

//Save pointer to ExtCom pin
///	_EXTCOM = ExtCom;
///	_EXTCOM->write(0);

	DISP  = 0;
	wait_ms(1);
	DISP = 1;

	//Save pointer to spi peripheral
	_spi = spi;
	//_spi->frequency(600000);
	_spi->format( 8, 0 );

	_internalEventCallback.attach(this, &LS013B7DH03::_cbHandler);

	//Initialize
	//_spi->set_dma_usage((DMAUsage)DMA_USAGE_NEVER);
	_refreshCount = 0;
	_lcdPolarity = 0;
	_state = IDLE;
	_completionCallbackPtr = NULL;
	_rowCount = 0;

	//Start toggling the EXTCOM pin
	//_displayToggler.attach(this, &LS013B7DH03::toggle, 0.008f);
}

/**
 * Call this function at 55 ~ 65 Hz to keep the display up-to-date.
 */
void LS013B7DH03::toggle() {
//	_EXTCOM->write(!_EXTCOM->read());
//	_refreshCount++;
}

/**
 * Function to get internal refresh counter
 */
uint32_t LS013B7DH03::getRefreshTicks() {
	return _refreshCount;
}

/**
 * Call this function to push all changes to the display
 */
int LS013B7DH03::update( cbptr_t callback ) {
	uint32_t rowCount = 0;
	bool update = false;

	// Check if something actually changed in the pixelbuffer
	for(rowCount = 0; rowCount < DISPLAY_HEIGHT/DISPLAY_BUFFER_TYPE_SIZE; rowCount++) {
		if(_dirtyRows[rowCount] != 0) update = true;
	}

	if(update == false) return LS013B7DH03_NO_ACTION;

	// Watch out to not mess up a transfer
	if(_state != IDLE) return LS013B7DH03_ERROR_BUSY;

	_completionCallbackPtr = callback;

	// Take control
	_state = WAIT_WRITE;
	_rowCount = 0;

	//Initialize the command vector
	_cmd[0] = (uint8_t)SWAP8(LS013B7DH03_CMD_UPDATE);
	_cmd[1] = SWAP8(1);

	// Activate LCD
	_CS->write(1);
	_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.01f);

	return LS013B7DH03_OK;
}

/**
 * Function to test display buffer
 */
int LS013B7DH03::showDemo() {
	for(uint32_t i = 0; i < DISPLAY_BUFFER_ELEMENTS; i+=2) {
		_pixelBuffer[i] = 0x00FFF000;
	}
	memset((void*)_dirtyRows, 0xFF, sizeof(_dirtyRows));

	return LS013B7DH03_OK;
}

/**
 * Function to get internal refresh counter
 */
int LS013B7DH03::executeOnceLCD() {
	if(interrupt_received)
		_cbHandler(0);
}

/**
 * Call this function to immediately clear the display
 */
int LS013B7DH03::clearImmediate( cbptr_t callback ) {
	// Watch out to not mess up a transfer
	if(_state != IDLE) return LS013B7DH03_ERROR_BUSY;

	_state = WAIT_CLEAR;
	_completionCallbackPtr = callback;

	// Clear out the pixel buffer
	memset((void*)_pixelBuffer, White, sizeof(_pixelBuffer));
	memset((void*)_dirtyRows, 0, sizeof(_dirtyRows));

	_cmd[0] = (uint8_t)(SWAP8(LS013B7DH03_CMD_ALL_CLEAR | _lcdPolarity));
	_cmd[1] = 0;

	// Wait for the ChipSelect line
	_CS->write(1);
	_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.01f);

	return LS013B7DH03_OK;
}

void LS013B7DH03::_cbHandlerTimeout( void ) {
	interrupt_received = true;
	//this->_cbHandler(0);
}

void LS013B7DH03::_cbHandler( int event ) {
	interrupt_received = false;
	if((_state == WAIT_WRITE) || (_state == WRITING))
	{
		_state = WRITING;
		while(_rowCount < DISPLAY_HEIGHT) {
			// Determine the next line to send
			if((_dirtyRows[_rowCount / DISPLAY_BUFFER_TYPE_SIZE] & (1 << (_rowCount % DISPLAY_BUFFER_TYPE_SIZE))) != 0) {

				// Row is dirty, send an update to the display
				_cmd[1] = (uint8_t)SWAP8(_rowCount + 1);
				memcpy((void*)&(_cmd[2]), (const void*)&(_pixelBuffer[_rowCount*(DISPLAY_WIDTH/DISPLAY_BUFFER_TYPE_SIZE)]), DISPLAY_WIDTH / DISPLAY_BUFFER_TYPE_SIZE * sizeof(DISPLAY_BUFFER_TYPE));

				if(_spi->write((const char*)_cmd, (2 + (DISPLAY_WIDTH / DISPLAY_BUFFER_TYPE_SIZE * sizeof(DISPLAY_BUFFER_TYPE))) , (char*)NULL, 0/*, _internalEventCallback, SPI_EVENT_COMPLETE*/) != (2 + (DISPLAY_WIDTH / DISPLAY_BUFFER_TYPE_SIZE * sizeof(DISPLAY_BUFFER_TYPE)))) {
					// SPI is busy, with another transaction. This means the data to the LCD has been corrupted, so fail here.
					_state = DONE;
					//printf("Failed at _cbHandler\n");
					// Make sure the handler is called again
					_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.1f);
				}else{	//sc...
					_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.001f);
				}

				// Transaction is in progress, so update row state
				_dirtyRows[_rowCount / DISPLAY_BUFFER_TYPE_SIZE] &= ~(1 << (_rowCount % DISPLAY_BUFFER_TYPE_SIZE));
				_rowCount++;
				return;
			}

			// Row wasn't touched, so check the next row
			_rowCount++;
		}

		// Done sending!
		_cmd[1] = 0xFF;
		_state = TRANSFERS_DONE;
		if(_spi->write((const char*)_cmd, 2, (char*)NULL, 0/*, _internalEventCallback, SPI_EVENT_COMPLETE*/) != 2) {
			// SPI is busy, with another transaction. This means the data to the LCD has been corrupted, so fail here.
			_state = DONE;

			// Make sure the handler is called again
			_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.1f);
		}else{	//sc...
			_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.001f);
		}
		return;
	}
	else if (_state == WAIT_CLEAR)
	{
		_state = TRANSFERS_DONE;
		if(_spi->write((const char*)_cmd, 2, (char*)NULL, 0/*, _internalEventCallback, SPI_EVENT_COMPLETE*/) != 2) {
			// SPI is busy, with another transaction. This means the data to the LCD has been corrupted, so fail here.
			_state = DONE;

			// Make sure the handler is called again
			_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.1f);
		}else{	//sc...
			_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.001f);
		}
		return;
	}
	else if (_state == TRANSFERS_DONE)
	{
		_state = DONE;
		_csTimeout.attach(this, &LS013B7DH03::_cbHandlerTimeout, 0.01f);
		return;
	}
	else if (_state == DONE)
	{
		_CS->write(0);
		_state = IDLE;
		if(_completionCallbackPtr != 0) _completionCallbackPtr();
		return;
	}
}

} // namespace silabs
