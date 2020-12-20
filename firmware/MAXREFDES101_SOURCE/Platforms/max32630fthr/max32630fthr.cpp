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
#include "max32630fthr.h"

//******************************************************************************
MAX32630FTHR::MAX32630FTHR() : i2c(P5_7, P6_0), max14690(&i2c)
{
}

//******************************************************************************
MAX32630FTHR::MAX32630FTHR(vio_t vio) : i2c(P5_7, P6_0), max14690(&i2c)
{
    init(vio);
}

//******************************************************************************
MAX32630FTHR::~MAX32630FTHR()
{
}

//******************************************************************************
int MAX32630FTHR::init(vio_t hdrVio)
{
    // Override the default values
    max14690.ldo2Millivolts = 3300;
    max14690.ldo3Millivolts = 3300;
    max14690.ldo2Mode = MAX14690::LDO_ENABLED;
    max14690.ldo3Mode = MAX14690::LDO_ENABLED;
    max14690.monCfg = MAX14690::MON_HI_Z;
    // Note that writing the local value does directly affect the part
    // The buck-boost regulator will remain off until init is called

    // Call init to apply all settings to the PMIC
    if (max14690.init() == MAX14690_ERROR) {
        printf("Error initializing MAX14690");
    }

    // Set micro SD card pins to 3.3V
    vddioh(P0_4, VIO_3V3);
    vddioh(P0_5, VIO_3V3);
    vddioh(P0_6, VIO_3V3);
    vddioh(P0_7, VIO_3V3);
    // Set LED pins to 3.3V
    vddioh(P2_4, VIO_3V3);
    vddioh(P2_5, VIO_3V3);
    vddioh(P2_6, VIO_3V3);
    // Set header pins to hdrVio
    vddioh(P3_0, hdrVio);
    vddioh(P3_1, hdrVio);
    vddioh(P3_2, hdrVio);
    vddioh(P3_3, hdrVio);
    vddioh(P3_4, hdrVio);
    vddioh(P3_5, hdrVio);
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
int MAX32630FTHR::vddioh(PinName pin, vio_t vio)
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
