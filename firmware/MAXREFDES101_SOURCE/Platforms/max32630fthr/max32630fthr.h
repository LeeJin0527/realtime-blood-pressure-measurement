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

#ifndef _MAX32630FTHR_H_
#define _MAX32630FTHR_H_

#include "mbed.h"
#include "MAX14690.h"

/**
 * @brief MAX32630FTHR Board Support Library
 *
 * @details The MAX32630FTHR is a rapid development application board for
 * ultra low power wearable applications.  It includes common peripherals and
 * expansion connectors all power optimized for getting the longest life from
 * the battery.  This library configures the power and I/O for the board.
 * <br>https://www.maximintegrated.com/max32630fthr
 *
 * @code
 * #include "mbed.h"
 * #include "max32630fthr.h"
 *
 * DigitalOut led1(LED1);
 * MAX32630FTHR pegasus(MAX32630FTHR::VIO_3V3);
 *
 * // main() runs in its own thread in the OS
 * // (note the calls to Thread::wait below for delays)
 * int main()
 * {
 *     // initialize power and I/O on MAX32630FTHR board
 *     pegasus.init();
 *
 *     while (true) {
 *         led1 = !led1;
 *         Thread::wait(500);
 *     }
 * }
 * @endcode
 */
class MAX32630FTHR
{
public:
// max32630fthr configuration utilities

    /**
      * @brief   IO Voltage
      * @details Enumerated options for operating voltage
      */
    typedef enum {
        VIO_1V8 = 0x00,    ///< 1.8V IO voltage at headers (from BUCK2)
        VIO_3V3 = 0x01,    ///< 3.3V IO voltage at headers (from LDO2)
    } vio_t;

    /**
        * MAX32630FTHR constructor.
        *
        */
    MAX32630FTHR();

    /**
        * MAX32630FTHR constructor.
        *
        */
    MAX32630FTHR(vio_t vio);

    /**
        * MAX32630FTHR destructor.
        */
    ~MAX32630FTHR();

    /**
     * @brief   Initialize MAX32630FTHR board
     * @details Initializes PMIC and I/O on MAX32630FTHR board.
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

    /// Local I2C bus for configuring PMIC and accessing BMI160 IMU.
    I2C i2c;

    /// MAX14690 PMIC Instance
    MAX14690 max14690;

};

#endif /* _MAX32630FTHR_H_ */
