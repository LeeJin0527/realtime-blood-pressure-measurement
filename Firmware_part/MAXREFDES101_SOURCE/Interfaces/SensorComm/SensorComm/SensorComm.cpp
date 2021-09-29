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
#include "SensorComm.h"
#include "Peripherals.h"
#include "BLE_ICARUS.h"

//define and initialize the static AsciiEn which is declared in the header
bool SensorComm::AsciiEn=false;

SensorComm::SensorComm(const char *type, bool visible)
{
	sensor = NULL;
	sensor_type = type;
	vis = visible;
}

SensorComm::~SensorComm()
{
}

void SensorComm::comm_init(MaximSensor *s)
{
	sensor = s;
}

const char* SensorComm::get_type()
{
	return sensor_type;
}

unsigned char SensorComm::get_sensor_id() {

	if (sensor!= NULL)
		return( sensor->get_sensor_id());
	else
		return(255);
}

const char* SensorComm::get_part_name()
{
	if (sensor != NULL)
	{
		return sensor->get_sensor_part_name();
	}
	else
	{
		return "unknown";
	}
}

const char* SensorComm::get_algo_ver()
{
	if (sensor != NULL)
	{
		return sensor->get_sensor_algo_ver();
	}
	else
	{
		return "unknown";
	}
}

int SensorComm::get_part_info(uint8_t *part_id, uint8_t *rev_id)
{
	if (sensor != NULL) {
		return sensor->get_part_info(part_id, rev_id);
	} else {
		*part_id = 0xFF;
		*rev_id = 0xFF;
		return -1;
	}
}

bool SensorComm::is_enabled()
{
	return (data_report_mode != 0);
}

uint8_t SensorComm::get_data_report_mode()
{
	return (data_report_mode);
}


int SensorComm::sensor_get_reg(char *ptr_ch, uint8_t *reg_addr, uint8_t *value)
{
	int ret = EXIT_FAILURE;
	uint8_t num_found;
	int which_reg;
	*value = 0;

	while (*ptr_ch) {
		if (isxdigit((int)*ptr_ch)) {
			num_found = (uint8_t)sscanf(ptr_ch, "%x", &which_reg);
			if (num_found == 1) {
				*reg_addr = which_reg;
				ret = sensor->readRegister(*reg_addr, value, 1);
			}
			break;
		} else {
			ptr_ch++;
		}
	}
	return ret;
}

int SensorComm::sensor_set_reg(char *ptr_ch)
{
	int ret = EXIT_FAILURE;
	uint8_t num_found;
	unsigned int reg_addr, value;
	char *ptr_char;

	ptr_char = ptr_ch;
	while (*ptr_char) {
		if (isxdigit((int)*ptr_char)) {
			num_found = (uint8_t)sscanf(ptr_char, "%x %x", &reg_addr, &value);
			if (num_found == 2) {
				ret = sensor->writeRegister(reg_addr, (uint8_t)value);
			}
			break;
		} else {
			ptr_char++;
		}
	}
	return ret;
}


int SensorComm::InsertRegValuesIntoBleQeueu(addr_val_pair *reg_values, uint8_t reg_count){
	Reg_Val_BLE values = {0};
	int ret = 0;

	if (!BLE::Instance().gap().getState().connected)
		return 0;

	while(reg_count >= kMaxRegisterInOneStruct){
		for(uint8_t i = 0; i < kMaxRegisterInOneStruct; ++i){
			values.reg_adresses[i] = reg_values[i].addr;
			values.reg_values[i] = reg_values[i].val;
		}
		ret |= BLE_Icarus_AddtoQueue((uint8_t *)(&values), sizeof(Reg_Val_BLE), sizeof(Reg_Val_BLE));
		reg_count -= kMaxRegisterInOneStruct;
	}

	if(reg_count){
		for(uint8_t i = 0; i < reg_count; ++i){
			values.reg_adresses[i] = reg_values[i].addr;
			values.reg_values[i] = reg_values[i].val;
		}
		ret |= BLE_Icarus_AddtoQueue((uint8_t *)(&values), sizeof(Reg_Val_BLE), sizeof(Reg_Val_BLE));
	}

	return ret;
}

void SensorComm::stop() { }

bool SensorComm::parse_command(const char* cmd) { return false; }

int SensorComm::data_report_execute(char* buf, int size) { return 0; }

void SensorComm::SensorComm_Set_Ble_Status(bool status) {
	m_sensorcomm_ble_interface_exists_ = status;
}

void SensorComm::SensorComm_Set_Flash_Status(bool status) {
	m_sensorcomm_flash_rec_started_ = status;
}
