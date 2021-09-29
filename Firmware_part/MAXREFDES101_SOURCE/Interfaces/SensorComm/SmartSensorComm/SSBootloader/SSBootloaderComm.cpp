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
#include "SSBootloaderComm.h"

#include "../../../../Utilities/mxm_assert.h"
#include "SSInterface.h"
#include "Peripherals.h"
#include "utils.h"
#include "BLE_ICARUS.h"


static bool flag_image_on_ram = false;

static const char* const cmd_tbl[] = {
    "bootldr",
    "exit",
    "reset",
    "page_size",
    "num_pages",
    "set_iv",
    "set_auth",
    "erase",
    "page_erase",
    "flash",
	"set_cfg bl enter_mode",
	"set_cfg bl enter_pin",
	"set_cfg bl enter_pol",
	"set_cfg bl exit_mode",
	"set_cfg bl exit_to",
	"set_cfg bl save",
	"image_on_ram",
	"set_cfg host ebl",
	"set_cfg host cdf", /* bootloader comm_delay_factor */
	"get_cfg host",
};

SSBootloaderComm::SSBootloaderComm(USBSerial *USB, SSInterface* ssInterface, DSInterface* dsInterface)
	:SensorComm("bl", false), m_USB(USB), ss_int(ssInterface), ds_int(dsInterface)
{
	bl_comm_delay_factor = 1;
}

int SSBootloaderComm::parse_auth(const char* cmd, uint8_t *auth_bytes)
{
    char cmdStr[] = "set_auth ";
    int length = strlen(cmd);
    int expected_length = strlen(cmdStr) + 2*AES_AUTH_SIZE;
    if (length != expected_length) {
        pr_err("Couldn't parse Auth bytes, incorrect number of characters (len:%d, expected:%d)\n",
            length, expected_length);
        return COMM_INVALID_PARAM;
    }

    const char* macPtr = cmd + strlen(cmdStr);

	int num_found;
	int byteVal;
	for (int aidx = 0; aidx < AES_AUTH_SIZE; aidx++) {
		num_found = sscanf(macPtr, "%2X", &byteVal);

		if (num_found != 1 || byteVal > 0xFF) {
			pr_err("Couldn't parse byte %d of Auth\n", aidx);
			return COMM_INVALID_PARAM;
		}

		auth_bytes[aidx] = (uint8_t)byteVal;
		macPtr += 2;
	}

	return COMM_SUCCESS;
}

int SSBootloaderComm::parse_iv(const char* cmd, uint8_t* iv_bytes)
{
    char cmdStr[] = "set_iv ";
    int length = strlen(cmd);
    int expected_length = strlen(cmdStr) + 2*AES_NONCE_SIZE;
    if (length != expected_length) {
        pr_err("Couldn't parse IV, incorrect number of characters (len:%d, expected:%d)\n",
               length, expected_length);
        return COMM_INVALID_PARAM;
    }

    const char* ivPtr = cmd + strlen(cmdStr);

	int num_found;
	int byteVal;
	for (int ividx = 0; ividx < AES_NONCE_SIZE; ividx++) {
		num_found = sscanf(ivPtr, "%2X", &byteVal);

		if (num_found != 1 || byteVal > 0xFF) {
			pr_err("Couldn't parse byte %d of IV\n", ividx);
			return COMM_INVALID_PARAM;
		}
		iv_bytes[ividx] = (uint8_t)byteVal;
		ivPtr += 2;
	}

	return COMM_SUCCESS;
}

bool SSBootloaderComm::parse_command(const char* cmd)
{
    int ret = EXIT_SUCCESS;
    bool recognizedCmd = false;
	char charbuf[768];
	int data_len = 0;
	bool flash_flag=false;

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
                case cmd_enter_bootldr:
                {
					SS_STATUS status;
					status = ss_int->reset_to_bootloader();
					if (status == SS_SUCCESS){
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
	                    m_USB->printf(charbuf);
					}
					else{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
	                    m_USB->printf(charbuf);
					}

                    ds_int->set_fw_platform(ss_int->get_ss_platform_name());
                    ds_int->set_fw_version(ss_int->get_ss_fw_version());
                    got_page_size = false;
                    sent_num_pages = false;
                } break;

                case cmd_exit_bootldr:
                {
					SS_STATUS status = ss_int->reset_to_main_app();
                    if (status == SS_SUCCESS){
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                        m_USB->printf(charbuf);
                    }
                    else{
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                        m_USB->printf(charbuf);
                    }

                    ds_int->set_fw_platform(ss_int->get_ss_platform_name());
                    ds_int->set_fw_version(ss_int->get_ss_fw_version());

		} break;

                case cmd_reset:
                {
					SS_STATUS status = ss_int->reset();
                    if (status == SS_SUCCESS)
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                    else
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                    m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);

                    ds_int->set_fw_platform(ss_int->get_ss_platform_name());
                    ds_int->set_fw_version(ss_int->get_ss_fw_version());

                } break;

                case cmd_page_size:
                {
                    uint8_t cmd_bytes[] = { SS_FAM_R_BOOTLOADER, SS_CMDIDX_PAGESIZE };
                    uint8_t rxbuf[3];

                    SS_STATUS status = ss_int->read_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0,
                            &rxbuf[0], ARRAY_SIZE(rxbuf));

                    if (status == SS_SUCCESS) {
                        //rxbuf holds page size in big-endian format
                        page_size = (256*(int)rxbuf[1]) + rxbuf[2];
                        mxm_assert_msg(page_size <= MAX_PAGE_SIZE, "Page size exceeds maximum allowed");

                        data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%d err=%d\r\n", cmd, page_size, COMM_SUCCESS);
                        m_USB->printf(charbuf);
                        got_page_size = true;

                    } else {
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                        m_USB->printf(charbuf);
                    }
                } break;

                case cmd_num_pages:
                {
                    int num_tok = sscanf(cmd, "num_pages %d", &num_pages);
                    if (num_tok != 1) {
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%d err=%d\r\n", cmd, 0, COMM_INVALID_PARAM);
                        m_USB->printf(charbuf);
						break;
                    }

                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SETNUMPAGES };
                    //num pages = 256*MSB + LSB
                    uint8_t data[] = { (uint8_t)((num_pages >> 8) & 0xFF), (uint8_t)(num_pages & 0xFF) };

                    SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            &data[0], ARRAY_SIZE(data));

                    if (status == SS_SUCCESS) {
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                        m_USB->printf(charbuf);
                        sent_num_pages = true;

                    } else {
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                        m_USB->printf(charbuf);
                    }
                } break;

                case cmd_set_iv:
                {
                    uint8_t iv_bytes[AES_NONCE_SIZE];
                    ret = parse_iv(cmd, &iv_bytes[0]);
                    if (ret != COMM_SUCCESS) {
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, ret);
                        m_USB->printf(charbuf);
                    }
                    else
                    {
                        uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SETIV };

                        SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            &iv_bytes[0], ARRAY_SIZE(iv_bytes));

                        if (status == SS_SUCCESS) {
                        	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                            m_USB->printf(charbuf);

                        } else {
                        	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                            m_USB->printf(charbuf);
                        }
                    }
                } break;

                case cmd_set_auth:
                {
                    uint8_t auth_bytes[AES_AUTH_SIZE];
                    ret = parse_auth(cmd, &auth_bytes[0]);
                    if (ret != COMM_SUCCESS) {
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, ret);
                        m_USB->printf(charbuf);
                    }
                    else
                    {
                        uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SETAUTH };

                        SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            &auth_bytes[0], ARRAY_SIZE(auth_bytes));

                        if (status == SS_SUCCESS) {
                        	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                            m_USB->printf(charbuf);

                        } else {
                        	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                            m_USB->printf(charbuf);
                        }
                    }
                } break;

                case cmd_erase:
                {
                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_ERASE };

                    SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0,
							SS_BOOTLOADER_ERASE_DELAY);
                    if (status == SS_SUCCESS){
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                        m_USB->printf(charbuf);
                    }
                    else{
                    	data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                        m_USB->printf(charbuf);
                    }
                } break;

                case cmd_page_erase:
                {
					int page_num_to_erase;
                    int num_tok = sscanf(cmd, "page_erase %d", &page_num_to_erase);
                    if (num_tok != 1) {
                        m_USB->printf("\r\n%s value=%d err=%d\r\n", cmd, 0, COMM_INVALID_PARAM);
						break;
                    }

                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_ERASE_PAGE };
                    //num pages = 256*MSB + LSB
                    uint8_t data[] = { (uint8_t)((page_num_to_erase >> 8) & 0xFF), (uint8_t)(page_num_to_erase & 0xFF) };

                    SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            &data[0], ARRAY_SIZE(data), 50);

			if (status == SS_SUCCESS) {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				sent_num_pages = true;
			} else {
				m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
			}
		} break;

                case cmd_flash:
                {
                	if (got_page_size && sent_num_pages) {
                		data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                		m_USB->printf(charbuf);

                		if (BLE::Instance().gap().getState().connected) {
                			BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
                			BLE_Icarus_TransferDataFromQueue();
                		}
                		BLE::Instance().waitForEvent();

                		flash_page_data();
                		flash_flag=true;
                	} else {
                		pr_err("Can't enter flash mode. Need number of pages, and size of page"
                				"(num_pages, page_size, commands)\r\n");
                		data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
                		m_USB->printf(charbuf);
                	}
		} break;

				case cmd_setcfg_bl_enter_mode:
				{
					uint8_t mode;
					ret = parse_cmd_data(cmd, cmd_tbl[i], &mode, 1, false);
					if (ret != 1) {
						pr_err("parse_cmd_data=%d\r\n", ret);
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CFG_ENTER_BL_MODE, mode };
					SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0);
                    if (status == SS_SUCCESS)
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                    else
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

				case cmd_setcfg_bl_ebl_pin:
				{
					uint8_t pin[2];
					ret = parse_cmd_data(cmd, cmd_tbl[i], &pin[0], 2, false);
					if (ret != 2) {
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CFG_EBL_PIN,
											pin[0], pin[1]};
					SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0);
                    if (status == SS_SUCCESS)
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                    else
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

				case cmd_setcfg_bl_ebl_pol:
				{
					uint8_t mode;
					ret = parse_cmd_data(cmd, cmd_tbl[i], &mode, 1, false);
					if (ret != 1) {
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_ENTRY, SS_BL_CFG_EBL_POL, mode };
					SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0);
                    if (status == SS_SUCCESS)
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                    else
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;

				case cmd_setcfg_bl_exit_mode:
				{
					uint8_t mode;
					ret = parse_cmd_data(cmd, cmd_tbl[i], &mode, 1, false);
					if (ret != 1) {
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_EXIT, SS_BL_CFG_EXIT_BL_MODE, mode };
					SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0);
                    if (status == SS_SUCCESS)
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                    else
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

				} break;
				case cmd_setcfg_bl_timeout:
				{
					uint8_t to;
					ret = parse_cmd_data(cmd, cmd_tbl[i], &to, 1, false);
					if (ret != 1) {
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_EXIT, SS_BL_CFG_TIMEOUT, to };
					SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0);
                    if (status == SS_SUCCESS)
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                    else
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

				} break;
				case cmd_setcfg_bl_save:
				{
                    uint8_t cmd_bytes[] = { SS_FAM_W_BOOTLOADER_CFG, SS_CMDIDX_BL_SAVE };

					SS_STATUS status = ss_int->write_cmd(
                            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
                            0, 0, 50);
                    if (status == SS_SUCCESS)
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
                    else
                        m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);

		} break;
				case cmd_image_on_ram:
				{
					uint8_t mode = 0;
					ret = parse_cmd_data(cmd, cmd_tbl[i], &mode, 1, false);
					if (ret != 1) {
						m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

					/* TODO: Implement effects of this */
					flag_image_on_ram = !!mode;
					pr_info("flag_image_on_ram: %d, mode: %d\n", flag_image_on_ram, mode);
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
					break;
				} break;
				case cmd_setcfg_host_ebl:
				{
					uint8_t tmp; /* 0: Command bootloader, 1: No command, trigger GPIO */

					int ret = parse_cmd_data(cmd, cmd_tbl[i], &tmp, 1, false);
					if (ret != 1) {
						m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

					if (ss_int->set_ebl_mode(tmp) == SS_SUCCESS)
						m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
					else
						m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
					break;
				} break;
				case cmd_setcfg_host_cdf:
				{
					uint8_t tmp;

					int ret = parse_cmd_data(cmd, cmd_tbl[i], &tmp, 1, false);
					if (ret != 1) {
						m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						break;
					}

					/* TODO: Implement effects of this */
					bl_comm_delay_factor = tmp;
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
					break;
				} break;
				case cmd_getcfg_host:
				{
					m_USB->printf("\r\n%s ebl=%d cdf=%d err=%d\r\n", cmd, ss_int->get_ebl_mode(), bl_comm_delay_factor, COMM_SUCCESS);
				} break;
                default:
                {
                	mxm_assert_msg(false, "Invalid switch case!");
                }
            }


				if (BLE::Instance().gap().getState().connected) {
					if(flash_flag==false)
					{
						BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
					}
				}


        }
    }

    return recognizedCmd;
}

void SSBootloaderComm::flash_page_data(void)
{
    int totalBytes = 0;
    int currentPage = 1;
	char charbuf_flash[256];
	int data_len_flash = 0;

	static uint8_t tx_buf[MAX_PAGE_SIZE + CHECKBYTES_SIZE + 2] = { SS_FAM_W_BOOTLOADER, SS_CMDIDX_SENDPAGE };
	uint8_t *data_buffer = &tx_buf[2];

	while (currentPage <= num_pages) {
		pr_info("Waiting for page %d/%d data (%d bytes)...", currentPage, num_pages, page_size);

        //Collect page data + checksum from PC/Android
//		totalBytes = 100;
        while (totalBytes < (page_size + CHECKBYTES_SIZE)) {
            data_buffer[totalBytes++] = m_USB->_getc();
            //Here we should be able to take the data over BLE
        }

		pr_info("Done\r\n");

        //Send data to SmartSensor
        SS_STATUS status = ss_int->write_cmd(tx_buf, page_size + CHECKBYTES_SIZE + 2, 2000 /*bl_comm_delay_factor * PAGE_WRITE_DELAY_MS*/);
		pr_err("status: %d\r\n", status);

        //Inform PC/Andoid of status
        if (status == SS_ERR_BTLDR_CHECKSUM) {
            pr_err("Verify checksum failed!\r\n");
            data_len_flash = snprintf(charbuf_flash, sizeof(charbuf_flash), "\r\npageFlashDone err=%d\r\n", FLASH_ERR_CHECKSUM);
            m_USB->printf(charbuf_flash);
        } else if (status != SS_SUCCESS) {
            pr_err("Page flash failed!\r\n");
            data_len_flash = snprintf(charbuf_flash, sizeof(charbuf_flash), "\r\npageFlashDone err=%d\r\n", FLASH_ERR_GENERAL);
            m_USB->printf(charbuf_flash);
        } else {
            currentPage++;
            pr_err("Page flash successful!\r\n");
            data_len_flash = snprintf(charbuf_flash, sizeof(charbuf_flash), "\r\npageFlashDone err=%d\r\n", COMM_SUCCESS);
            m_USB->printf(charbuf_flash);
        }

		totalBytes = 0;

				if (BLE::Instance().gap().getState().connected) {
					BLE_Icarus_AddtoQueue((uint8_t *)charbuf_flash, (int32_t)sizeof(charbuf_flash), data_len_flash);
					BLE_Icarus_TransferDataFromQueue();
				}
				BLE::Instance().waitForEvent();


    }
}
