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

#include <ctype.h>
#include "TempComm.h"
#include "mxc_errors.h"
#include "MAX30205.h"
#include "CRC8.h"
#include "Peripherals.h"
#include "utils.h"
#include "BLE_ICARUS.h"

#define TP_REG_COUNT   64





#define MINIMUM_PERIOD_SECOND 0.5

const char *cmd_tbl_tp[] = {
	"get_format temp 0",
	"read temp 0",	/* raw */
	"set_cfg temp sr", //set sample rate
	"get_reg temp",
	"set_reg temp",
	"dump_reg temp",
};

TempComm::TempComm(USBSerial* USB): 
	SensorComm("temp", true)
{
	m_USB = USB;
	TempComm_Set_ReadTempStatus(false);
	ticker_period_second_ = MINIMUM_PERIOD_SECOND;
	sampling_period_ms_ = MINIMUM_PERIOD_SECOND * 1000;

}

void TempComm::stop()
{
	int ret;
	comm_mutex.lock();
	data_report_mode = 0;
	comm_mutex.unlock();
	ret = sensor->sensor_enable(0);
	m_tempcomm_ticker_.detach();
	TempComm_Set_ReadTempStatus(false);
	if (ret < 0) {
		pr_err("sensor_enable failed. ret: %d", ret);
	}
}

bool TempComm::parse_command(const char* cmd)
{
	int i;
	int ret = EXIT_SUCCESS;
	uint8_t reg_addr;
	uint16_t val;
	bool recognizedCmd = false;
	int data_len = 0;
	char charbuf[512];
	addr_val_pair reg_vals[TP_REG_COUNT];
	bool comma;

	if (sensor == NULL) {
		pr_err("sensor object is invalid!");
		return false;
	}

	for (i = 0; i < NUM_CMDS; i++) {
		if (starts_with(cmd, cmd_tbl_tp[i])) {
			cmd_state_t user_cmd = (cmd_state_t)i;
			recognizedCmd = true;
			switch (user_cmd) {
				case get_tp_format_mode0:
					if(AsciiEn)
					{
						m_USB->printf("\r\n%s format=smpleCnt,temp err=0\r\n",
								cmd);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s enc=bin cs=1 format={smpleCnt,8},{temp,16,2} err=0\r\n",
								cmd);
						m_USB->printf(charbuf);
					}
					break;
				case read_tp_mode0:
					comm_mutex.lock();
					data_report_mode = read_tp_mode0;
					comm_mutex.unlock();
					sample_count = 0;
					ret = sensor->sensor_enable(1);
					if (ret < 0) {
						pr_err("sensor_enable failed. ret: %d\r\n", ret);
					}

					m_tempcomm_ticker_.detach();
					m_tempcomm_ticker_.attach(callback(this,&TempComm::TempComm_Set_ReadTempStatus_Ticker), ticker_period_second_);
					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, ret);
					m_USB->printf(charbuf);
					break;
				case set_cfg_sr:
					ret = (parse_cmd_data(cmd, cmd_tbl_tp[i], &sampling_period_ms_, 1, false) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, -1);
						m_USB->printf(charbuf);
						break;
					}
					ticker_period_second_ = sampling_period_ms_/ 1000.0;
					if(ticker_period_second_ < MINIMUM_PERIOD_SECOND){
						pr_err("minimum value is less than 0.5\r\n");
						sampling_period_ms_ = MINIMUM_PERIOD_SECOND * 1000;
						ticker_period_second_ = MINIMUM_PERIOD_SECOND;
						ret = -1;
					}
					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, ret);
					m_USB->printf(charbuf);
					break;
				case get_reg:
					reg_addr = 0;
					val = 0;
					ret = parse_get_reg_cmd(cmd, sensor_type, &reg_addr);
					if (!ret) {
						ret = ((MAX30205*)sensor)->readRegister(static_cast<MAX30205::Registers_e> (reg_addr), val);
					}

					reg_vals[0].addr = reg_addr;
					reg_vals[0].val = val;
					InsertRegValuesIntoBleQeueu(reg_vals, 1);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s reg_val=%02X err=%d\r\n", cmd, val, ret);
					m_USB->printf(charbuf);
					break;
				case set_reg:
					ret = parse_set_reg_cmd(cmd, sensor_type, &reg_addr, &val);
					if (!ret) {
						ret = ((MAX30205*)sensor)->writeRegister(static_cast<MAX30205::Registers_e> (reg_addr), val);
					}

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, ret);
					m_USB->printf(charbuf);
					break;
				case dump_regs:
					for (int i = 0; i < TP_REG_COUNT; i++) {
                        reg_vals[i].addr = 0xFF;
                    }

                    ret = sensor->dump_registers(reg_vals);

                    if (ret) {
                        m_USB->printf("\r\n%s err=%d\n", cmd, ret);

                        data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\n", cmd, ret);
                    }
                    else {
                        data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s reg_val=", cmd);
                        comma = false;
                        for (int i = 0; i < TP_REG_COUNT; i++) {
                            if(reg_vals[i].addr == 0xFF)
                                break;
                            if (comma) {
                                data_len += snprintf(charbuf + data_len,
                                        sizeof(charbuf) - data_len - 1, ",");
                            }
                            data_len += snprintf(charbuf + data_len,
                                    sizeof(charbuf) - data_len - 1,
                                    "{%X,%X}", (unsigned int)reg_vals[i].addr, (unsigned int)reg_vals[i].val);
                            comma = true;
                        }
                        data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1, " err=0\r\n");

                        m_USB->printf(charbuf);

                    }

                    break;

				default:
					break;
			}


				if (BLE::Instance().gap().getState().connected) {
					BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
				}


		}
	}

	return recognizedCmd;
}

int TempComm::data_report_execute(char* buf, int size)
{
	int16_t data_len = 0;
	uint8_t tmp_report_mode;
	uint16_t tp_val;
	uint32_t ret;
	uint32_t tp_val_ext;
	float Celsius;

	tp0_comm_packet *data_packet;

	if (sensor == NULL)
		return 0;

	if(!is_enabled())
		return 0;

	comm_mutex.lock();
	tmp_report_mode = data_report_mode;
	comm_mutex.unlock();

	switch(tmp_report_mode) {
	case read_tp_mode0:
		if ( (m_can_read_temp_) ) {
			TempComm_Set_ReadTempStatus(false);
			ret = ((MAX30205*)sensor)->readTemperature(tp_val);


		    if (ret != 0)
				return 0;
			tp_val_ext = (uint32_t)tp_val;
			Celsius = ((MAX30205*)sensor)->toCelsius(tp_val_ext);
			tp_val = Celsius*100;
			TempComm_instant_temp_celsius = Celsius;

if(AsciiEn)
{
			data_len = snprintf(buf, size - 1,
					"%lu,%2.3f\r\n",
					sample_count++,
					Celsius);
}
else{
			data_packet = (tp0_comm_packet *)buf;
			data_packet->start_byte = 0xAA;
			data_packet->smpleCnt = sample_count++;
			data_packet->tp = tp_val;
			data_packet->crc8 = crc8((uint8_t*)data_packet, sizeof(*data_packet) - sizeof(uint8_t));
			data_len = sizeof(*data_packet);
}
		}
		break;

	default:
		return 0;
	}

	if (data_len < 0) {
		pr_err("snprintf buf failed");
	} else if (data_len > size) {
		pr_err("buffer is insufficient to hold data");
	}

	return data_len;
}

void TempComm::TempComm_Set_ReadTempStatus(bool en){
	m_can_read_temp_ = en;
}

void TempComm::TempComm_Set_ReadTempStatus_Ticker(){
	m_can_read_temp_ = true;
}
