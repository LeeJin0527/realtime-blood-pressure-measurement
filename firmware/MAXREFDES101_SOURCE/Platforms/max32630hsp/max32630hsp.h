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

#ifndef _MAX32630HSP_H_
#define _MAX32630HSP_H_

#include "mbed.h"
#include "MAX20303.h"


/**
 * @brief MAX32630HSP Board Support Library
 *
 * @details The MAX32630HSP is a rapid development application board for
 * ultra low power wearable applications.  It includes common peripherals and
 * expansion connectors all power optimized for getting the longest life from
 * the battery.  This library configures the power and I/O for the board.
 * <br>https://www.maximintegrated.com/max32630hsp
 *
 * @code
 * #include "mbed.h"
 * #include "max32630hsp.h"
 *
 * DigitalOut led1(LED1);
 * MAX32630HSP icarus(MAX32630HSP::VIO_3V3);
 *
 * // main() runs in its own thread in the OS
 * // (note the calls to Thread::wait below for delays)
 * int main()
 * {
 *     // initialize power and I/O on MAX32630HSP board
 *     icarus.init();
 *
 *     while (true) {
 *         led1 = !led1;
 *         Thread::wait(500);
 *     }
 * }
 * @endcode
 */

// Sharp LS013B7DH03 Memory Display
#define SCK_PIN     		P6_1
#define MOSI_PIN    		P6_2
#define CS_PIN      		P6_5
#define EXTCOM_PIN  		P6_4
#define DISP_PIN    		P6_6
#define DISPSEL_PIN 		NC
#define	PIN_POWERBUTTON		P7_6
#define	PIN_UPBUTTON		P2_3
#define	PIN_DOWNBUTTON		P6_5

class MAX32630HSP
{
public:
// max32630hsp configuration utilities

    /**
      * @brief   IO Voltage
      * @details Enumerated options for operating voltage
      */
    typedef enum {
        VIO_1V8 = 0x00,    ///< 1.8V IO voltage at headers (from BUCK2)
        VIO_3V3 = 0x01,    ///< 3.3V IO voltage at headers (from LDO2)
    } vio_t;

    enum ButtonStatus {
    	BUTTONSTATUS_INACTIVE = 0,
    	BUTTONSTATUS_RELEASED,
		BUTTONSTATUS_PRESSED,
		BUTTONSTATUS_PRESSED_WAITING,
		BUTTONSTATUS_LONG_PRESS,
		BUTTONSTATUS_LONG_PRESS_WAITING,
		BUTTONSTATUS_LONG_PRESS_WAITING_RED,
    } ;

    /**
        * MAX32630HSP constructor.
        *
        */
    MAX32630HSP();

    /**
        * MAX32630HSP constructor.
        *
        */
    MAX32630HSP(vio_t vio);

    /**
        * MAX32630HSP constructor.
        *
        */
    MAX32630HSP(vio_t vio, InterruptIn *max32630hsp3_powerButtonInterrupt);

    /**
        * MAX32630HSP destructor.
        */
    ~MAX32630HSP();

    //InterruptIn _interruptIn_PowerButton;

    /**
     * @brief   Initialize MAX32630HSP board
     * @details Initializes PMIC and I/O on MAX32630HSP board.
     *  Configures PMIC to enable LDO2 and LDO3 at 3.3V.
     *  Disables resisitive pulldown on MON(AIN_0)
     *  Sets default I/O voltages to 3V3 for micro SD card.
     *  Sets I/O voltage for header pins to hdrVio specified.
     * @param hdrVio I/O voltage for header pins
     * @returns 0 if no errors, -1 if error.
    */
    int init(vio_t hdrVio);

    /**
     * @brief   Sets I/O Voltage
     * @details Sets the voltage rail to be used for a given pin.
     *  VIO_1V8 selects VDDIO which is supplied by Buck2, which is set at 1.8V,
     *  VIO_3V3 selects VDDIOH which is supplied by LDO2, which is typically 3.3V/
     * @param   pin Pin whose voltage supply is being assigned.
     * @param   vio Voltage rail to be used for specified pin.
     * @returns 0 if no errors, -1 if error.
    */
    int vddioh(PinName pin, vio_t vio);

    /**Interrupt Hander for Power Button Press**/

    //InterruptIn _interruptIn_UpButton(PIN_UPBUTTON);
    //InterruptIn _interruptIn_DownButton(PIN_DOWNBUTTON);

    /* Set vddio for Sharp LS013B7DH03 Display */
	void enableDisplay(void);

	void Max32630HSP_CheckInterrupt_Status(void);

    /// Local I2C bus for configuring PMIC and accessing BMI160 IMU.
    I2C i2c;

    /// MAX20303 PMIC Instance
    MAX20303 max20303;


    InterruptIn *m_max32630hsp3_powerButtonInterrupt;
    Timeout	button_timeout;

    volatile ButtonStatus status_powerButton;
    ButtonStatus status_upButton;
    ButtonStatus status_downButton;
    void event_powerButtonPressed(void);
    void event_powerButtonReleased(void);
    void event_longpresscheck(void);
};

#endif /* _MAX32630HSP_H_ */
