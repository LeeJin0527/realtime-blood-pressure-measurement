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

#ifndef _TEMPCOMM_H_
#define _TEMPCOMM_H_

#include "mbed.h"
#include "SensorComm.h"
#include "USBSerial.h"
#include "MAX30205.h"

/**
 * @brief	TempComm Temperature sensor command handler class.
 * @details
 */
class TempComm:	public SensorComm
{
public:

	/* PUBLIC FUNCTION DECLARATIONS */
	/**
	* @brief	TempComm constructor.
	*
	*/
	TempComm(USBSerial* USB);

	/* PROTECTED FUNCTION DECLARATIONS */
	/**
	* @brief	Parses Sensor Studio commands.
	* @details	Parses and executes commands. Prints command result to i/o device.
	* @returns true if sensor acted upon the command, false if command was unknown
	*/
	bool parse_command(const char* cmd);

	/**
	* @brief	  Fill in buffer with sensor data
	*
	* @param[in]	  buf Buffer to fill data into
	* @param[in]	  size Maximum size of buffer
	* @param[out]	Number of bytes written to buffer
	*/
	int data_report_execute(char* buf, int size);

	/**
	* @brief	Stop collecting data and disable sensor
	*/
	void stop();

	//sc... track whether the temperature can be read
	void TempComm_Set_ReadTempStatus(bool en);

	//sc... track whether the temperature can be read
	void TempComm_Set_ReadTempStatus_Ticker();

	volatile float TempComm_instant_temp_celsius;

private:
	/* PRIVATE TYPE DEFINITIONS */
	typedef enum _cmd_state_t {
		get_tp_format_mode0=0,
		read_tp_mode0,
		set_cfg_sr,
		get_reg,
		set_reg,
		dump_regs,
		NUM_CMDS,
	} cmd_state_t;

	typedef struct __attribute__((packed)) {
		uint32_t start_byte :8;
		uint32_t smpleCnt	:8;	 //Represent 0-127
		uint32_t tp			:16; //TP ADC data
		uint8_t				:0;	 //Needed to align crc to byte boundary
		uint8_t crc8		:8;
	} tp0_comm_packet;

	/* PRIVATE VARIABLES */
	USBSerial *m_USB;
	volatile char m_can_read_temp_;
	Ticker m_tempcomm_ticker_;
	// read temp ticker settings
	float ticker_period_second_;
	uint32_t sampling_period_ms_;


};

#endif /* _PPGCOMM_H_ */
