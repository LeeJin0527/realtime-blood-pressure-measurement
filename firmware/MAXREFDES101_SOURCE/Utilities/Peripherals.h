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
#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

#include "mbed.h"
#include "Serial.h"
#include "USBSerial.h"


#include "MAX30001.h"
#include "MAX30001_Helper.h"


#define pr_USB(fmt, args...) if(Peripherals::usbSerial() != NULL) \
	Peripherals::usbSerial()->printf("\r\n[" fmt "[(%s:%d)\r\n", ##args, __func__, __LINE__)

#define pr_SERIAL(fmt, args...) printf("\r\n[" fmt "[(%s:%d)\r\n", ##args, __func__, __LINE__)

#define pr_err(fmt, args...) if(Peripherals::usbSerial() != NULL) \
	Peripherals::usbSerial()->printf("\r\n[" fmt "[(%s:%d)\r\n", ##args, __func__, __LINE__); \
	if (Peripherals::daplinkSerial() != NULL) \
		Peripherals::daplinkSerial()->printf("\r\n[" fmt, ##args);

#ifdef SHOW_DEBUG_MSGS
#define pr_debug(fmt, args...) 	if(Peripherals::usbSerial() != NULL) \
	Peripherals::usbSerial()->printf("\r\n[" fmt "[(%s:%d)\r\n", ##args, __func__, __LINE__); \
	if (Peripherals::daplinkSerial() != NULL) \
		Peripherals::daplinkSerial()->printf("\r\n[" fmt, ##args);
#else
	#define pr_debug(fmt, args...)
#endif

#ifdef SHOW_INFO_MSGS
#define pr_info(fmt, args...) if(Peripherals::usbSerial() != NULL)\
	Peripherals::usbSerial()->printf("[" fmt, ##args); \
	if (Peripherals::daplinkSerial() != NULL)\
		Peripherals::daplinkSerial()->printf("\r\n[" fmt, ##args);
#else
	#define pr_info(fmt, args...)
#endif

/**
* This static class is used as a central locatoin for all devices on the HSP platform
* it gives (in-effect) a singleton interface for each device so that anywhere in code
* one can reference on of these devices
*/
class Peripherals {
public:
    static USBSerial *setUSBSerial(USBSerial * device) { mUSBSerial = device; return device; }
    static USBSerial *usbSerial(void) { return mUSBSerial; }

    static Serial *setDaplinkSerial(Serial * device) { mDaplinkSerial = device; return device; }
    static Serial *daplinkSerial(void) { return mDaplinkSerial; }


    static MAX30001 *max30001(void) { return mMAX30001; }
    static MAX30001 *setMAX30001(MAX30001 *device) { mMAX30001 = device; return device; }

    static MAX30001_Helper *max30001Helper(void) { return mMAX30001_Helper; }
    static MAX30001_Helper *setMAX30001Helper(MAX30001_Helper *device) { mMAX30001_Helper = device; return device; }


private:
    static USBSerial *mUSBSerial;
    static Serial *mDaplinkSerial;

    static MAX30001 *mMAX30001;
    static MAX30001_Helper *mMAX30001_Helper;

};

#endif // _PERIPHERALS_H_

