/*******************************************************************************
* Author: Shaun Kelsey, shaun.kelsey@maximintegrated.com
* Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/

#ifndef _LEDSTATUS_H_
#define _LEDSTATUS_H_
#include "mbed.h"

class LEDStatus
{
public:

	/** Create an LED_Status controller
	 * 
	 *  @param red PinName of red LED
	 *  @param rstate Initial state of red LED
	 *  @param green PinName of green LED
	 *  @param gstate Initial state of green LED
	 *  @param blue PinName of blue LED
	 *  @param bstate Initial state of blue LED
	 */
	LEDStatus(PinName red, int rstate, PinName green, int gstate, PinName blue, int bstate);

	/** Set the active state of each LED
	 *
	 *  @param rstate active state of red LED
	 *  @param gstate active state of green LED
	 *  @param bstate active state of blue LED
	 */
	void set_state(int rstate, int gstate, int bstate);

	/** Set and keep the LEDs in their active state */
	void solid();

	/** Set the LEDs to toggle between their active state and their off state
	 *
	 *  @param ontime The amount of time (in milliseconds) to remain in the active state
	 *  @param offtime The amount of time (in milliseconds) to remain off
	 *  @param nb The number of times to blink before remaining in the off state
	 *  				Set to -1 to blink indefinitely
	 *
	 */
	void blink(int ontime, int offtime, int nb = -1);

	/** Set the LEDs to toggle between their active state and their off state
	 *
	 *  @param period The period of each blink (in milliseconds)
	 *  @param offtime The percentage of the period during which the LEDs will be in their active state
	 *  @param nb The number of times to blink before remaining in the off state
	 *  				Set to -1 to blink indefinitely
	 */
	void blink(int period, float duty, int nb = -1);

	/** Get whether LEDState is in the blinking state
	 */
	bool is_blinking();

	/** Called in main loop of program to advance the state of the class
	 *  @detail Instead of using a Ticker and generating additional interrupts and overhead,
	 *  		LEDStatus relies on update being called at a decent interval in order
	 *  		to advance its interval time keeping and blink LEDs and the correct rate.
	 *  		If only using solid() mode, update() does not need to be called.
	 */
	void update();

private:

	DigitalOut rLED;
	DigitalOut gLED;
	DigitalOut bLED;

	int r_act;
	int g_act;
	int b_act;

	bool blinking;
	bool bs;
	int count;
	int total;

	uint64_t ont_us;
	uint64_t offt_us;

	Timer timer;

};

#endif

