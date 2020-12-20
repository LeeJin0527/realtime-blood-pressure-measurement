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

#ifndef _SSMAX8614XCOMM_H_
#define _SSMAX8614XCOMM_H_

#include "mbed.h"
#include "SensorComm.h"
#include "USBSerial.h"
#include "SSInterface.h"
#include "queue.h"
#include "bmi160.h"

#define	SENSOR_ID_SSMAX8614X	0x04

#define PPG_CONF_LEVEL_FLASH_ADDRESS 0x0010F100
#define PPG_EXPIRE_DURATION_FLASH_ADDRESS 0x0010F104


/**
 * @brief	SSMAX8614XComm Command handler class for communication with MAX30101 on SmartSensor board
 * @details
 */
class SSMAX8614XComm:	public SensorComm
{
public:

	/* PUBLIC FUNCTION DECLARATIONS */
	/**
	* @brief	SSMAX8614XComm constructor.
	*
	*/
	SSMAX8614XComm(USBSerial* USB, SSInterface* ssInterface, DSInterface* dsInterface);

	/**
	* @brief	Parses DeviceStudio-style commands.
	* @details  Parses and executes commands. Prints return code to i/o device.
	* @returns true if sensor acted upon the command, false if command was unknown
	*/
	bool parse_command(const char* cmd);

	/**
	 * @brief	 Fill in buffer with sensor data
	 *
	 * @param[in]	 buf Buffer to fill data into
	 * @param[in]	 size Maximum size of buffer
	 * @param[out]   Number of bytes written to buffer
	 */
	int data_report_execute(char* buf, int size);

	/**
	 * @brief	Stop collecting data and disable sensor
	 */
	void stop();

	/**
	 * @brief Get the maxim part number of the device
	 */
	const char* get_part_name() { return "max8614x"; }

	/**
	 * @brief  Execute the smart sensor self test routine
	 *
	 * @return SS_SUCCESS or error code
	 */
	int selftest_max8614x();

	/**
	 * @brief  Execute the accelerometer self test routine
	 * @return SS_SUCCESS or error code
	 */
	int selftest_accelerometer();

	/**
	 * @brief Evaluate the accelerometer self test routine
	 *
	 * @param message - message to be printed in the failure cases
	 * @param value - result of the self test passed as parameter
	 * @return true if result is SUCCESSFULL false otherwise
	 */
	bool self_test_result_evaluate(const char *message, uint8_t value);

	unsigned char get_sensor_id();

	void setBMI160(BMI160_I2C *pbmi160);

	uint16_t instant_hr;
	uint16_t instant_hr_conf;
	uint8_t  instant_hr_activityClass;

	uint16_t instant_spo2;
	uint16_t instant_spo2_conf;
	uint8_t  instant_spo2_percentage;
	uint8_t  instant_lowSignalQualityFlag;

private:
	/* PRIVATE TYPE DEFINITIONS */
	typedef enum _cmd_state_t {
		get_format_ppg_0,
		read_ppg_0,		  // moved here to match parser index "3"
		get_reg_ppg,
		set_reg_ppg,
		dump_reg_ppg,
		set_agc_dis,
		set_agc_en,
		self_test_ppg_os58,
		self_test_ppg_acc,
		//---------------------------WHRM_AEC_SCD -------	
		set_cfg_whrmaecscd_aecenable,
		get_cfg_whrmaecscd_aecenable,
		set_cfg_whrmaecscd_scdenable,
		get_cfg_whrmaecscd_scdenable,
		set_cfg_whrmaecscd_adjpdperiod,
		get_cfg_whrmaecscd_adjpdperiod,
		set_cfg_whrmaecscd_debouncewin,
		get_cfg_whrmaecscd_debouncewin,
		set_cfg_whrmaecscd_motionthreshold,
		get_cfg_whrmaecscd_motionthreshold,
		set_cfg_whrmaecscd_minpdcurrent,
		get_cfg_whrmaecscd_minpdcurrent,
		set_cfg_whrmaecscd_pdconfig,
		get_cfg_whrmaecscd_pdconfig,
		set_cfg_whrmaecscd_ledconfig,
		get_cfg_whrmaecscd_ledconfig,	
		//------------------------------------------------
		send_raw,
		set_cfg_accel_sh_dis,
		set_cfg_accel_sh_en,
		set_cfg_ppg_conf_level,
		set_cfg_ppg_expire_duration,
		NUM_CMDS,
	} cmd_state_t;


	typedef struct {
		uint32_t led1;
		uint32_t led2;
		uint32_t led3;
		uint32_t led4;
		uint32_t led5;
		uint32_t led6;
	} max8614x_mode1_data;

	typedef struct __attribute__((packed)){  
		uint16_t hr;         	// mode 1 & 2
		uint8_t hr_conf;     	// mode 1 & 2
		uint16_t rr;         	// mode 1 & 2
		uint8_t activity_class; // mode 1 & 2
		uint8_t rr_conf;		// mode 2
		uint32_t walk_steps;	// mode 2
		uint32_t run_steps;		// mode 2
		uint32_t kcal;			// mode 2
		uint32_t cadence;		// mode 2
		uint8_t is_led_cur_adj;	// mode 2
		uint16_t adj_led_cur;	// mode 2
		uint8_t is_t_int_cur_adj;// mode 2
		uint16_t adj_t_int_cur;	// mode 2
		uint8_t is_f_smp_adj;	// mode 2
		uint8_t adj_f_smp;		// mode 2
		uint8_t smp_ave;		// mode 2
		uint8_t state;			// mode 2
		uint8_t is_high_motion;	// mode 2
		uint8_t status;			// mode 2
	} whrm_modeX_data;

	typedef struct {
		int16_t x;
		int16_t y;
		int16_t z;
	} accel_mode1_data;


	typedef struct __attribute__((packed)) {
		uint32_t start_byte	:8;

		uint32_t sample_cnt	:32;
		uint32_t led1	:20;
		uint32_t led2	:20;
		uint32_t led3	:20;
		uint32_t led4	:20;
		uint32_t led5	:20;
		uint32_t led6	:20;
		uint32_t x	:14;	//Represent values of 0.000 through 8.191
		uint32_t y	:14;	//Represent values of 0.000 through 8.191
		uint32_t z	:14;	//Represent values of 0.000 through 8.191
		uint32_t hr	:12;	//Represent values of 0.0 through 204.7
		uint32_t spo2	:11;	//Represent values of 0.0 through 102.3 (only need up to 100.0)
		uint32_t status	:8;

		uint8_t	:0;			//Align CRC byte on byte boundary
		uint8_t crc8:8;
	} ds_pkt_data_mode1;


	typedef struct __attribute__((packed)) {
		uint32_t start_byte	:8;
		uint32_t sample_cnt	:16;
		uint32_t grnCnt		:20;
		uint32_t grn2Cnt	:20;
		uint32_t x	:14;				//Represent values of 0.000 through 8.191
		uint32_t y	:14;				//Represent values of 0.000 through 8.191
		uint32_t z	:14;				//Represent values of 0.000 through 8.191
		uint32_t hr	:12;				//Represent values of 0.0 through 204.7
		uint32_t hr_confidence  :8;		//Represent values of 0.0 through 100
		uint32_t rr	:11;	//Represent values of 0.0 through 102.3 (only need up to 100.0)
		uint32_t status	:8;
		uint8_t	:0;			//Align CRC byte on byte boundary
		uint8_t crc8:8;
	} ds_pkt_data_mode2_compact;

	typedef struct _accel_data_t {
		float x;
		float y;
		float z;
		int16_t x_raw;
		int16_t y_raw;
		int16_t z_raw;
	} accel_data_t;

	/* PRIVATE VARIABLES */
	int input_fifo_size;
	bool sensor_data_from_host;

	BMI160_I2C *m_bmi160;

	USBSerial *m_USB;
	SSInterface *ss_int;
	DSInterface *ds_int;

	queue_t max8614x_queue;
	uint8_t max8614x_queue_buf[64 * sizeof(max8614x_mode1_data)];

	queue_t whrm_queue;
	uint8_t whrm_queue_buf[64 * sizeof(whrm_modeX_data)]; // use the bigger of model and 2

	queue_t accel_output_queue;
	uint8_t accel_output_queue_buf[64 * sizeof(accel_mode1_data)];

	queue_t accel_input_queue;
	uint8_t accel_input_queue_buf[64 * sizeof(accel_mode1_data)];


	ss_data_req max8614x_mode1_data_req;
	ss_data_req whrm_mode1_data_req;
	ss_data_req whrm_mode2_data_req;

	ss_data_req accel_mode1_data_req;
	ss_data_req agc_mode1_data_req;


	bool agc_enabled;

	// sensor and algo status
	status_algo_sensors_st sensor_algo_en_dis_;

	/* PRIVATE CONST VARIABLES */
	static const int SSMAX8614X_REG_SIZE = 1;
	static const int SSMAX8614X_MODE1_DATASIZE = 18;	//Taken from API doc
	static const int SSWHRM_MODE1_DATASIZE = 6;			//Taken from API doc
	static const int SSWHRM_MODEX_DATASIZE  = 35;       // For WHRM_AEC_SCD
    static const int SSACCEL_MODE1_DATASIZE = 6;		//Taken from API doc
	static const int SSAGC_MODE1_DATASIZE = 0;			//Taken from API doc
	

	/* PRIVATE METHODS */
	void max8614x_data_rx(uint8_t *data_ptr);
	void whrm_data_rx_mode1(uint8_t *data_ptr);
	void whrm_data_rx_mode2(uint8_t *data_ptr);
	void accel_data_rx(uint8_t *data_ptr);
	void agc_data_rx(uint8_t *data_ptr);
	int get_sensor_xyz(accel_data_t&);
	int parse_cal_str(const char *ptr_ch, const char *cmd, uint8_t *cal_data, int cal_data_sz);
	int parse_str(const char *ptr_ch, const char *cmd, uint8_t *data, int data_sz);
};

#endif /* _SSMAX8614XCOMM_H_ */
