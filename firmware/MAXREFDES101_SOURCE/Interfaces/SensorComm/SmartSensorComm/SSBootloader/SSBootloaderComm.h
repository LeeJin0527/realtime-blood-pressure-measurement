/*******************************************************************************
 * Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
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

#ifndef _SSBOOTLOADERCOMM_H_
#define _SSBOOTLOADERCOMM_H_

#include "mbed.h"
#include "SensorComm.h"
#include "USBSerial.h"
#include "SSInterface.h"

#define SS_BOOTLOADER_ERASE_DELAY	2000

#define PAGE_WRITE_DELAY_MS         700
#define MAX_PAGE_NUMBER             31

/**
 * @brief	SSBootloaderComm Command handler class for bootloader routines on SmartSensor
 * @details
 */
class SSBootloaderComm:	public SensorComm
{
public:

	/* PUBLIC FUNCTION DECLARATIONS */
	/**
	* @brief	SSBootloaderComm constructor.
	*
	*/
	SSBootloaderComm(USBSerial* USB, SSInterface* ssInterface, DSInterface* dsInterface);

	/**
	* @brief	Parses DeviceStudio-style commands.
	* @details  Parses and executes commands. Prints return code to i/o device.
	* @returns true if sensor acted upon the command, false if command was unknown
	*/
	bool parse_command(const char* cmd);

private:

    /* PRIVATE METHODS */
    int parse_auth(const char* cmd, uint8_t *auth_bytes);
    int parse_iv(const char* cmd, uint8_t* iv_bytes);
    void flash_page_data(void);

	/* PRIVATE TYPE DEFINITIONS */
	typedef enum _cmd_state_t {
		cmd_enter_bootldr=0,
		cmd_exit_bootldr,
		cmd_reset,
		cmd_page_size,
		cmd_num_pages,
		cmd_set_iv,
		cmd_set_auth,
		cmd_erase,
		cmd_page_erase,
		cmd_flash,
		cmd_setcfg_bl_enter_mode,
		cmd_setcfg_bl_ebl_pin,
		cmd_setcfg_bl_ebl_pol,
		cmd_setcfg_bl_exit_mode,
		cmd_setcfg_bl_timeout,
		cmd_setcfg_bl_save,
		cmd_image_on_ram,
		cmd_setcfg_host_ebl,
		cmd_setcfg_host_cdf,
		cmd_getcfg_host,
		NUM_CMDS,
	} cmd_state_t;

	/* PRIVATE VARIABLES */
	USBSerial *m_USB;
	SSInterface *ss_int;
    DSInterface *ds_int;

    char bootldr_version[128];

    int num_pages;
    int page_size;
    int bl_comm_delay_factor;

    bool got_page_size;
    bool sent_num_pages;

    /* PRIVATE CONST VARIABLES */
    static const int AES_NONCE_SIZE = 11;
    static const int AES_AUTH_SIZE = 16;
    static const int MAX_PAGE_SIZE = 8192;
    static const int CHECKBYTES_SIZE = 16;

};

#endif /* _SSBOOTLOADERCOMM_H_ */
