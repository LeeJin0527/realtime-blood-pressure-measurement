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

#ifndef _ECGCOMM_H_
#define _ECGCOMM_H_

#include "mbed.h"
#include "SensorComm.h"
#include "USBSerial.h"

/**
 * @brief	EcgComm ECG sensor command handler class.
 * @details
 */
class EcgComm:	public SensorComm
{
public:

	static const uint8_t kEcgInitParametersCount = 11;

	static const uint8_t kRtoRInitParametersCount = 9;

	static const uint8_t kInterruptParametersCount = 17;


	/* PUBLIC FUNCTION DECLARATIONS */
	/**
	* @brief	EcgComm constructor.
	*
	*/
	EcgComm(USBSerial* USB);

	/* PROTECTED FUNCTION DECLARATIONS */
	/**
	* @brief	Parses DeviceStudio commands.
	* @details	Parses and executes commands. Prints return code to i/o device.
	* @returns true if sensor acted upon the command, false if command was unknown
	*/
	bool parse_command(const char* cmd);

	/**
	* @brief	 Fill in buffer with sensor data
	*
	* @param[in]	 buf Buffer to fill data into
	* @param[in]	 size Maximum size of buffer
	* @param[out]	Number of bytes written to buffer
	*/
	int data_report_execute(char* buf, int size);


	/**
	* @brief	Stop collecting data and disable sensor
	*/
	void stop();

	// this function parses the parameters for max30001_ECG_InitStart function
	int ECG_Parse_Parameters(char *substring, uint8_t parameters[], uint8_t parameters_len);

	// this function parses the parameters for max30001_ECG_InitStart function for single digits
	virtual int ECG_Parse_Parameters_Single(const char *substring, uint8_t parameters[], uint8_t parameters_len);


private:
	/* PRIVATE TYPE DEFINITIONS */
	typedef enum _cmd_state_t {
		InterruptInit=0,
		get_format_1,
		get_format_2,
		ecg_mode,
		ecg_mode_2,		//android app data stream mode
		get_reg,
		set_reg,
		dump_regs,
		set_cfg_ecg_invert,
		NUM_CMDS,
	} cmd_state_t;

	typedef struct __attribute__((packed)) {
		uint32_t start_byte		:8;
		uint32_t sample_count	:8;		//Represent 0-127
		uint32_t ecg			:24;	//ECG ADC data
		uint32_t rtor			:14;	//R to R data
		uint32_t rtor_bpm		:8;
		uint8_t 				:0;		//Needed to align crc to byte boundary
		uint8_t crc8			:8;
	} ecg1_comm_packet;

	typedef struct __attribute__((packed)) {
		uint32_t start_byte		:8;
		uint32_t sample_count	:8;
		uint32_t rtor			:14;	//R to R data
		uint32_t rtor_bpm		:8;
		uint32_t ecg			:24;	//ECG ADC data
		uint32_t ecg_2			:24;	//ECG ADC data 2
		uint32_t ecg_3			:24;	//ECG ADC data 2
		uint32_t ecg_4			:24;	//ECG ADC data 2
		uint8_t 				:0;		//Needed to align crc to byte boundary
		uint8_t crc8			:8;
	} ecg_comm_packet_ble;
	static const uint8_t m_ecg_ble_packet_count_ = 4;	//how many ecg info in a ble struct



	/* PRIVATE VARIABLES */
	USBSerial *m_USB;
};

char getHexDigit(char ch_hex, uint8_t *bt_hex);

int ConvertHexString2Decimal(char *ch_hex, uint8_t *bt_hex, int len);

		/*! Sensor output data to report */
typedef struct {
	int32_t ecg;
	uint16_t rtor;
	uint8_t rtor_bpm;
	uint8_t reserved;
	float x;
	float y;
	float z;
} ecg_sensor_report;

#endif /* _PPGCOMM_H_ */
