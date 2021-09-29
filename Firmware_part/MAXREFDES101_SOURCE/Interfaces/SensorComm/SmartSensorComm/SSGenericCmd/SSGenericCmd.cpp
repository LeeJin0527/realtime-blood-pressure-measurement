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

#include "DSInterface.h"
#include "SSGenericCmd.h"

#include "../../../../Utilities/mxm_assert.h"
#include "Peripherals.h"
#include "utils.h"
#include "CRC8.h"

static const char* const cmd_tbl[] = {
	"ss_write",
	"ss_read",
};

SSGenericCmd::SSGenericCmd(USBSerial *USB, SSInterface* ssInterface, DSInterface* dsInterface)
    :SensorComm("ss_test", false), m_USB(USB), ss_int(ssInterface), ds_int(dsInterface)
{
}

bool SSGenericCmd::parse_command(const char* cmd)
{
    SS_STATUS status;
    bool recognizedCmd = false;
	char response[1024];
	int ridx = 0;

    if (!ss_int) {
        pr_err("No SmartSensor Interface defined!");
        return false;
    }
    if (!ds_int) {
        pr_err("No DeviceStudio Interface defined!");
        return false;
    }

    for (int i = 0; i < NUM_CMDS; i++) {
        if (starts_with(cmd, cmd_tbl[i])) {
            cmd_state_t user_cmd = (cmd_state_t)i;
            recognizedCmd = true;

            switch (user_cmd) {
				case (ss_write):
				{
					uint8_t cmd_bytes[256];
					int num_cmd_bytes = parse_cmd_data(cmd, cmd_tbl[i], &cmd_bytes[0], sizeof(cmd_bytes), true);
					if (num_cmd_bytes <= 0) {
						ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
								"\r\n%s err=%d", cmd, num_cmd_bytes);
						break;
					}

					status = ss_int->write_cmd(&cmd_bytes[0], num_cmd_bytes,
														0, 0, 500);
					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"\r\n%s err=0\r\n", cmd);

					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"\r\nWrote: { ");
					for (int i = 0; i < num_cmd_bytes; i++) {
						ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
								"%02X ", cmd_bytes[i]);
					}
					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"}\r\n");
					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"Status: { %02X }\r\n", (uint8_t)status);

				} break;

				case (ss_read):
				{
					uint8_t cmd_bytes[256];
					uint8_t read_bytes[256];


					int num_cmd_bytes = parse_cmd_data(cmd, cmd_tbl[i], &cmd_bytes[0], sizeof(cmd_bytes), true);
					if (num_cmd_bytes <= 0) {
						ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
								"\r\n%s err=%d", cmd, num_cmd_bytes);
						break;
					}

					//Last space separated value is the number of bytes to read
					uint8_t num_rx_bytes = cmd_bytes[num_cmd_bytes - 1];
					if (num_rx_bytes > sizeof(read_bytes)) {
        				pr_err("Can read up to %d bytes", num_rx_bytes);
						ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
								"\r\n%s err=-1", cmd);
						break;
					}

					status = ss_int->read_cmd(&cmd_bytes[0], num_cmd_bytes - 1,
														0, 0,
														&read_bytes[0], num_rx_bytes,
														500);

					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
										"\r\n%s err=0\r\n", cmd);

					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"\r\nWrote: { ");
					for (int i = 0; i < num_cmd_bytes; i++) {
						ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
								"%02X ", cmd_bytes[i]);
					}
					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"}\r\n");
					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"Read: { ");
					for (int i = 0; i < num_rx_bytes; i++) {
						ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
								"%02X ", read_bytes[i]);
					}
					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"}\r\n");
					ridx += snprintf(response + ridx, sizeof(response) - ridx - 1,
							"Status: { %02X }\r\n", (uint8_t)status);

				} break;

                default:
                {
					recognizedCmd = false;
					mxm_assert_msg(false, "Invalid switch case!");
                } break;

            }
			break;
        }
    }

	if (recognizedCmd) {
		m_USB->printf(response);
	}

    return recognizedCmd;
}
