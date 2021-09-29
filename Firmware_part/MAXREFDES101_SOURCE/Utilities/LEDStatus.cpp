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

#include "LEDStatus.h"
#include "Peripherals.h"

LEDStatus::LEDStatus(PinName red, int rstate, PinName green, int gstate, PinName blue, int bstate):
	rLED(red, rstate), gLED(green, gstate), bLED(blue, bstate), blinking(false), timer()
{
}

void LEDStatus::set_state(int rstate, int gstate, int bstate)
{
	r_act = rstate;
	g_act = gstate;
	b_act = bstate;

	if (!blinking) {
		rLED = r_act;
		gLED = g_act;
		bLED = b_act;
	}
}

void LEDStatus::solid()
{
	blinking = false;
	rLED = r_act;
	gLED = g_act;
	bLED = b_act;

	timer.stop();
}

void LEDStatus::blink(int ontime, int offtime, int nb)
{
	ont_us = 1000 * ontime;
	offt_us = 1000 * offtime;
	count = 1;
	total = nb;
	blinking = true;

	rLED = r_act;
	gLED = g_act;
	bLED = b_act;
	bs = true;

	timer.reset();
	timer.start();
}

void LEDStatus::blink(int period, float duty, int nb)
{
	ont_us = 1000 * period * duty;
	offt_us = (1000 * period) - ont_us;
	count = 1;
	total = nb;
	blinking = true;

	rLED = r_act;
	gLED = g_act;
	bLED = b_act;
	bs = true;

	timer.reset();
}

bool LEDStatus::is_blinking()
{
	return blinking;
}

void LEDStatus::update()
{
	uint64_t time = timer.read_high_resolution_us();
	uint64_t frame_time = time % (ont_us + offt_us);

	if (bs && (frame_time > ont_us)) {
		bs = false;
		rLED = LED_OFF;
		gLED = LED_OFF;
		bLED = LED_OFF;
		count++;

	} else if (!bs && (frame_time <= ont_us)) {
		bs = true;
		rLED = r_act;
		gLED = g_act;
		bLED = b_act;

		if (total > 0 && count >= total) {
			blinking = false;
			timer.stop();
		}
	}
}

