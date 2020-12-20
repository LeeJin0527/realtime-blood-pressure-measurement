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

#ifndef _SENSORCOMM_H_
#define _SENSORCOMM_H_

#include "mbed.h"
#include <stdint.h>
#include "MaximSensor.h"

/**
 * @brief	SensorComm is Maxim Sensor Studio GUI command handler base class.
 * @details	SensorComm includes base functions and data structures for to
 *	create new command handler classes. All command handler classes should
 *	implement this class.
 */
class SensorComm
{
public:

	//Flag to check if ascii is enabled
	static bool AsciiEn;

	const static uint8_t kMaxRegisterInOneStruct = 4;

	// size of that struct is 20 bytes(checked)
	typedef struct{
		uint8_t reg_adresses[4];
		uint32_t reg_values[4];
	}Reg_Val_BLE;

	/* PUBLIC FUNCTION DECLARATIONS */
	/**
	* @brief	SensorComm constructor.
	*
	* @param[in]	sensorType Name of sensor (ie "ppg")
	* @param[in]	visible	Whether this sensor should be visible to users in DeviceStudio
	*/
	SensorComm(const char *type, bool visible);

	/**
	* @brief	SensorComm destructor.
	*
	*/
	virtual ~SensorComm();

	/**
	* @brief	Initializer for SensorComm.
	*
	* @param[in]	s MaximSensor instance to handle sensor commands.
	*/
	void comm_init(MaximSensor *s);

	/**
	* @brief	Get the type of the sensor
	*
	* @param[out]	char* to name of type (ie "ppg")
	*/
	const char* get_type();

	/**
	* @brief	Get the maxim part name of the sensor
	*
	* @param[out]	char* to name (ie "max86140")
	*/
	virtual const char* get_part_name();

	/**
	* @brief	Get the maxim algorithm version of the sensor
	*
	* @param[out]	char* to name (ie "max86140")
	*/
	virtual const char* get_algo_ver();

	/**
	* @brief	Get the id and revision of the part
	*
	*/
	int get_part_info(uint8_t *part_id, uint8_t *part_rev);

	/**
	* @brief	Returns whether the sensor should be visible to users in DeviceStudio
	*/
	bool is_visible() { return vis; }

	/**
	* @brief	Get whether or not the sensor is enabled
	*
	* @param[out] true if the sensor is enabled
	*/
	bool is_enabled();

	/**
	* @brief	Get data report mode of the sensor
	*
	* @param[out] data report mode
	*/
	uint8_t get_data_report_mode();



	/**
	* @brief	Stop collecting data and disable sensor
	*/
	virtual void stop();

	/**
	* @brief	Parses DeviceStudio commands.
	* @details	Parses and executes commands. Prints return code to i/o device.
	*
	* @param[in] cmd - Command to parse
	*
	* @returns true if sensor acted upon the command, false if command was unknown
	*/
	virtual bool parse_command(const char* cmd);

	/**
	* @brief	 Fill in buffer with sensor data
	*
	* @param[in]	 buf Buffer to fill data into
	* @param[in]	 size Maximum size of buffer
	* @param[out]	Number of bytes written to buffer
	*/
	virtual int data_report_execute(char* buf, int size);

	/**
	 *
	 * @param status
	 */
	void SensorComm_Set_Ble_Status(bool status);

	void SensorComm_Set_Flash_Status(bool status);

	int InsertRegValuesIntoBleQeueu(addr_val_pair *reg_values, uint8_t reg_count);

	virtual unsigned char get_sensor_id();

protected:

	/* PROTECTED VARIABLES */
	Mutex comm_mutex;

	MaximSensor *sensor;

	int sensor_get_reg(char *ptr_ch, uint8_t *reg_addr, uint8_t *value);
	int sensor_set_reg(char *ptr_ch);

	volatile uint8_t data_report_mode;
	volatile uint8_t console_interface_exists;
	volatile bool m_sensorcomm_ble_interface_exists_;
	volatile bool m_sensorcomm_flash_rec_started_;

	const char* sensor_type;
	bool vis;
	int sample_count;
};

#endif /* _SENSORCOMM_H_ */
