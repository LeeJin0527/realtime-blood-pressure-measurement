/*******************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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

#include "mbed.h"
#include "max3263x.h"
#include "ioman_regs.h"
#include "PinNames.h"
#include "max32630hsp.h"


//******************************************************************************
MAX32630HSP::MAX32630HSP() : i2c(P5_7, P6_0), max20303(&i2c)
{
}

//******************************************************************************
MAX32630HSP::MAX32630HSP(vio_t vio) : i2c(P5_7, P6_0), max20303(&i2c)
{

    init(vio);
}

//******************************************************************************
MAX32630HSP::MAX32630HSP(vio_t vio, InterruptIn *max32630hsp3_powerButtonInterrupt) : i2c(P5_7, P6_0), max20303(&i2c), m_max32630hsp3_powerButtonInterrupt(max32630hsp3_powerButtonInterrupt)
{
    init(vio);

    status_powerButton = BUTTONSTATUS_INACTIVE;
    m_max32630hsp3_powerButtonInterrupt->disable_irq();
    m_max32630hsp3_powerButtonInterrupt->rise(this, &MAX32630HSP::event_powerButtonReleased);
    m_max32630hsp3_powerButtonInterrupt->mode(PullUp);
	m_max32630hsp3_powerButtonInterrupt->enable_irq();
}


//******************************************************************************
MAX32630HSP::~MAX32630HSP()
{

}


//******************************************************************************
int MAX32630HSP::init(vio_t hdrVio)
{
	/* Wait for pmic to settle down */
	wait_ms(800);

    set_time(1544787300);  // Set RTC time to Wed, 28 Oct 2009 11:35:37

	/*Set LDO1 to 1.8v*/
	max20303.LDO1Config();

	/*Set LDO2 to 3v*/
    max20303.LDO2Config();

	//max20303.BoostEnable();
	max20303.BuckBoostEnable();


	max20303.led0on(0);
	max20303.led1on(0);
	max20303.led2on(0);

	/* Wait for pmic to settle down */
    wait_ms(200);

    // Set LED pins to 3.3V
    vddioh(P2_4, VIO_3V3);
    vddioh(P2_5, VIO_3V3);
    vddioh(P2_6, VIO_3V3);

    // set i2c pins to 1.8V
    vddioh(P3_4, VIO_1V8);
    vddioh(P3_5, VIO_1V8);
    //ble module pins to 1.8V
    vddioh(P0_0, VIO_1V8);
    vddioh(P0_1, VIO_1V8);
    vddioh(P0_2, VIO_1V8);
    vddioh(P0_3, VIO_1V8);

    // Set header pins to hdrVio
    vddioh(P3_0, hdrVio);
    vddioh(P3_1, hdrVio);
    vddioh(P3_2, hdrVio);
    vddioh(P3_3, hdrVio);
    vddioh(P4_0, hdrVio);
    vddioh(P4_1, hdrVio);
    vddioh(P4_2, hdrVio);
    vddioh(P4_3, hdrVio);
    vddioh(P4_4, hdrVio);
    vddioh(P4_5, hdrVio);
    vddioh(P4_6, hdrVio);
    vddioh(P4_7, hdrVio);
    vddioh(P5_0, hdrVio);
    vddioh(P5_1, hdrVio);
    vddioh(P5_2, hdrVio);
    vddioh(P5_3, hdrVio);
    vddioh(P5_4, hdrVio);
    vddioh(P5_5, hdrVio);
    vddioh(P5_6, hdrVio);



    return 0;
}

//******************************************************************************
void MAX32630HSP::event_powerButtonPressed(void) {
	status_powerButton = BUTTONSTATUS_PRESSED;
}

//******************************************************************************
void MAX32630HSP::event_powerButtonReleased(void) {
	if(status_powerButton == BUTTONSTATUS_LONG_PRESS_WAITING_RED) {
		status_powerButton = BUTTONSTATUS_LONG_PRESS;
	} else {
		status_powerButton = BUTTONSTATUS_RELEASED;
	}
}

//******************************************************************************
void MAX32630HSP::event_longpresscheck(void) {
	status_powerButton = BUTTONSTATUS_LONG_PRESS_WAITING;
}

//******************************************************************************
void MAX32630HSP::enableDisplay(void)
{
	vddioh(P6_4, VIO_3V3); //EXTCOM
	vddioh(P6_1, VIO_3V3); //SCLK
	vddioh(P6_2, VIO_3V3); //MOSI1
	vddioh(P6_5, VIO_3V3); //SCS
	vddioh(P6_4, VIO_3V3); //EXTCOM
	vddioh(P6_6, VIO_3V3); //DISP
}

//******************************************************************************
void MAX32630HSP::Max32630HSP_CheckInterrupt_Status(void) {

	switch (status_powerButton) {
	case BUTTONSTATUS_PRESSED: {
		max20303.led0on(0);
		max20303.led1on(0);
		max20303.led2on(1);
		/* Button press detected. Wait for button release */
		m_max32630hsp3_powerButtonInterrupt->disable_irq();
		m_max32630hsp3_powerButtonInterrupt->rise(this, &MAX32630HSP::event_powerButtonReleased);
		m_max32630hsp3_powerButtonInterrupt->mode(PullUp);
		m_max32630hsp3_powerButtonInterrupt->enable_irq();
		/* Button press detected. Start Timeout object for checking long key press event */
		button_timeout.attach( this, &MAX32630HSP::event_longpresscheck , 2.0 );
		/* Button is pressed */
		status_powerButton = BUTTONSTATUS_PRESSED_WAITING;

	} break;
	case BUTTONSTATUS_RELEASED: {
		/* Button is released. Stop timeout object */
		button_timeout.detach();
		max20303.led0on(0);
		max20303.led1on(0);
		max20303.led2on(0);
		/* Button is released. Expect for button press event next time */
		m_max32630hsp3_powerButtonInterrupt->disable_irq();
		m_max32630hsp3_powerButtonInterrupt->fall(this, &MAX32630HSP::event_powerButtonPressed);
		m_max32630hsp3_powerButtonInterrupt->mode(PullUp);
		m_max32630hsp3_powerButtonInterrupt->enable_irq();
		status_powerButton = BUTTONSTATUS_INACTIVE;
	} break;
	case BUTTONSTATUS_LONG_PRESS_WAITING: {
		max20303.led0on(0);
		max20303.led1on(1);
		max20303.led2on(0);
		status_powerButton = BUTTONSTATUS_LONG_PRESS_WAITING_RED;
	} break;
	case BUTTONSTATUS_LONG_PRESS: {
		max20303.PowerOffDelaythePMIC();
		wait_ms(250);
		status_powerButton = BUTTONSTATUS_INACTIVE;
	} break;
	default:
		break;
	}
}

//******************************************************************************
int MAX32630HSP::vddioh(PinName pin, vio_t vio)
{
    __IO uint32_t *use_vddioh = &((mxc_ioman_regs_t *)MXC_IOMAN)->use_vddioh_0;

    if (pin == NOT_CONNECTED) {
        return -1;
    }

    use_vddioh += PINNAME_TO_PORT(pin) >> 2;
    if (vio) {
        *use_vddioh |= (1 << (PINNAME_TO_PIN(pin) + ((PINNAME_TO_PORT(pin) & 0x3) << 3)));
    } else {
        *use_vddioh &= ~(1 << (PINNAME_TO_PIN(pin) + ((PINNAME_TO_PORT(pin) & 0x3) << 3)));
    }

    return 0;
}
