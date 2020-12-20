/***************************************************************************
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
****************************************************************************
*/

#include "SSInterface.h"

#include "../../Utilities/mxm_assert.h"
#include "Peripherals.h"
#include "utils.h"
#include "i2cm.h"

#include "pwrman_regs.h"
#include "ioman.h"



SSInterface::SSInterface(I2C &i2cBus, PinName ss_mfio, PinName ss_reset)
	:m_i2cBus(&i2cBus), m_spiBus(NULL),
	mfio_pin(ss_mfio), reset_pin(ss_reset), irq_pin(ss_mfio)/*,
	irq_evt(1000000, "irq")*/
{
	reset_pin.input();
	irq_pin.fall(callback(this, &SSInterface::irq_handler));

	//reset_to_main_app();
	//get_data_type(&data_type, &sc_en);
	ebl_mode = EBL_GPIO_TRIGGER_MODE;
}

SSInterface::SSInterface(SPI &spiBus, PinName ss_mfio, PinName ss_reset)
	:m_i2cBus(NULL), m_spiBus(&spiBus),
	mfio_pin(ss_mfio), reset_pin(ss_reset), irq_pin(ss_mfio)/*,
	irq_evt(1000000, "irq")*/
{
	reset_pin.input();
	irq_pin.fall(callback(this, &SSInterface::irq_handler));

	//reset_to_main_app();
	//get_data_type(&data_type, &sc_en);
	ebl_mode = EBL_GPIO_TRIGGER_MODE;
}

SSInterface::~SSInterface()
{
}

SS_STATUS SSInterface::reset_to_main_app()
{
	SS_STATUS status;
	disable_irq();

	int bootldr = in_bootldr_mode();
	if (bootldr > 0) {
		status = exit_from_bootloader();
	} else if (bootldr == 0) {
		reset_pin.output();

	        if (ebl_mode == EBL_GPIO_TRIGGER_MODE)
		cfg_mfio(PIN_OUTPUT);

		reset_pin.write(0);
		wait_ms(SS_RESET_TIME);

		if (ebl_mode == EBL_GPIO_TRIGGER_MODE)
			mfio_pin.write(1);

		reset_pin.write(1);
		wait_ms(SS_STARTUP_TO_MAIN_APP_TIME);

		if (ebl_mode == EBL_GPIO_TRIGGER_MODE)
			cfg_mfio(PIN_INPUT);

		reset_pin.input();
		status = SS_SUCCESS;
	} else
		status = SS_ERR_UNKNOWN;
	enable_irq();
	return status;
}

SS_STATUS SSInterface::reset_to_bootloader()
{
	disable_irq();
	int bootldr = in_bootldr_mode();
	if (bootldr > 0) {
		enable_irq();
		return SS_SUCCESS;
	}
	reset_pin.output();
	if (ebl_mode == EBL_GPIO_TRIGGER_MODE)
		cfg_mfio(PIN_OUTPUT);

	reset_pin.write(0);
	wait_ms(SS_RESET_TIME);
	if (ebl_mode == EBL_GPIO_TRIGGER_MODE)
		 mfio_pin.write(0);

	reset_pin.write(1);
	wait_ms(SS_STARTUP_TO_BTLDR_TIME);
	reset_pin.input();

	if (ebl_mode == EBL_CMD_TRIGGER_MODE)
		stay_in_bootloader();

	if (ebl_mode == EBL_GPIO_TRIGGER_MODE) {
		cfg_mfio(PIN_INPUT);
		stay_in_bootloader();
	}

	 // Verify we entered bootloader mode
	if (in_bootldr_mode() < 0) {
		enable_irq();
		return SS_ERR_UNKNOWN;
	}
	enable_irq();
	return SS_SUCCESS;
}

SS_STATUS SSInterface::exit_from_bootloader()
{
	uint8_t cmd_bytes[] = { SS_FAM_W_MODE, SS_CMDIDX_MODE };
	uint8_t data[] = { 0x00 };

	SS_STATUS status = write_cmd(
			&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
			&data[0], ARRAY_SIZE(data));

	in_bootldr = (status == SS_SUCCESS) ? true : false;
	return status;
}

SS_STATUS SSInterface::stay_in_bootloader()
{
	uint8_t cmd_bytes[] = { SS_FAM_W_MODE, SS_CMDIDX_MODE };
	uint8_t data[] = { SS_MASK_MODE_BOOTLDR };

	SS_STATUS status = write_cmd(
			&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
			&data[0], ARRAY_SIZE(data));

	in_bootldr = (status == SS_SUCCESS) ? true : false;
	return status;
}

int SSInterface::get_ebl_mode()
{
	return ebl_mode;
}

SS_STATUS SSInterface::set_ebl_mode(uint8_t mode)
{
	if (mode == EBL_CMD_TRIGGER_MODE || mode == EBL_GPIO_TRIGGER_MODE) {
		ebl_mode = mode;
		return SS_SUCCESS;
	} else {
		return SS_ERR_INPUT_VALUE;
	}
}

SS_STATUS SSInterface::reset()
{
	int bootldr = in_bootldr_mode();
	if (bootldr > 0)
		return reset_to_bootloader();
	else if (bootldr == 0)
		return reset_to_main_app();
	else
		return SS_ERR_UNKNOWN;
}

SS_STATUS SSInterface::self_test(int idx, uint8_t *result, int sleep_ms){
    uint8_t cmd_bytes[] = { SS_FAM_R_SELFTEST, (uint8_t)idx };
    uint8_t rxbuf[2];
    SS_STATUS ret;

	result[0] = 0xFF;
	ret = read_cmd(cmd_bytes, 2, (uint8_t *)0, 0, rxbuf, ARRAY_SIZE(rxbuf), sleep_ms);
	result[0] = rxbuf[1];
	return ret;
}

void SSInterface::cfg_mfio(PinDirection dir)
{
	if (dir == PIN_INPUT) {
		mfio_pin.input();
		mfio_pin.mode(PullUp);
	} else {
		disable_irq();
		mfio_pin.output();
	}
}

void SSInterface::enable_irq()
{
	irq_pin.enable_irq();
}
void SSInterface::disable_irq()
{
	irq_pin.disable_irq();
}

void SSInterface::mfio_selftest(){
	disable_irq();
	irq_pin.fall(callback(this, &SSInterface::irq_handler_selftest));
	enable_irq();
}

int SSInterface::in_bootldr_mode()
{
	uint8_t cmd_bytes[] = { SS_FAM_R_MODE, SS_CMDIDX_MODE };
	uint8_t rxbuf[2] = { 0 };

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
			0, 0,
			&rxbuf[0], ARRAY_SIZE(rxbuf));
	if (status != SS_SUCCESS)
		return -1;

	return (rxbuf[1] & SS_MASK_MODE_BOOTLDR);
}

const char* SSInterface::get_ss_fw_version()
{
    uint8_t cmd_bytes[2];
    uint8_t rxbuf[4];

	int bootldr = in_bootldr_mode();

	if (bootldr > 0) {
		cmd_bytes[0] = SS_FAM_R_BOOTLOADER;
		cmd_bytes[1] = SS_CMDIDX_BOOTFWVERSION;
	} else if (bootldr == 0) {
		cmd_bytes[0] = SS_FAM_R_IDENTITY;
		cmd_bytes[1] = SS_CMDIDX_FWVERSION;
	} else {
		return plat_name;
	}

    SS_STATUS status = read_cmd(
             &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
             0, 0,
             &rxbuf[0], ARRAY_SIZE(rxbuf));

    if (status == SS_SUCCESS) {
        snprintf(fw_version, sizeof(fw_version),
            "%d.%d.%d", rxbuf[1], rxbuf[2], rxbuf[3]);
		pr_info("fw_version:%s\r\n", fw_version);
    }

    return &fw_version[0];
}

const char* SSInterface::get_ss_algo_version()
{
    uint8_t cmd_bytes[3];
    uint8_t rxbuf[4];

	int bootldr = in_bootldr_mode();

	if (bootldr > 0) {
		cmd_bytes[0] = SS_FAM_R_BOOTLOADER;
		cmd_bytes[1] = SS_CMDIDX_BOOTFWVERSION;
		cmd_bytes[2] = 0;
	} else if (bootldr == 0) {
		cmd_bytes[0] = SS_FAM_R_IDENTITY;
		cmd_bytes[1] = SS_CMDIDX_ALGOVER;
		cmd_bytes[2] = SS_CMDIDX_AVAILSENSORS;
	} else {
		return plat_name;
	}

    SS_STATUS status = read_cmd(
             &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
             0, 0,
             &rxbuf[0], ARRAY_SIZE(rxbuf));

    if (status == SS_SUCCESS) {
        snprintf(algo_version, sizeof(algo_version),
            "%d.%d.%d", rxbuf[1], rxbuf[2], rxbuf[3]);
		pr_info("algo_version:%s\r\n", fw_version);
    }

    return &algo_version[0];
}
const char* SSInterface::get_ss_platform_name()
{
    uint8_t cmd_bytes[] = { SS_FAM_R_IDENTITY, SS_CMDIDX_PLATTYPE };
    uint8_t rxbuf[2];

    SS_STATUS status = read_cmd(
            &cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
            0, 0,
            &rxbuf[0], ARRAY_SIZE(rxbuf));

    if (status == SS_SUCCESS) {
        if (rxbuf[1] == SS_PLAT_MAX3263X) {
			if (in_bootldr_mode() > 0) {
				plat_name = SS_BOOTLOADER_PLATFORM_MAX3263X;
			} else {
	            plat_name = SS_PLATFORM_MAX3263X;
			}
        } else if (rxbuf[1] == SS_PLAT_MAX32660) {
			if (in_bootldr_mode() > 0) {
				plat_name = SS_BOOTLOADER_PLATFORM_MAX32660;
			} else {
				plat_name = SS_PLATFORM_MAX32660;
			}
        }
    }

    return plat_name;
}

SS_STATUS SSInterface::write_cmd(uint8_t *cmd_bytes, int cmd_bytes_len,
	uint8_t *data, int data_len,
    int sleep_ms)
{
    int total_len = data_len + cmd_bytes_len;

    if (total_len <= SS_SMALL_BUF_SIZE) {
        return write_cmd_small(cmd_bytes, cmd_bytes_len, data, data_len, sleep_ms);
    } else if (total_len <= SS_MED_BUF_SIZE) {
        return write_cmd_medium(cmd_bytes, cmd_bytes_len, data, data_len, sleep_ms);
    } else if (total_len <= SS_LARGE_BUF_SIZE) {
        return write_cmd_large(cmd_bytes, cmd_bytes_len, data, data_len, sleep_ms);
    } else {
    	mxm_assert_msg(true, "Tried to send I2C tx larger than maximum allowed size\n");
        return SS_ERR_DATA_FORMAT; 
    }
}

#include "i2cm.h"
void dump_i2c_regs(int idx)
{
	mxc_i2cm_regs_t *i2cm = MXC_I2CM_GET_I2CM(idx);
	pr_err("****************************\r\n"
		"I2CM(%d) registers Dump\r\n"
		"i2cm: %p\r\n"
		"fs_clk_div : 0x%X\r\n"
		"timeout : 0x%X\r\n"
		"ctrl : 0x%X\r\n"
		"trans : 0x%X\r\n"
		"intfl : 0x%X\r\n"
		"inten : 0x%X\r\n"
		"bb : 0x%X\r\n"
		"****************************\r\n",
		idx,
		i2cm,
		i2cm->fs_clk_div,
		i2cm->timeout,
		i2cm->ctrl,
		i2cm->trans,
		i2cm->intfl,
		i2cm->inten,
		i2cm->bb);

	/* Clean flags */
	i2cm->intfl = i2cm->intfl;
}

SS_STATUS SSInterface::write_cmd(uint8_t *tx_buf, int tx_len, int sleep_ms)
{
	pr_info("write_cmd: ");
	for (int i = 0; i < tx_len; i++) {
		pr_info("0x%02X ", tx_buf[i]);
	}
	pr_info("\r\n");

    int ret = m_i2cBus->write(SS_I2C_8BIT_SLAVE_ADDR, (char*)tx_buf, tx_len);

	int retries = 4;
	while (ret != 0 && retries-- > 0) {
		pr_err("i2c wr retry\r\n");
		wait_ms(1);
	ret = m_i2cBus->write(SS_I2C_8BIT_SLAVE_ADDR, (char*)tx_buf, tx_len);
	}

    if (ret != 0) {
    	pr_err("m_i2cBus->write returned %d\r\n", ret);

        return SS_ERR_UNAVAILABLE;
    }

    wait_ms(sleep_ms);

    char status_byte;
    ret = m_i2cBus->read(SS_I2C_8BIT_SLAVE_ADDR, &status_byte, 1);
	bool try_again = (status_byte == SS_ERR_TRY_AGAIN);
	while ((ret != 0 || try_again)
			&& retries-- > 0) {
		pr_info("i2c rd retry\r\n");
		wait_ms(sleep_ms);
	ret = m_i2cBus->read(SS_I2C_8BIT_SLAVE_ADDR, &status_byte, 1);
		try_again = (status_byte == SS_ERR_TRY_AGAIN);
	}

    if (ret != 0 || try_again) {

    	pr_err("m_i2cBus->read returned %d, ss status_byte %d\r\n", ret, status_byte);
        return SS_ERR_UNAVAILABLE;
    }

	pr_info("status_byte: %d\r\n", status_byte);

	return (SS_STATUS)status_byte;
}

SS_STATUS SSInterface::write_cmd_small(uint8_t *cmd_bytes, int cmd_bytes_len,
                       uint8_t *data, int data_len,
                       int sleep_ms)
{
    uint8_t write_buf[SS_SMALL_BUF_SIZE];
    memcpy(write_buf, cmd_bytes, cmd_bytes_len);
    memcpy(write_buf + cmd_bytes_len, data, data_len);

	SS_STATUS status = write_cmd(write_buf, cmd_bytes_len + data_len, sleep_ms);
	return status;
}

SS_STATUS SSInterface::write_cmd_medium(uint8_t *cmd_bytes, int cmd_bytes_len,
                       uint8_t *data, int data_len,
                       int sleep_ms)
{
    uint8_t write_buf[SS_MED_BUF_SIZE];
    memcpy(write_buf, cmd_bytes, cmd_bytes_len);
    memcpy(write_buf + cmd_bytes_len, data, data_len);

	SS_STATUS status = write_cmd(write_buf, cmd_bytes_len + data_len, sleep_ms);
	return status;
}

SS_STATUS SSInterface::write_cmd_large(uint8_t *cmd_bytes, int cmd_bytes_len,
                       uint8_t *data, int data_len,
                       int sleep_ms)
{
    uint8_t write_buf[SS_LARGE_BUF_SIZE];
    memcpy(write_buf, cmd_bytes, cmd_bytes_len);
    memcpy(write_buf + cmd_bytes_len, data, data_len);

	SS_STATUS status = write_cmd(write_buf, cmd_bytes_len + data_len, sleep_ms);
	return status;
}

SS_STATUS SSInterface::read_cmd(uint8_t *cmd_bytes, int cmd_bytes_len,
	uint8_t *data, int data_len,
	uint8_t *rxbuf, int rxbuf_sz,
    int sleep_ms)
{
#if 0
	pr_info("read_cmd: ");
	for (int i = 0; i < cmd_bytes_len; i++) {
		pr_info("0x%02X ", cmd_bytes[i]);
	}
	pr_info("\r\n");

#endif

	int retries = 4;

    int ret = m_i2cBus->write(SS_I2C_8BIT_SLAVE_ADDR, (char*)cmd_bytes, cmd_bytes_len, (data_len != 0));
#ifdef SHOW_I2C_DEBUG_MESSAGES
    printf("ret1 : %d\rt\n",ret);
#endif
    if (data_len != 0) {
        ret |= m_i2cBus->write(SS_I2C_8BIT_SLAVE_ADDR, (char*)data, data_len, false);
#ifdef SHOW_I2C_DEBUG_MESSAGES
        printf("ret2 : %d\rt\n",ret);
#endif
    }

	while (ret != 0 && retries-- > 0) {

		pr_err("i2c wr retry\r\n");
		wait_ms(1);
	ret = m_i2cBus->write(SS_I2C_8BIT_SLAVE_ADDR, (char*)cmd_bytes, cmd_bytes_len, (data_len != 0));
#ifdef SHOW_I2C_DEBUG_MESSAGES
	printf("ret3 : %d\rt\n",ret);
#endif
	    if (data_len != 0) {
	        ret |= m_i2cBus->write(SS_I2C_8BIT_SLAVE_ADDR, (char*)data, data_len, false);
#ifdef SHOW_I2C_DEBUG_MESSAGES
	        printf("ret4 : %d\rt\n",ret);
#endif
	    }
	}

    if (ret != 0) {
    	pr_err("m_i2cBus->write returned %d\r\n", ret);

        return SS_ERR_UNAVAILABLE;
    }

    wait_ms(sleep_ms);

    ret = m_i2cBus->read(SS_I2C_8BIT_SLAVE_ADDR, (char*)rxbuf, rxbuf_sz);
	bool try_again = (rxbuf[0] == SS_ERR_TRY_AGAIN);
	while ((ret != 0 || try_again) && retries-- > 0) {
		pr_info("i2c rd retry\r\n");
		wait_ms(sleep_ms);
	ret = m_i2cBus->read(SS_I2C_8BIT_SLAVE_ADDR, (char*)rxbuf, rxbuf_sz);
		try_again = (rxbuf[0] == SS_ERR_TRY_AGAIN);
	}
    if (ret != 0 || try_again) {
    	pr_err("m_i2cBus->read returned %d, ss status_byte %d\r\n", ret, rxbuf[0]);

        return SS_ERR_UNAVAILABLE;
    }

	pr_info("status_byte: %d\r\n", rxbuf[0]);
	pr_info("data: ");
	for (int i = 1; i < rxbuf_sz; i++) {
		pr_info("0x%02X ", rxbuf[i]);
	}
	pr_info("\r\n");

    return (SS_STATUS)rxbuf[0];
}

SS_STATUS SSInterface::get_reg(int idx, uint8_t addr, uint32_t *val)
{
	mxm_assert_msg((idx <= SS_MAX_SUPPORTED_SENSOR_NUM), "idx must be < SS_MAX_SUPPORTED_SENSOR_NUM, or update code to handle variable length idx values");

	uint8_t cmd_bytes[] = { SS_FAM_R_REGATTRIBS, (uint8_t)idx };
	uint8_t rx_reg_attribs[3] = {0};

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rx_reg_attribs[0], ARRAY_SIZE(rx_reg_attribs));

	if (status != SS_SUCCESS)
		return status;

	int reg_width = rx_reg_attribs[1];

	uint8_t cmd_bytes2[] = { SS_FAM_R_READREG, (uint8_t)idx, addr };
	uint8_t rxbuf[5] = {0};

	status = read_cmd(&cmd_bytes2[0], ARRAY_SIZE(cmd_bytes2),
						0, 0,
						&rxbuf[0], reg_width + 1);

	if (status == SS_SUCCESS) {
		*val = 0;
		for (int i = 0; i < reg_width; i++) {
			*val = (*val << 8) | rxbuf[i + 1];
		}
	}

	return status;
}

SS_STATUS SSInterface::set_reg(int idx, uint8_t addr, uint32_t val, int byte_size)
{
	mxm_assert_msg((idx <= SS_MAX_SUPPORTED_SENSOR_NUM), "idx must be < SS_MAX_SUPPORTED_SENSOR_NUM, or update code to handle variable length idx values");

	uint8_t cmd_bytes[] = { SS_FAM_W_WRITEREG, (uint8_t)idx, addr };
	uint8_t data_bytes[4];
	for (int i = 0; i < byte_size; i++) {
		data_bytes[i] = (val >> (8 * (byte_size - 1)) & 0xFF);
	}

	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								&data_bytes[0], byte_size, SS_ENABLE_SENSOR_SLEEP_MS);

	return status;
}

SS_STATUS SSInterface::dump_reg(int idx, addr_val_pair* reg_vals, int reg_vals_sz, int* num_regs)
{
	mxm_assert_msg((idx <= SS_MAX_SUPPORTED_SENSOR_NUM), "idx must be < SS_MAX_SUPPORTED_SENSOR_NUM, or update code to handle variable length idx values");

	uint8_t cmd_bytes[] = { SS_FAM_R_REGATTRIBS, (uint8_t)idx };
	uint8_t rx_reg_attribs[3] = {0};

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rx_reg_attribs[0], ARRAY_SIZE(rx_reg_attribs));

	if (status != SS_SUCCESS)
		return status;

	int reg_width = rx_reg_attribs[1];
	*num_regs = rx_reg_attribs[2];
	mxm_assert_msg((*num_regs <= reg_vals_sz), "Need to increase reg_vals array to hold all dump_reg data");
	mxm_assert_msg(((size_t)reg_width <= sizeof(uint32_t)), "IC returned register values greater than 4 bytes in width");

	int dump_reg_sz = (*num_regs) * (reg_width + 1) + 1; //+1 to reg_width for address, +1 for status byte

	uint8_t rxbuf[512];
	mxm_assert_msg(((size_t)dump_reg_sz <= sizeof(rxbuf)), "Need to increase buffer size to receive dump_reg data");

	cmd_bytes[0] = SS_FAM_R_DUMPREG;
	status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rxbuf[0], dump_reg_sz, SS_DUMP_REG_SLEEP_MS);

	if (status != SS_SUCCESS)
		return status;

	//rxbuf format is [status][addr0](reg_width x [val0])[addr1](reg_width x [val1])...
	for (int reg = 0; reg < *num_regs; reg++) {
		reg_vals[reg].addr = rxbuf[(reg * (reg_width + 1)) + 1];
		uint32_t *val = &(reg_vals[reg].val);
		*val = 0;
		for (int byte = 0; byte < reg_width; byte++) {
			*val = (*val << 8) | rxbuf[(reg * (reg_width + 1)) + byte + 2];
		}
	}

	return SS_SUCCESS;
}

SS_STATUS SSInterface::enable_sensor(int idx, int mode, ss_data_req *data_req, uint8_t ext_mode)
{
	mxm_assert_msg((idx <= SS_MAX_SUPPORTED_SENSOR_NUM), "idx must be < SS_MAX_SUPPORTED_SENSOR_NUM, or update code to handle variable length idx values");
	mxm_assert_msg((mode <= SS_MAX_SUPPORTED_MODE_NUM), "mode must be < SS_MAX_SUPPORTED_MODE_NUM, or update code to handle variable length mode values");
	mxm_assert_msg((mode != 0), "Tried to enable sensor to mode 0, but mode 0 is disable");


	uint8_t cmd_bytes[] = { SS_FAM_W_SENSORMODE, (uint8_t)idx, (uint8_t)mode, ext_mode };

	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes), 0, 0, 5 * SS_ENABLE_SENSOR_SLEEP_MS);

	if (status == SS_SUCCESS) {
		sensor_enabled_mode[idx] = mode;
		sensor_data_reqs[idx] = data_req;
	}
	return status;
}

SS_STATUS SSInterface::disable_sensor(int idx)
{
	mxm_assert_msg((idx <= SS_MAX_SUPPORTED_SENSOR_NUM), "idx must be < SS_MAX_SUPPORTED_SENSOR_NUM, or update code to handle variable length idx values");
	uint8_t cmd_bytes[] = { SS_FAM_W_SENSORMODE, (uint8_t)idx, 0 };

	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes), 0, 0, SS_ENABLE_SENSOR_SLEEP_MS);

	if (status == SS_SUCCESS) {
		sensor_enabled_mode[idx] = 0;
		sensor_data_reqs[idx] = 0;
	}

	return status;
}

SS_STATUS SSInterface::enable_algo(int idx, int mode, ss_data_req *data_req)
{
	mxm_assert_msg((idx <= SS_MAX_SUPPORTED_ALGO_NUM), "idx must be < SS_MAX_SUPPORTED_ALGO_NUM, or update code to handle variable length idx values");
	mxm_assert_msg((mode <= SS_MAX_SUPPORTED_MODE_NUM), "mode must be < SS_MAX_SUPPORTED_MODE_NUM, or update code to handle variable length mode values");
	mxm_assert_msg((mode != 0), "Tried to enable algo to mode 0, but mode 0 is disable");

	uint8_t cmd_bytes[] = { SS_FAM_W_ALGOMODE, (uint8_t)idx, (uint8_t)mode };

	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes), 0, 0, 25 * SS_ENABLE_SENSOR_SLEEP_MS);

	if (status == SS_SUCCESS) {
		algo_enabled_mode[idx] = mode;
		algo_data_reqs[idx] = data_req;
	}

	return status;
}

SS_STATUS SSInterface::disable_algo(int idx)
{
	mxm_assert_msg((idx <= SS_MAX_SUPPORTED_ALGO_NUM), "idx must be < SS_MAX_SUPPORTED_ALGO_NUM, or update code to handle variable length idx values");
	uint8_t cmd_bytes[] = { SS_FAM_W_ALGOMODE, (uint8_t)idx, 0 };

	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes), 0, 0, SS_ENABLE_SENSOR_SLEEP_MS);

	if (status == SS_SUCCESS) {
		algo_enabled_mode[idx] = 0;
		algo_data_reqs[idx] = 0;
	}

	return status;
}

SS_STATUS SSInterface::set_algo_cfg(int algo_idx, int cfg_idx, uint8_t *cfg, int cfg_sz)
{
	mxm_assert_msg((algo_idx <= SS_MAX_SUPPORTED_ALGO_NUM), "idx must be < SS_MAX_SUPPORTED_ALGO_NUM, or update code to handle variable length idx values");
	mxm_assert_msg((cfg_idx <= SS_MAX_SUPPORTED_ALGO_CFG_NUM), "idx must be < SS_MAX_SUPPORTED_ALGO_CFG_NUM, or update code to handle variable length idx values");

	uint8_t cmd_bytes[] = { SS_FAM_W_ALGOCONFIG, (uint8_t)algo_idx, (uint8_t)cfg_idx };
	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								 cfg, cfg_sz);

	return status;
}

SS_STATUS SSInterface::get_algo_cfg(int algo_idx, int cfg_idx, uint8_t *cfg, int cfg_sz)
{
	mxm_assert_msg((algo_idx <= SS_MAX_SUPPORTED_ALGO_NUM), "idx must be < SS_MAX_SUPPORTED_ALGO_NUM, or update code to handle variable length idx values");
	mxm_assert_msg((cfg_idx <= SS_MAX_SUPPORTED_ALGO_CFG_NUM), "idx must be < SS_MAX_SUPPORTED_ALGO_CFG_NUM, or update code to handle variable length idx values");

	uint8_t cmd_bytes[] = { SS_FAM_R_ALGOCONFIG, (uint8_t)algo_idx, (uint8_t)cfg_idx };
	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								cfg, cfg_sz);

	return status;
}

//--------------------------------------------------------------
SS_STATUS SSInterface::send_raw(uint8_t *rawdata, int rawdata_sz)
{
	SS_STATUS status = write_cmd(&rawdata[0], rawdata_sz, 5 * SS_ENABLE_SENSOR_SLEEP_MS);
	return status;
}
//---------------------------------------------------------------
SS_STATUS SSInterface::set_data_type(int data_type, bool sc_en)
{
	mxm_assert_msg((data_type >= 0) && (data_type <= 3), "Invalid value for data_type");
	uint8_t cmd_bytes[] = { SS_FAM_W_COMMCHAN, SS_CMDIDX_OUTPUTMODE };
	uint8_t data_bytes[] = { (uint8_t)((sc_en ? SS_MASK_OUTPUTMODE_SC_EN : 0) |
							((data_type << SS_SHIFT_OUTPUTMODE_DATATYPE) & SS_MASK_OUTPUTMODE_DATATYPE)) };

	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								&data_bytes[0], ARRAY_SIZE(data_bytes));

	this->data_type = data_type;
	this->sc_en = sc_en;

	return status;
}


SS_STATUS SSInterface::get_data_type(int *data_type, bool *sc_en)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_COMMCHAN, SS_CMDIDX_OUTPUTMODE };
	uint8_t rxbuf[2] = {0};

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rxbuf[0], ARRAY_SIZE(rxbuf));
	if (status == SS_SUCCESS) {
		*data_type =
			(rxbuf[1] & SS_MASK_OUTPUTMODE_DATATYPE) >> SS_SHIFT_OUTPUTMODE_DATATYPE;
		*sc_en =
			(bool)((rxbuf[1] & SS_MASK_OUTPUTMODE_SC_EN) >> SS_SHIFT_OUTPUTMODE_SC_EN);
	}

	return status;
}

SS_STATUS SSInterface::set_fifo_thresh(int thresh)
{
	mxm_assert_msg((thresh > 0 && thresh <= 255), "Invalid value for fifo a full threshold");
	uint8_t cmd_bytes[] = { SS_FAM_W_COMMCHAN, SS_CMDIDX_FIFOAFULL };
	uint8_t data_bytes[] = { (uint8_t)thresh };

	SS_STATUS status = write_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								&data_bytes[0], ARRAY_SIZE(data_bytes));
	return status;
}

SS_STATUS SSInterface::get_fifo_thresh(int *thresh)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_COMMCHAN, SS_CMDIDX_FIFOAFULL };
	uint8_t rxbuf[2] = {0};

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rxbuf[0], ARRAY_SIZE(rxbuf));

	if (status == SS_SUCCESS) {
		*thresh = rxbuf[1];
	}

	return status;
}

SS_STATUS SSInterface::ss_comm_check()
{
	uint8_t cmd_bytes[] = { SS_FAM_R_IDENTITY, SS_CMDIDX_PLATTYPE };
	uint8_t rxbuf[2];

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rxbuf[0], ARRAY_SIZE(rxbuf));

	int tries = 4;
	while (status == SS_ERR_TRY_AGAIN && tries--) {
		wait_ms(1000);
		status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
							0, 0,
							&rxbuf[0], ARRAY_SIZE(rxbuf));
	}

	return status;
}

void SSInterface::fifo_sample_size(int data_type, int *sample_size)
{
	*sample_size = 0;

	if (data_type == SS_DATATYPE_RAW || data_type == SS_DATATYPE_BOTH) {
		for (int i = 0; i < SS_MAX_SUPPORTED_SENSOR_NUM; i++) {
			if (sensor_enabled_mode[i]) {
				mxm_assert_msg(sensor_data_reqs[i], "no ss_data_req found for enabled sensor");
				*sample_size += sensor_data_reqs[i]->data_size;
			}
		}
	}

	if (data_type == SS_DATATYPE_ALGO || data_type == SS_DATATYPE_BOTH) {
		for (int i = 0; i < SS_MAX_SUPPORTED_ALGO_NUM; i++) {
			if (algo_enabled_mode[i]) {
				mxm_assert_msg(algo_data_reqs[i], "no ss_data_req found for enabled algo");
				*sample_size += algo_data_reqs[i]->data_size;
			}
		}
	}
}


SS_STATUS SSInterface::num_avail_samples(int *num_samples)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_OUTPUTFIFO, SS_CMDIDX_OUT_NUMSAMPLES };
	uint8_t rxbuf[2] = {0};

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rxbuf[0], ARRAY_SIZE(rxbuf), SS_DEFAULT2_CMD_SLEEP_MS);

	if (status == SS_SUCCESS) {
		*num_samples = rxbuf[1];
	}

	return status;
}

SS_STATUS SSInterface::get_log_len(int *log_len)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_LOG, SS_CMDIDX_R_LOG_LEN };
	uint8_t rxbuf[2] = {0};

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rxbuf[0], ARRAY_SIZE(rxbuf), 1);

	if (status == SS_SUCCESS) {
		*log_len = (rxbuf[1] << 8) | rxbuf[0];
	}

	return status;
}


SS_STATUS SSInterface::get_sensor_sample_size(uint8_t sensor_id, uint8_t *sample_size)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_INPUTFIFO, SS_CMDIDX_SAMPLE_SIZE, sensor_id };
	uint8_t rxbuf[2]; /* status + sample size */

	pr_info("[Reading external sample size for id: %d\n", sensor_id);
	SS_STATUS status = read_cmd(cmd_bytes, ARRAY_SIZE(cmd_bytes),
								NULL, 0,
								rxbuf, sizeof(rxbuf));
	*sample_size = rxbuf[1];
	return status;
}

SS_STATUS SSInterface::get_input_fifo_size(int *fifo_size)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_INPUTFIFO, SS_CMDIDX_INPUT_FIFO_SIZE};
	uint8_t rxbuf[3]; /* status + fifo size */

	pr_info("[Reading external Input FIFO size.\n");
	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								NULL, 0,
								rxbuf, sizeof(rxbuf), SS_DEFAULT2_CMD_SLEEP_MS);
	*fifo_size = rxbuf[1] << 8 | rxbuf[2];
	return status;
}

SS_STATUS SSInterface::get_sensor_fifo_size(uint8_t sensor_id, int *fifo_size)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_INPUTFIFO, SS_CMDIDX_SENSOR_FIFO_SIZE, sensor_id};
	uint8_t rxbuf[3]; /* status + fifo size */

	pr_info("[Reading sensor's FIFO size for id: %d\n", sensor_id);
	SS_STATUS status = read_cmd(cmd_bytes, ARRAY_SIZE(cmd_bytes),
								NULL, 0,
								rxbuf, sizeof(rxbuf));
	*fifo_size = rxbuf[1] << 8 | rxbuf[2];
	return status;
}

SS_STATUS SSInterface::get_num_samples_in_sensor_fifo(uint8_t sensor_id, int *fifo_size)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_INPUTFIFO, SS_CMDIDX_NUM_SAMPLES_SENSOR_FIFO, sensor_id};
	uint8_t rxbuf[3]; /* status + fifo size */

	pr_info("[Reading sensor's FIFO size for id: %d\n", sensor_id);
	SS_STATUS status = read_cmd(cmd_bytes, ARRAY_SIZE(cmd_bytes),
								NULL, 0,
								rxbuf, sizeof(rxbuf));
	*fifo_size = rxbuf[1] << 8 | rxbuf[2];
	return status;
}

SS_STATUS SSInterface::get_num_bytes_in_input_fifo(int *fifo_size)
{
	uint8_t cmd_bytes[] = { SS_FAM_R_INPUTFIFO, SS_CMDIDX_NUM_SAMPLES_INPUT_FIFO};
	uint8_t rxbuf[3]; /* status + fifo size */

	pr_info("[Reading input FIFO size for id\n");
	SS_STATUS status = read_cmd(cmd_bytes, ARRAY_SIZE(cmd_bytes),
								NULL, 0,
								rxbuf, sizeof(rxbuf),
								SS_DEFAULT2_CMD_SLEEP_MS);
	*fifo_size = rxbuf[1] << 8 | rxbuf[2];
	return status;
}

SS_STATUS SSInterface::feed_to_input_fifo(uint8_t *tx_buf, int tx_buf_sz, int *nb_written)
{
	int ret;
	uint8_t rxbuf[3];

	tx_buf[0] = SS_FAM_W_INPUTFIFO;
	tx_buf[1] = SS_CMDIDX_WRITE_FIFO;

	ret = read_cmd(&tx_buf[0], tx_buf_sz,
			NULL, 0,
			rxbuf, sizeof(rxbuf), SS_DEFAULT3_CMD_SLEEP_MS);

	*nb_written = rxbuf[1] * 256 + rxbuf[2];
	return (SS_STATUS)ret;
}

SS_STATUS SSInterface::read_fifo_data(int num_samples, int sample_size,
	uint8_t* databuf, int databuf_sz)
{
	int bytes_to_read = num_samples * sample_size + 1; //+1 for status byte
	mxm_assert_msg((bytes_to_read <= databuf_sz), "databuf too small");

	uint8_t cmd_bytes[] = { SS_FAM_R_OUTPUTFIFO, SS_CMDIDX_READFIFO };

	pr_info("[reading %d bytes (%d samples)\r\n", bytes_to_read, num_samples);

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								databuf, bytes_to_read, 10);

	return status;
}

SS_STATUS SSInterface::read_ss_log(int num_bytes, uint8_t *log_buf, int log_buf_sz)
{
	int bytes_to_read = num_bytes + 1; //+1 for status byte
	mxm_assert_msg((bytes_to_read <= log_buf_sz), "log_buf too small");

	uint8_t cmd_bytes[] = { SS_FAM_R_LOG, SS_CMDIDX_R_LOG_DATA };

	pr_info("[reading %d bytes (%d samples)\r\n", bytes_to_read, bytes_to_read);

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								log_buf, bytes_to_read, 5);

	return status;
}

static uint8_t databuf[512];
void SSInterface::ss_execute_once(){

	if(m_irq_received_ == false)
		return;

	uint8_t sample_count;
	m_irq_received_ = false;
	uint8_t cmd_bytes[] = { SS_FAM_R_STATUS, SS_CMDIDX_STATUS };
	uint8_t rxbuf[2] = {0};

	//irq_evt.start();

	disable_irq();

	SS_STATUS status = read_cmd(&cmd_bytes[0], ARRAY_SIZE(cmd_bytes),
								0, 0,
								&rxbuf[0], ARRAY_SIZE(rxbuf), SS_DEFAULT2_CMD_SLEEP_MS);
	pr_info("ss_int: %2X", rxbuf[1]);

	if (status != SS_SUCCESS) {
		pr_err("Couldn't read status byte of SmartSensor!");
		enable_irq();
		//irq_evt.stop();
		return;
	}

	if (rxbuf[1] & SS_MASK_STATUS_ERR) {
		pr_err("SmartSensor status error: %d", rxbuf[1] & SS_MASK_STATUS_ERR);
	}
	if (rxbuf[1] & SS_MASK_STATUS_FIFO_OUT_OVR) {
		pr_err("SmartSensor Output FIFO overflow!");
	}
	if (rxbuf[1] & SS_MASK_STATUS_FIFO_IN_OVR) {
		pr_err("SmartSensor Input FIFO overflow!");
	}

	if (rxbuf[1] & SS_MASK_STATUS_LOG_OVR) {
		pr_err("SmartSensor log overflow!");
	}

	if (rxbuf[1] & SS_MASK_STATUS_LOG_RDY) {
		pr_err("SmartSensor Log ready");
		int log_len;
		status = get_log_len(&log_len);
		if (status != SS_SUCCESS)
		{
			pr_err("Couldn't read log lenght");
			enable_irq();
			//irq_evt.stop();
			return;
		}

		mxm_assert_msg((log_len <= sizeof(databuf)), "log size in SS longer than buffer");
		status = read_ss_log(log_len, &databuf[0], sizeof(databuf));
		if (status != SS_SUCCESS)
		{
			pr_err("Couldn't read from SmartSensor Log");
			enable_irq();
			//irq_evt.stop();
			return;
		}

		databuf[log_len] = 0;
		Peripherals::usbSerial()->printf("\r\n%s", (char *)databuf);
	}

	if (rxbuf[1] & SS_MASK_STATUS_DATA_RDY) {
		int num_samples = 1;
		status = num_avail_samples(&num_samples);
		if (status != SS_SUCCESS)
		{
			pr_err("Couldn't read number of available samples in SmartSensor Output FIFO");
			enable_irq();
			//irq_evt.stop();
			return;
		}

		int sample_size;
		fifo_sample_size(data_type, &sample_size);

		int bytes_to_read = num_samples * sample_size + 1; //+1 for status byte
		if ((uint32_t)bytes_to_read > sizeof(databuf)) {
			//Reduce number of samples to read to fit in buffer
			num_samples = (sizeof(databuf) - 1) / sample_size;
		}

		wait_ms(5);
		status = read_fifo_data(num_samples, sample_size, &databuf[0], sizeof(databuf));
		if (status != SS_SUCCESS)
		{
			pr_err("Couldn't read from SmartSensor Output FIFO");
			enable_irq();
			//irq_evt.stop();
			return;
		}

		//Skip status byte
		uint8_t *data_ptr = &databuf[1];

		int i = 0;
		for (i = 0; i < num_samples; i++) {
			if (sc_en) {
				sample_count = *data_ptr++;
				pr_info("Received sample #%d", sample_count);
			}

			//Chop up data and send to modules with enabled sensors
			if (data_type == SS_DATATYPE_RAW || data_type == SS_DATATYPE_BOTH) {
				for (int i = 0; i < SS_MAX_SUPPORTED_SENSOR_NUM; i++) {
					if (sensor_enabled_mode[i]) {
						mxm_assert_msg(sensor_data_reqs[i],
								"no ss_data_req found for enabled sensor");
						sensor_data_reqs[i]->callback(data_ptr);
						data_ptr += sensor_data_reqs[i]->data_size;
					}
				}
			}
			if (data_type == SS_DATATYPE_ALGO || data_type == SS_DATATYPE_BOTH) {
				for (int i = 0; i < SS_MAX_SUPPORTED_ALGO_NUM; i++) {
					if (algo_enabled_mode[i]) {
					mxm_assert_msg(algo_data_reqs[i],
								"no ss_data_req found for enabled algo");
						algo_data_reqs[i]->callback(data_ptr);
						data_ptr += algo_data_reqs[i]->data_size;
					}
				}
			}
		}
	}
	enable_irq();
	//irq_evt.stop();
}

void SSInterface::ss_clear_interrupt_flag(){
	m_irq_received_ = false;
}

void SSInterface::irq_handler()
{
	m_irq_received_ = true;
}

void SSInterface::irq_handler_selftest(){
	mfio_int_happened = true;
}

bool SSInterface::reset_mfio_irq(){
	bool ret = mfio_int_happened;
	mfio_int_happened = false;
	disable_irq();
	irq_pin.fall(callback(this, &SSInterface::irq_handler));
	enable_irq();
	return ret;
}
