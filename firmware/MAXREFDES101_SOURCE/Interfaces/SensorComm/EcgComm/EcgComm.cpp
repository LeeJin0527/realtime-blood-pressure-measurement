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

#include "EcgComm.h"
#include "EcgComm_Defines.h"
#include <ctype.h>
#include "CRC8.h"
#include "Peripherals.h"
#include "utils.h"
#include "MAX30001_Helper.h"
#include "BLE_ICARUS.h"

#define ECG_REG_COUNT 64

const char *cmd_tbl_ecg[] = {
	"int",
	"get_format ecg 1",
	"get_format ecg 2",
	"read ecg 1",		//ecg
	"read ecg 2",		//android app data streaming mode
	"get_reg ecg",
	"set_reg ecg",
	"dump_reg ecg",
	"set_cfg ecg invert"
};

typedef union {
	struct {
		uint32_t ptag     		:3;
		uint32_t etag     		:3;
		uint32_t ecg_data       :16;
		uint32_t reserved       :8;
	};
	int32_t ecg_data_whole;
} ecg_data_parser;

EcgComm::EcgComm(USBSerial* USB):
	SensorComm("ecg", true)
{
	m_USB = USB;
}

void EcgComm::stop()
{
	int ret;
	comm_mutex.lock();
	data_report_mode = 0;
	comm_mutex.unlock();
	sample_count = 0;
	ret = sensor->sensor_enable(0);
	if (ret < 0) {
		pr_err("sensor_enable failed. ret: %d", ret);
	}
}

bool EcgComm::parse_command(const char* cmd)
{
	int i;
	int ret = EXIT_SUCCESS;
	uint8_t reg_addr;
	uint32_t reg_val = 0;
	bool recognizedCmd = false;
	uint8_t params[32];
	char charbuf[768];
	int data_len = 0;
	addr_val_pair reg_vals[ECG_REG_COUNT];
	int comma = 0;

	if (sensor == NULL) {
		pr_err("sensor object is invalid!");
		return false;
	}

	for (i = 0; i < NUM_CMDS; i++) {
		if (starts_with(cmd, cmd_tbl_ecg[i])) {
			cmd_state_t user_cmd = (cmd_state_t)i;
			recognizedCmd = true;
			switch (user_cmd) {
				case get_format_1:
					if(AsciiEn)
					{
						m_USB->printf("\r\n%s format=smpleCnt,ecg err=0\r\n",
								cmd);
					}
					else
					{

						if(BLE::Instance().gap().getState().connected) {
							data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s enc=bin cs=1 format={smpleCnt,8},{rtor,14},{rtorbpm,8},"
																		  "{pTag.0,3},{eTag.0,3},{ecg.0,18},"
																		  "{pTag.1,3},{eTag.1,3},{ecg.1,18},"
																		  "{pTag.2,3},{eTag.2,3},{ecg.2,18},"
																		  "{pTag.3,3},{eTag.3,3},{ecg.3,18} err=0\r\n", cmd);
						} else {
							m_USB->printf("\r\n%s enc=bin cs=1 format={smpleCnt,8},{pTag,3},{eTag,3},"
									"{ecg,18},{rtor,14},{rtorbpm,8} err=0\r\n", cmd);
						}
					}
					break;
				case get_format_2:
					if(AsciiEn)
					{
						m_USB->printf("\r\n%s format=smpleCnt,rtor err=0\r\n",
								cmd);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s enc=bin cs=1 format={smplCnt,8},{rtor,15} err=0\r\n",
								 cmd);
						m_USB->printf(charbuf);
					}
					break;
				case ecg_mode:

					comm_mutex.lock();
					data_report_mode = ecg_mode;
					comm_mutex.unlock();
					ret = sensor->sensor_enable(1);
					if (ret < 0) {
						pr_err("sensor_enable failed. ret: %d", ret);
					}

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, ret);
					m_USB->printf(charbuf);
					break;
				case ecg_mode_2:
					comm_mutex.lock();
					data_report_mode = ecg_mode_2;
					comm_mutex.unlock();
					ret = ((MAX30001_Helper*)sensor)->Max30001Helper_SetECGSampleRate(MAX30001_Helper::k128SPS);
					ret |= sensor->sensor_enable(1);
					if (ret < 0) {
						pr_err("sensor_enable failed. ret: %d", ret);
					}

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, ret);
					m_USB->printf(charbuf);
					break;

				case get_reg:
					reg_addr = 0;
					reg_val = 0;
					ret = parse_get_reg_cmd(cmd, sensor_type, &reg_addr);
					if (!ret) {
						ret = sensor->MS_max30001readRegister(reg_addr, &reg_val);
						if(ret == 0) {
							data_len = snprintf(charbuf, sizeof(charbuf),
									"\r\n%s reg_val=%08X err=%d\r\n", cmd, reg_val, ret);
							m_USB->printf(charbuf);
						}else {
							data_len = snprintf(charbuf, sizeof(charbuf),
									"\r\n%s err=%d\r\n", cmd, ret);
							m_USB->printf(charbuf);
						}
					}else{
						data_len = snprintf(charbuf, sizeof(charbuf),
								"\r\n%s err=%d\r\n", cmd, ret);
						m_USB->printf(charbuf);
					}
					break;
				case set_reg:
					reg_addr = 0;
					reg_val = 0;
					ret = parse_set_reg_cmd(cmd, sensor_type, &reg_addr, &reg_val);
					if (!ret) {
						ret = sensor->MS_max30001writeRegister(reg_addr, reg_val);
						if(ret == 0) {
							data_len = snprintf(charbuf, sizeof(charbuf),
									"\r\n%s err=%d\r\n", cmd, ret);
							m_USB->printf(charbuf);
						}else {
							data_len = snprintf(charbuf, sizeof(charbuf),
									"\r\n%s err=%d\r\n", cmd, ret);
							m_USB->printf(charbuf);
						}
					}else{
						data_len = snprintf(charbuf, sizeof(charbuf),
								"\r\n%s err=%d\r\n", cmd, ret);
						m_USB->printf(charbuf);
					}
					break;
				case dump_regs:
					for (int j = 0; j < ECG_REG_COUNT; j++) {
						reg_vals[j].addr = 0xFF;
					}
					ret = sensor->dump_registers(reg_vals);

					if (ret) {
						m_USB->printf("\r\n%s err=%d\n", cmd, ret);	
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\n", cmd, ret);
					}
					else {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s reg_val=", cmd);
						comma = false;

						for (int reg = 0; reg < ECG_REG_COUNT; reg++) {
							if(reg_vals[reg].addr == 0xFF)
								break;

							if (comma) {
                                data_len += snprintf(charbuf + data_len,
                                        sizeof(charbuf) - data_len - 1, ",");
							}
                            data_len += snprintf(charbuf + data_len,
                                    sizeof(charbuf) - data_len - 1,
                                    "{%X,%X}", (unsigned int)reg_vals[reg].addr, (unsigned int)reg_vals[reg].val);
							comma = 1;
						}
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1, " err=0\r\n");
						m_USB->printf(charbuf);
					}
					break;
				case InterruptInit:
					ret = ECG_Parse_Parameters_Single((cmd + strlen(cmd_tbl_ecg[i])), params, kInterruptParametersCount);
					if(ret != 0){
						printf("ECG_Parse_Parameters_Single has failed\n");
						break;
					}
					ret = sensor->MS_max30001_INT_assignment(params[0], params[1], params[2],
															 params[3], params[4], params[5],
															 params[6], params[7], params[8],
															 params[9], params[10], params[11],
															 params[12], params[13], params[14],
															 params[15], params[16]);
					m_USB->printf("\r\n%s err=%d\r\n", cmd, ret == 0 ? 0 : -255);
					break;

				case set_cfg_ecg_invert: {
					ret = ((MAX30001_Helper*)sensor)->MAX30001_Helper_Invert_Waveform();
					m_USB->printf("\r\n%s err=%d\r\n", cmd, ret);
				} break;

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

int EcgComm::data_report_execute(char* buf, int size)
{
	int ret = 0;
	int16_t data_len = 0;
	uint8_t tmp_report_mode;
	ecg_sensor_report sensor_report = {0};
	ecg_sensor_report sensor_report_2 = {0};
	ecg_sensor_report sensor_report_3 = {0};
	ecg_sensor_report sensor_report_4 = {0};
	ecg1_comm_packet* data_packet;
	ecg_comm_packet_ble* data_packet_ble;

	if(!is_enabled())
		return 0;

	if (sensor == NULL)
		return 0;

	comm_mutex.lock();
	tmp_report_mode = data_report_mode;
	comm_mutex.unlock();

	if(((MAX30001_Helper*)sensor)->Max30001Helper_getInterruptStatus()) {
		((MAX30001_Helper*)sensor)->Max30001Helper_max30001_int_handler();
		((MAX30001_Helper*)sensor)->Max30001Helper_setInterruptStatus(false);
	}

	switch(tmp_report_mode) {
		case ecg_mode:
		case ecg_mode_2:
if(AsciiEn){

			ret = ((MAX30001_Helper*)sensor)->get_sensor_report(sensor_report);

			if (ret < 0)
				return 0;
			data_len = snprintf(buf, size - 1,
				"%lu,%ld,%d,%d,%.3f,%.3f,%.3f\r\n",
				sample_count++,
				sensor_report.ecg,
				sensor_report.rtor,
				sensor_report.rtor_bpm,
				sensor_report.x,
				sensor_report.y,
				sensor_report.z);
}
else{
			if((m_sensorcomm_ble_interface_exists_ | m_sensorcomm_flash_rec_started_) && (((MAX30001_Helper*)sensor)->MAX30001_Helper_Queue_Size() >= m_ecg_ble_packet_count_)) {

				ret = ((MAX30001_Helper*)sensor)->get_sensor_report(sensor_report);
				ret |= ((MAX30001_Helper*)sensor)->get_sensor_report(sensor_report_2);
				ret |= ((MAX30001_Helper*)sensor)->get_sensor_report(sensor_report_3);
				ret |= ((MAX30001_Helper*)sensor)->get_sensor_report(sensor_report_4);

				if (ret < 0)
					return 0;
				data_packet_ble = (ecg_comm_packet_ble*)buf;
				data_packet_ble->start_byte = 0xAA;
				data_packet_ble->sample_count = sample_count;
				sample_count += m_ecg_ble_packet_count_;
				data_packet_ble->ecg = sensor_report.ecg;
				data_packet_ble->ecg_2 = sensor_report_2.ecg;
				data_packet_ble->ecg_3 = sensor_report_3.ecg;
				data_packet_ble->ecg_4 = sensor_report_4.ecg;
				data_packet_ble->rtor = (sensor_report.rtor | sensor_report_2.rtor | sensor_report_3.rtor | sensor_report_4.rtor);
				data_packet_ble->rtor_bpm = (sensor_report.rtor_bpm | sensor_report_2.rtor_bpm | sensor_report_3.rtor_bpm | sensor_report_4.rtor_bpm);
				data_packet_ble->crc8 = crc8((uint8_t*)data_packet_ble, sizeof(*data_packet_ble) - sizeof(uint8_t));
				data_len = sizeof(*data_packet_ble);
			} else if((m_sensorcomm_ble_interface_exists_ == 0) & (m_sensorcomm_flash_rec_started_ == 0)) {

				ret = ((MAX30001_Helper*)sensor)->get_sensor_report(sensor_report);

				if (ret < 0)
					return 0;
				data_packet = (ecg1_comm_packet*)buf;
				data_packet->start_byte = 0xAA;
				data_packet->sample_count = sample_count++;
				data_packet->ecg = sensor_report.ecg;
				data_packet->rtor = sensor_report.rtor;
				data_packet->rtor_bpm = sensor_report.rtor_bpm;
				data_packet->crc8 = crc8((uint8_t*)data_packet, sizeof(*data_packet) - sizeof(uint8_t));
				data_len = sizeof(*data_packet);
			}
}
			break;
		default:
			break;
	}

	if (data_len < 0) {
		pr_err("snprintf buf failed");
	} else if (data_len > size) {
		pr_err("buffer is insufficient to hold data");
	}

	return data_len;
}

// this function parses the parameters for max30001_ECG_InitStart function
int EcgComm::ECG_Parse_Parameters(char *substring, uint8_t parameters[], uint8_t parameters_len){
	char *pt_ch = substring;

	if(strlen(pt_ch) < (parameters_len*2)){
		pr_err("Wrong number of params");
		return -1;
	}else{
		if(ConvertHexString2Decimal(pt_ch, parameters, parameters_len) != 0)
			return -1;
		for(int i = 0; i < parameters_len; ++i){
			printf("%d\n", parameters[i]);
		}
	}
	return 0;
}

// this function parses the parameters for max30001_ECG_InitStart function for single digits
int EcgComm::ECG_Parse_Parameters_Single(const char *substring, uint8_t parameters[], uint8_t parameters_len){
	const char *pt_ch = substring;
	int i = 0;
	unsigned char num_found = 0;

	if(strlen(pt_ch) < (parameters_len)){
		pr_err("Wrong number of params");
		return -1;
	}else{
		for(i = 0; i < parameters_len; ++i){
			if(getHexDigit(pt_ch[i], &num_found)){
				parameters[i] = num_found;
			}
			else{
				pr_err("parsing of parameters failed");
				return -1;
			}
		}
		for(i = 0; i < parameters_len; ++i){
			printf("%d\n", parameters[i]);
		}
	}
	return 0;

}

char getHexDigit(char ch_hex, uint8_t *bt_hex)
{
	if (ch_hex >= '0' && ch_hex <= '9')
		*bt_hex = ch_hex - '0';
	else if (ch_hex >= 'A' && ch_hex <= 'F')
		*bt_hex = ch_hex - 'A' + 10;
	else if (ch_hex >= 'a' && ch_hex <= 'f')
		*bt_hex = ch_hex - 'a' + 10;
	else
		return false;
	return true;
}


int ConvertHexString2Decimal(char *pt_ch, uint8_t *bt_hex, int len){

	if(strlen(pt_ch) < ((uint32_t)len * 2))
		return -1;

	for(int i = 0; i < len; ++i){
		uint8_t hex_digit;
		// Get most significant hex digit
		if (!getHexDigit(*(pt_ch++), &hex_digit))
			return -1;
		else
		{
			hex_digit <<= 4;
			bt_hex[i] = hex_digit;
		}
		// Get least significant digit
		if (!getHexDigit(*(pt_ch++), &hex_digit))
			return -1;
		else{
			bt_hex[i] |= hex_digit;
		}
	}
	return 0;
}



