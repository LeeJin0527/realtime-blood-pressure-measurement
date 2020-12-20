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
#include "SSMAX8614XComm.h"

#include "../../../../Utilities/mxm_assert.h"
#include "flc.h"
#include "flc_regs.h"
#include "Peripherals.h"
#include "utils.h"
#include "CRC8.h"


#include "BLE_ICARUS.h"




// By default, host accel is enabled

//#define FORCE_RESET_WHEN_FAILED

#define MAX_NUM_WR_ACC_SAMPLES			5
#define BMI160_SAMPLE_RATE				25

static const char* const cmd_tbl[] = {
    "get_format ppg 0",
    "read ppg 0", 		// moved here to match parser index "3"
	"get_reg ppg",
	"set_reg ppg",
	"dump_reg ppg",
	"set_cfg ppg agc 0",
	"set_cfg ppg agc 1",
	"self_test ppg os58",
	"self_test ppg acc",
	"set_cfg whrmaecscd aecenable",
	"get_cfg whrmaecscd aecenable",
	"set_cfg whrmaecscd scdenable",
	"get_cfg whrmaecscd scdenable",
	"set_cfg whrmaecscd adjpdperiod",
	"get_cfg whrmaecscd adjpdperiod",
	"set_cfg whrmaecscd debouncewin",
	"get_cfg whrmaecscd debouncewin",
	"set_cfg whrmaecscd motionthreshold",
	"get_cfg whrmaecscd motionthreshold",
	"set_cfg whrmaecscd minpdcurrent",
	"get_cfg whrmaecscd minpdcurrent",
	"set_cfg whrmaecscd pdconfig",
	"get_cfg whrmaecscd pdconfig",
	"set_cfg whrmaecscd ledconfig",
	"get_cfg whrmaecscd ledconfig",	
	"send_raw",  // write raw data to I2C (Slave address will be perpended)
	"set_cfg accel_sh 0",   // disables sensorhub accel, host accel is used 
	"set_cfg accel_sh 1",   // enables sensorhub accel, host accel is not used
	"set_cfg whrm conf_level", //sets the conf level coming from the GUI inside the internal flash
	"set_cfg whrm hr_expiration", ////sets the expiration dfuration coming from the GUI inside the internal flash

};

SSMAX8614XComm::SSMAX8614XComm(USBSerial *USB, SSInterface* ssInterface, DSInterface* dsInterface)
    :SensorComm("ppg", true), m_USB(USB), ss_int(ssInterface), ds_int(dsInterface), agc_enabled(true)
{
	max8614x_mode1_data_req.data_size = SSMAX8614X_MODE1_DATASIZE;
	max8614x_mode1_data_req.callback = callback(this, &SSMAX8614XComm::max8614x_data_rx);

	whrm_mode1_data_req.data_size = SSWHRM_MODE1_DATASIZE;
	whrm_mode1_data_req.callback = callback(this, &SSMAX8614XComm::whrm_data_rx_mode1);

	whrm_mode2_data_req.data_size = SSWHRM_MODEX_DATASIZE;
	whrm_mode2_data_req.callback = callback(this, &SSMAX8614XComm::whrm_data_rx_mode2);
	
	accel_mode1_data_req.data_size = SSACCEL_MODE1_DATASIZE;
	accel_mode1_data_req.callback = callback(this, &SSMAX8614XComm::accel_data_rx);

	agc_mode1_data_req.data_size = SSAGC_MODE1_DATASIZE;
	agc_mode1_data_req.callback = callback(this, &SSMAX8614XComm::agc_data_rx);

	queue_init(&max8614x_queue, max8614x_queue_buf, sizeof(max8614x_mode1_data), sizeof(max8614x_queue_buf));
	queue_init(&whrm_queue, whrm_queue_buf, sizeof(whrm_modeX_data), sizeof(whrm_queue_buf));

	queue_init(&accel_output_queue, accel_output_queue_buf, sizeof(accel_mode1_data), sizeof(accel_output_queue_buf));
	queue_init(&accel_input_queue, accel_input_queue_buf, sizeof(accel_mode1_data), sizeof(accel_input_queue_buf));

	
	sensor_algo_en_dis_.sensorhub_accel = 1;   // enable sensor hub accel by default

    m_bmi160 = 0;

	input_fifo_size = 0;
	sensor_data_from_host = false;
}

void SSMAX8614XComm::stop()
{
	status_algo_sensors_st sensor_algo_en_dis_temp;
	comm_mutex.lock();
	ss_int->disable_irq();
	data_report_mode = 0;
	sample_count = 0;

	if (sensor_algo_en_dis_.max8614x_enabled) {
		ss_int->disable_sensor(SS_SENSORIDX_MAX86140);
		queue_reset(&max8614x_queue);
	}

	if (sensor_algo_en_dis_.accel_enabled) {
		ss_int->disable_sensor(SS_SENSORIDX_ACCEL);
		queue_reset(&accel_output_queue);
	}

	if (sensor_algo_en_dis_.whrm_enabled) {
		ss_int->disable_algo(SS_ALGOIDX_WHRM);
		queue_reset(&whrm_queue);
	}

	// store a copy of settings
	sensor_algo_en_dis_temp.status_vals = sensor_algo_en_dis_.status_vals;
	// clear all settings
	sensor_algo_en_dis_.status_vals = 0;
	//recover settings that need to be maintained
	sensor_algo_en_dis_.sensorhub_accel = sensor_algo_en_dis_temp.sensorhub_accel;
	ss_int->ss_clear_interrupt_flag();
	ss_int->enable_irq();

	if(sensor_algo_en_dis_.sensorhub_accel==0){
		m_bmi160->reset();
		queue_reset(&accel_input_queue);
	}


	comm_mutex.unlock();
	input_fifo_size = 0;
	sensor_data_from_host = false;
}

int SSMAX8614XComm::parse_cal_str(const char *ptr_ch, const char *cmd, uint8_t *cal_data, int cal_data_sz)
{
	char ascii_byte[] = { 0, 0, 0 };
	const char* sptr = ptr_ch + strlen(cmd);
	int found = 0;
	int ssfound;
	unsigned int val32;

	//Eat spaces after cmd
	while (*sptr != ' ' && *sptr != '\0') { sptr++; }
	if (*sptr == '\0')
		return -1;
	sptr++;

	while (found < cal_data_sz) {
		if (*sptr == '\0')
			break;
		ascii_byte[0] = *sptr++;
		ascii_byte[1] = *sptr++;
		ssfound = sscanf(ascii_byte, "%x", &val32);
		if (ssfound != 1)
			break;
		*(cal_data + found) = (uint8_t)val32;
		//pr_err("cal_data[%d]=%d\r\n", found, val32);
		found++;
	}

	//pr_err("total found: %d\r\n", found);
	if (found < cal_data_sz)
		return -1;
	return 0;
}

int SSMAX8614XComm::parse_str(const char *ptr_ch, const char *cmd, uint8_t *data, int data_sz)
{
	char ascii_byte[] = { 0, 0, 0 };
	const char* sptr = ptr_ch + strlen(cmd);
	int found = 0;
	int ssfound;
	unsigned int val32;

	//Eat spaces after cmd
	while (*sptr != ' ' && *sptr != '\0') { sptr++; }
	if (*sptr == '\0')
		return -1;
	sptr++;

	while (found < data_sz) {
		if (*sptr == '\0')
			break;
		ascii_byte[0] = *sptr++;
		ascii_byte[1] = *sptr++;
		ssfound = sscanf(ascii_byte, "%x", &val32);
		if (ssfound != 1)
			return -1;
		*(data + found) = (uint8_t)val32;
		pr_err("cal_data[%d]=%d\r\n", found, val32);
		found++;
	}

	pr_err("total found: %d\r\n", found);
	return found;
}


void SSMAX8614XComm::setBMI160(BMI160_I2C *pbmi160)
{
	if (pbmi160) {
		m_bmi160 = pbmi160;
		m_bmi160->reset();
	}
}


int SSMAX8614XComm::get_sensor_xyz(accel_data_t &accel_data) {
	int ret = 0;
if(sensor_algo_en_dis_.sensorhub_accel==0){
	BMI160::SensorData stacc_data = {0};
	if (m_bmi160) {
		ret = m_bmi160->getSensorXYZ(stacc_data, BMI160::SENS_2G);
		if (ret < 0)
			return ret;
	}

	accel_data.x = stacc_data.xAxis.scaled;
	accel_data.y = stacc_data.yAxis.scaled;
	accel_data.z = stacc_data.zAxis.scaled;

	accel_data.x_raw = stacc_data.xAxis.raw;
	accel_data.y_raw = stacc_data.yAxis.raw;
	accel_data.z_raw = stacc_data.zAxis.raw;

}

	return ret;
}

bool SSMAX8614XComm::parse_command(const char* cmd)
{
	int ret;
    SS_STATUS status;
    bool recognizedCmd = false;
	int data_len = 0;
	char charbuf[768];
	addr_val_pair reg_vals[64];

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
				case set_cfg_accel_sh_dis:
				{
					sensor_algo_en_dis_.sensorhub_accel = 0;
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);

				} break;
				case set_cfg_accel_sh_en:
				{
					sensor_algo_en_dis_.sensorhub_accel = 1;
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);

				} break;
				case set_cfg_ppg_conf_level:
				{
					int j=24;
					char temp_buf[4];
					uint32_t PpgConfLevel[2];
					while(cmd[j]!='\0'){
						temp_buf[j-24] = cmd[j];
						j++;
					}
					temp_buf[j-24]='\0';
					sscanf(temp_buf,"%u",&PpgConfLevel[0]);

					PpgConfLevel[1] = (*(uint32_t *)PPG_EXPIRE_DURATION_FLASH_ADDRESS);

					if(FLC_Init()!=0){
						m_USB->printf("\r\n%s err=Initializing flash\r\n", cmd);
						break;
					}
					if( FLC_PageErase(PPG_CONF_LEVEL_FLASH_ADDRESS, MXC_V_FLC_ERASE_CODE_PAGE_ERASE, MXC_V_FLC_FLSH_UNLOCK_KEY)!=0 ){
						m_USB->printf("\r\n%s err=Erasing page\r\n", cmd);
						break;
					}
					if(FLC_Write(PPG_CONF_LEVEL_FLASH_ADDRESS,PpgConfLevel,sizeof(PpgConfLevel),MXC_V_FLC_FLSH_UNLOCK_KEY)!=0){
						m_USB->printf("\r\n%s err=Writing flash\r\n", cmd);
						break;
					}
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);

				} break;
				case set_cfg_ppg_expire_duration:
				{
					int j=27;
					char temp_buf[4];
					uint32_t PpgExpireDuration[2];
					while(cmd[j]!='\0'){
						temp_buf[j-27] = cmd[j];
						j++;
					}
					temp_buf[j-27]='\0';
					sscanf(temp_buf,"%u",&PpgExpireDuration[1]);

					PpgExpireDuration[0] = (*(uint32_t *)PPG_CONF_LEVEL_FLASH_ADDRESS);

					if(FLC_Init()!=0){
						m_USB->printf("\r\n%s err=Initializing flash\r\n", cmd);
						break;
					}
					if( FLC_PageErase(PPG_EXPIRE_DURATION_FLASH_ADDRESS, MXC_V_FLC_ERASE_CODE_PAGE_ERASE, MXC_V_FLC_FLSH_UNLOCK_KEY)!=0 ){
						m_USB->printf("\r\n%s err=Erasing page\r\n", cmd);
						break;
					}
					if(FLC_Write(PPG_CONF_LEVEL_FLASH_ADDRESS,PpgExpireDuration,sizeof(PpgExpireDuration),MXC_V_FLC_FLSH_UNLOCK_KEY)!=0){
						m_USB->printf("\r\n%s err=Writing flash\r\n", cmd);
						break;
					}
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);

				} break;
				case get_format_ppg_0:
				{
					if(AsciiEn)
					{
						if (ds_int->algo_report_mode == 1){
							m_USB->printf("\r\n%s format=smpleCnt,grnCnt,led2,led3,grn2Cnt,led5,led6,"
									"accelX,accelY,accelZ,hr,hrconf,r,activity err=0\r\n", cmd);
						}
						else { // report mode 2
							m_USB->printf("\r\n%s format=smpleCnt,grnCnt,led2,led3,grn2Cnt,led5,led6,"
									"accelX,accelY,accelZ,hr,hrconf,r,rconf,activity,walkSteps,runSteps,"
									"energyKcal,stepCadence,isLedCurrentAdj,adjLedCurrent,isTAdj,adjT,isFAdj,"
									"adjF,aecSmpAve,aecState,isAECMotion,scdStatus err=0\r\n", cmd);
						}
					}
					else{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s enc=bin cs=1 format={smpleCnt,16},"
								"{grnCnt,20},{grn2Cnt,20},{accelX,14,3},{accelY,14,3},"
								"{accelZ,14,3},{hr,12},{hrconf,8},{r,11,1},{activity,8} err=0\r\n", cmd);
						m_USB->printf(charbuf);
					}
				} break;

				case read_ppg_0:
				{
					sample_count = 0;

					status = ss_int->get_input_fifo_size(&input_fifo_size);
					if (status != SS_SUCCESS) {
						data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d\n", __LINE__);
						m_USB->printf(charbuf);
						break;
					}

					status = ss_int->set_data_type(SS_DATATYPE_BOTH, false);
					if (status != SS_SUCCESS) {
						data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d, set DATA_TYPE\n, ", __LINE__);
#ifdef FORCE_RESET_WHEN_FAILED
							ret = ss_int->reset();
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
									"\r\nReset!\r\n");
#endif
						m_USB->printf(charbuf);
						break;
					}

					status = ss_int->set_fifo_thresh(5);
					if (status != SS_SUCCESS) {
						data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d, set thresh\n", __LINE__);
#ifdef FORCE_RESET_WHEN_FAILED
							ret = ss_int->reset();
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
									"\r\nReset!\r\n");
#endif
						m_USB->printf(charbuf);
						break;
					}


					if (status != SS_SUCCESS) {
						data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d - agc_enabled: %d\n", __LINE__, agc_enabled);
						ss_int->enable_irq();

#ifdef FORCE_RESET_WHEN_FAILED
							ret = ss_int->reset();
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
									"\r\nReset!\r\n");
#endif
						m_USB->printf(charbuf);
						break;
					}

					ss_int->disable_irq();

					status = ss_int->enable_sensor(SS_SENSORIDX_MAX86140, 1, &max8614x_mode1_data_req);
					if (status != SS_SUCCESS) {
						data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d, enable max86140\n", __LINE__);
						ss_int->enable_irq();
	#ifdef FORCE_RESET_WHEN_FAILED
						ret = ss_int->reset();
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"\r\nReset!\r\n");
	#endif
						m_USB->printf(charbuf);
						break;
					}
					sensor_algo_en_dis_.max8614x_enabled = 1;



					if (sensor_algo_en_dis_.sensorhub_accel) {
						status = ss_int->enable_sensor(SS_SENSORIDX_ACCEL, 1, &accel_mode1_data_req, SH_INPUT_DATA_DIRECT_SENSOR);
						if (status != SS_SUCCESS) {
							data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d\n", __LINE__);
							ss_int->enable_irq();
	#ifdef FORCE_RESET_WHEN_FAILED
							ret = ss_int->reset();
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"\r\nReset!\r\n");
	#endif
							m_USB->printf(charbuf);
							break;
						}
						sensor_algo_en_dis_.accel_enabled = 1;
					}
					else {
						m_bmi160->BMI160_DefaultInitalize();
						if (m_bmi160->setSampleRate(BMI160_SAMPLE_RATE) != 0) {
							pr_err("Unable to set BMI160's sample rate\n");
							data_len = snprintf(charbuf , sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d\n", __LINE__);
							ss_int->enable_irq();
	#ifdef FORCE_RESET_WHEN_FAILED
							ret = ss_int->reset();
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"\r\nReset!\r\n");
	#endif
							m_USB->printf(charbuf);
							break;
						}

						ret = m_bmi160->enable_data_ready_interrupt();
						if(ret != E_SUCCESS){
							pr_err("Unable to enable BMI160 Interrupt, ret: %d\n", ret);
							break;
						}



						status = ss_int->enable_sensor(SS_SENSORIDX_ACCEL, 1, &accel_mode1_data_req, SH_INPUT_DATA_FROM_HOST);
						if (status != SS_SUCCESS) {
							data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d\n", __LINE__);
							ss_int->enable_irq();
	#ifdef FORCE_RESET_WHEN_FAILED
							ret = ss_int->reset();
							data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"\r\nReset!\r\n");
	#endif
							m_USB->printf(charbuf);
							break;
						}
						sensor_data_from_host = true;
						sensor_algo_en_dis_.accel_enabled = 1;
					}


					if (ds_int->algo_report_mode == 2)
						status = ss_int->enable_algo(SS_ALGOIDX_WHRM, 2, &whrm_mode2_data_req);
					else // default is mode 1
						status = ss_int->enable_algo(SS_ALGOIDX_WHRM, 1, &whrm_mode1_data_req);
					if (status != SS_SUCCESS) {
						data_len = snprintf(charbuf, sizeof(charbuf) - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"FAILED at line %d, enable whrm\n", __LINE__);
						ss_int->enable_irq();
	#ifdef FORCE_RESET_WHEN_FAILED
						ret = ss_int->reset();
						data_len += snprintf(charbuf+data_len, sizeof(charbuf)- data_len - 1,
								"\r\nReset!\r\n");
	#endif
						m_USB->printf(charbuf);
						break;
					}
					sensor_algo_en_dis_.whrm_enabled = 1;


					comm_mutex.lock();
					data_report_mode = read_ppg_0;
					comm_mutex.unlock();
					data_len = snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
					m_USB->printf(charbuf);
					ss_int->enable_irq();
				} break;

				case get_reg_ppg:
				{
					uint8_t addr;
					uint32_t val;

					ret = parse_get_reg_cmd(cmd, sensor_type, &addr);
					if (!ret) {
						status = ss_int->get_reg(SS_SENSORIDX_MAX86140, addr, &val);
						if (status == SS_SUCCESS) {
							data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s reg_val=%02X err=%d\r\n", cmd, (uint8_t)val, COMM_SUCCESS);
							m_USB->printf(charbuf);
						} else {
							data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
							m_USB->printf(charbuf);
						}
					} else {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
					}

				} break;

				case set_reg_ppg:
				{
					uint8_t addr;
					uint8_t val;

					ret = parse_set_reg_cmd(cmd, sensor_type, &addr, &val);
					if (!ret) {
						status = ss_int->set_reg(SS_SENSORIDX_MAX86140, addr, val, SSMAX8614X_REG_SIZE);
						if (status == SS_SUCCESS) {
							data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
							m_USB->printf(charbuf);
						} else {
							data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
							m_USB->printf(charbuf);
						}
					} else {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
					}

				} break;

				case dump_reg_ppg:
				{
					int num_regs;
					status = ss_int->dump_reg(SS_SENSORIDX_MAX86140, &reg_vals[0], ARRAY_SIZE(reg_vals), &num_regs);
					if (status == SS_SUCCESS) {
						bool comma = false;
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1, "\r\n%s reg_val=", cmd);
						for (int reg = 0; reg < num_regs; reg++) {
							if (comma) {
								data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1, ",");
							}
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1,
											"{%X,%lX}", reg_vals[reg].addr, reg_vals[reg].val);
							comma = true;
						}
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1, " err=%d\r\n", COMM_SUCCESS);
						m_USB->printf(charbuf);

					} else {
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1,
								"\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case set_agc_en:
				{
					agc_enabled = true;
					status = ss_int->enable_algo(SS_ALGOIDX_AGC, 1, &agc_mode1_data_req);
					sensor_algo_en_dis_.agc_enabled = 1;
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				} break;

				case set_agc_dis:
				{
					agc_enabled = false;
					status = ss_int->disable_algo(SS_ALGOIDX_AGC);
					sensor_algo_en_dis_.agc_enabled = 0;
					m_USB->printf("\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
				} break;

				case self_test_ppg_os58:
				{
					ret = selftest_max8614x();
					data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1,
							"%s selftest_max8614x: err=<%d>\r\n", cmd, ret);
					m_USB->printf(charbuf);
				} break;

				case self_test_ppg_acc:
				{
					ret = selftest_accelerometer();
					data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len - 1,
							"%s selftest_accelerometer: err=<%d>\r\n", cmd, ret);
					m_USB->printf(charbuf);
				} break;

				//--------- WHRM_AEC_SCD AEC ENABLE
				case set_cfg_whrmaecscd_aecenable:
				{
					uint8_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[1] = { (uint8_t)(val[0]) };

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_AEC_ENABLE, &Temp[0], 1);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_aecenable:
				{
					uint8_t rxBuff[1+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_AEC_ENABLE, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//--------- WHRM_AEC_SCD SCD ENABLE
				case set_cfg_whrmaecscd_scdenable:
				{
					uint8_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[1] = { (uint8_t)(val[0]) };

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_SCD_ENABLE, &Temp[0], 1);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_scdenable:
				{
					uint8_t rxBuff[1+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_SCD_ENABLE, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//--------- WHRM_AEC_SCD ADJ TARGET PD PERIOD
				case set_cfg_whrmaecscd_adjpdperiod:
				{
					uint16_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[2] = { (uint8_t)((val[0] >> (1*8)) & 0xFF),  (uint8_t)((val[0] >> (0*8)) & 0xFF)};

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_ADJ_TARGET_PD_CURRENT_PERIOD, &Temp[0], 2);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_adjpdperiod:
				{
					uint8_t rxBuff[2+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_ADJ_TARGET_PD_CURRENT_PERIOD, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//--------- WHRM_AEC_SCD DEBOUNCE WINDOW
				case set_cfg_whrmaecscd_debouncewin:
				{
					uint16_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[2] = { (uint8_t)((val[0] >> (1*8)) & 0xFF),  (uint8_t)((val[0] >> (0*8)) & 0xFF)};

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_SCD_DEBOUNCE_WINDOW, &Temp[0], 2);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_debouncewin:
				{
					uint8_t rxBuff[2+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_SCD_DEBOUNCE_WINDOW, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//--------- WHRM_AEC_SCD MOTION MAG THRESHOLD
				case set_cfg_whrmaecscd_motionthreshold:
				{
					uint16_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[2] = { (uint8_t)((val[0] >> (1*8)) & 0xFF),  (uint8_t)((val[0] >> (0*8)) & 0xFF)};

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_MOTION_MAG_THRESHOLD, &Temp[0], 2);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_motionthreshold:
				{
					uint8_t rxBuff[2+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_MOTION_MAG_THRESHOLD, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//--------- WHRM_AEC_SCD MIN PD CURRENT
				case set_cfg_whrmaecscd_minpdcurrent:
				{
					uint16_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[2] = { (uint8_t)((val[0] >> (1*8)) & 0xFF),  (uint8_t)((val[0] >> (0*8)) & 0xFF)};

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_MIN_PD_CURRENT, &Temp[0], 2);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_minpdcurrent:
				{
					uint8_t rxBuff[2+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_MIN_PD_CURRENT, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//--------- WHRM_AEC_SCD PD CONFIG
				case set_cfg_whrmaecscd_pdconfig:
				{
					uint8_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[1] = { (uint8_t)(val[0]) };

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_PD_CONFIG, &Temp[0], 1);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_pdconfig:
				{
					uint8_t rxBuff[1+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_PD_CONFIG, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//--------- WHRM_AEC_SCD LED CONFIG
				case set_cfg_whrmaecscd_ledconfig:
				{
					uint8_t val[1];
					ret = (parse_cmd_data(cmd, cmd_tbl[i], &val[0], 1, true) != 1);
					if (ret) {
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_INVALID_PARAM);
						m_USB->printf(charbuf);
						break;
					}

					uint8_t Temp[1] = { (uint8_t)(val[0]) };

					status = ss_int->set_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_LED_CONFIG, &Temp[0], 1);
					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;

				case get_cfg_whrmaecscd_ledconfig:
				{
					uint8_t rxBuff[1+1];  // first byte is status
					char outstr[2*sizeof(rxBuff)];
					int str_idx = 0;
					uint8_t *Temp = &rxBuff[1];
					uint8_t val[1];

					status = ss_int->get_algo_cfg(SS_ALGOIDX_WHRM, SS_CFGIDX_WHRM_LED_CONFIG, &rxBuff[0], sizeof(rxBuff));
					for (int i = 0; i < sizeof(rxBuff)-1; i++)
						str_idx += snprintf(outstr + str_idx, sizeof(outstr) - str_idx - 1, "%02X", Temp[i]);

					data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s value=%s err=%d\r\n", cmd, outstr, COMM_SUCCESS);
					m_USB->printf(charbuf);

				} break;

				//---------  SEND RAW
				case send_raw:
				{
					int size=0;
					uint8_t data[256];  // max size of command

					size = parse_str(cmd, cmd_tbl[i], data, sizeof(data));
					status = ss_int->send_raw(data, size);

					if (status == SS_SUCCESS)
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_SUCCESS);
						m_USB->printf(charbuf);
					}
					else
					{
						data_len = snprintf(charbuf, sizeof(charbuf), "\r\n%s err=%d\r\n", cmd, COMM_GENERAL_ERROR);
						m_USB->printf(charbuf);
					}

				} break;
				//--------------------------------------------------------

                default:
                {
                    mxm_assert_msg(false, "Invalid switch case!");
                } break;

            }
			

	if (BLE::Instance().gap().getState().connected) {
		BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
	}


            break;

        }
    }

    return recognizedCmd;
}

void SSMAX8614XComm::max8614x_data_rx(uint8_t* data_ptr)
{
	max8614x_mode1_data sample;
	sample.led1 = (data_ptr[0] << 16) | (data_ptr[1] << 8) | data_ptr[2];
	sample.led2 = (data_ptr[3] << 16) | (data_ptr[4] << 8) | data_ptr[5];
	sample.led3 = (data_ptr[6] << 16) | (data_ptr[7] << 8) | data_ptr[8];
	sample.led4 = (data_ptr[9] << 16) | (data_ptr[10] << 8) | data_ptr[11];
	sample.led5 = (data_ptr[12] << 16) | (data_ptr[13] << 8) | data_ptr[14];
	sample.led6 = (data_ptr[15] << 16) | (data_ptr[16] << 8) | data_ptr[17];

	pr_info("led1=%.6X led2=%.6X led3=%.6X led4=%.6X led5=%.6X led6=%.6X\r\n",
			sample.led1, sample.led2, sample.led3, sample.led4, sample.led5, sample.led6);

	enqueue(&max8614x_queue, &sample);
}

void SSMAX8614XComm::whrm_data_rx_mode1(uint8_t* data_ptr)
{
	whrm_modeX_data sample;

	sample.hr = (data_ptr[0] << 8) | data_ptr[1];
	sample.hr_conf = data_ptr[2];
	sample.rr = (data_ptr[3] << 8) | data_ptr[4];
	sample.activity_class = data_ptr[5];
	//m_USB->printf("\r\n Md1- hr:%d  conf:%d  rr:%d  act:%d\r\n", sample.hr,sample.hr_conf,sample.rr,sample.activity_class );
	enqueue(&whrm_queue, &sample);
	pr_info("hr=%.1f conf=%d rr=%.1f rr_conf=%d status=%d\r\n", (float)sample.hr / 10.0, sample.hr_conf, (float)sample.rr/10, sample.rr_conf,sample.activity_class);	
}

void SSMAX8614XComm::whrm_data_rx_mode2(uint8_t* data_ptr)
{
	whrm_modeX_data sample;

	sample.hr = (data_ptr[0] << 8) | data_ptr[1];
	sample.hr_conf = data_ptr[2];
	sample.rr = (data_ptr[3] << 8) | data_ptr[4];
	sample.activity_class = data_ptr[5];
	sample.rr_conf = data_ptr[6];	
	sample.walk_steps = (data_ptr[7] << 24) | (data_ptr[8] << 16) |(data_ptr[9] << 8) | data_ptr[10];
	sample.run_steps = (data_ptr[11] << 24) | (data_ptr[12] << 16) |(data_ptr[13] << 8) | data_ptr[14];			
	sample.kcal = (data_ptr[15] << 24) | (data_ptr[16] << 16) |(data_ptr[17] << 8) | data_ptr[18];			
	sample.cadence = (data_ptr[19] << 24) | (data_ptr[20] << 16) |(data_ptr[21] << 8) | data_ptr[22];			
	sample.is_led_cur_adj = data_ptr[23];
	sample.adj_led_cur = (data_ptr[24] << 8) | data_ptr[25];
	sample.is_t_int_cur_adj = data_ptr[26];
	sample.adj_t_int_cur = (data_ptr[27] << 8) | data_ptr[28];
	sample.is_f_smp_adj = data_ptr[29];
	sample.adj_f_smp = data_ptr[30];		
	sample.smp_ave = data_ptr[31];
	sample.state = data_ptr[32];
	sample.is_high_motion = data_ptr[33];
	sample.status = data_ptr[34];

	//m_USB->printf("\r\nMd2- hr:%d  conf:%d  rr:%d  act:%d\r\n", sample.hr,sample.hr_conf,sample.rr,sample.activity_class );
	enqueue(&whrm_queue, &sample);
	pr_info("hr=%.1f conf=%d rr=%.1f rr_conf=%d status=%d\r\n", (float)sample.hr / 10.0, sample.hr_conf, (float)sample.rr/10, sample.rr_conf,sample.activity_class);	
}


void SSMAX8614XComm::accel_data_rx(uint8_t* data_ptr)
{
	//See API doc for data format
	accel_mode1_data sample;
	sample.x = (data_ptr[0] << 8) | data_ptr[1];
	sample.y = (data_ptr[2] << 8) | data_ptr[3];
	sample.z = (data_ptr[4] << 8) | data_ptr[5];

	enqueue(&accel_input_queue, &sample);
}

void SSMAX8614XComm::agc_data_rx(uint8_t* data_ptr)
{
	//NOP: AGC does not collect data
}

int SSMAX8614XComm::data_report_execute(char* buf, int size)
{
	uint8_t tmp_report_mode;
	max8614x_mode1_data max8614x_sample = { 0 };
	whrm_modeX_data whrm_sample_modeX = { 0 };
	//wspo2_modeX_data wspo2_sample_modeX = { 0 };
	accel_mode1_data accel_sample = { 0 };


	accel_data_t accel_data = {0};

	int16_t data_len = 0;

	static uint8_t isReportEnabled = 0;

	if (size <= 0)
	{
		m_USB->printf("\r\nError: RepSize = %d\r\n", size);
		return 0;
	}

	if (isReportEnabled != is_enabled())
	{
		isReportEnabled = is_enabled();
		m_USB->printf("\r\nRep Enable = %d \r\n",isReportEnabled);
	}

	if (!is_enabled())
	{
		return 0;
	}

	ss_int->ss_execute_once();

	comm_mutex.lock();
    tmp_report_mode = data_report_mode;
    comm_mutex.unlock();


	while (sensor_data_from_host) {
		accel_data_t accel_data = {0};

		int ret;
		ret = get_sensor_xyz(accel_data);
		if (ret < 0)
			break;

		accel_mode1_data acc_sample;
		acc_sample.x = (int16_t)(accel_data.x*1000);
		acc_sample.y = (int16_t)(accel_data.y*1000);
		acc_sample.z = (int16_t)(accel_data.z*1000);

		if (enqueue(&accel_output_queue, &acc_sample) < 0)
			pr_err("Thrown an accel sample\n");

		if (queue_len(&accel_output_queue) < MAX_NUM_WR_ACC_SAMPLES)
			break;

		int num_bytes = 0;
		SS_STATUS status = ss_int->get_num_bytes_in_input_fifo(&num_bytes);
		if (status != SS_SUCCESS) {
			pr_err("Unable to read num bytes in input fifo\r\n");
			break;
		}

		int num_tx = input_fifo_size - num_bytes;
		mxm_assert_msg((num_tx >= 0), "num_tx can't be negative");
		if (num_tx <= 0)
			break;

		int num_samples = num_tx / sizeof(accel_mode1_data);
		num_samples = min(num_samples, MAX_NUM_WR_ACC_SAMPLES);
		num_tx = num_samples * sizeof(accel_mode1_data);

		if (num_samples == 0) {
			pr_err("Input FIFO is Full\n");
			break;
		}

		accel_mode1_data peek_buf[num_samples];
		ret = queue_front_n(&accel_output_queue, peek_buf, num_samples, num_samples * sizeof(accel_mode1_data));
		if (ret < 0)
			mxm_assert_msg((num_tx >= 0), "Unable to peek samples from accel queue. Something is wrong.");

		uint8_t tx_buf[2 + num_tx]; /* 2 bytes were allocated for commands */
		for (int i = 2, j = 0; j < num_samples; i+= sizeof(accel_mode1_data), j++) {
			acc_sample = peek_buf[j];

			tx_buf[i] = acc_sample.x;
			tx_buf[i + 1] = acc_sample.x >> 8;

			tx_buf[i + 2] = acc_sample.y;
			tx_buf[i + 3] = acc_sample.y >> 8;

			tx_buf[i + 4] = acc_sample.z;
			tx_buf[i + 5] = acc_sample.z >> 8;

		}

    	int num_wr_bytes = 0;
		int nb_expected = num_tx;
		status = ss_int->feed_to_input_fifo(tx_buf, num_tx + 2, &num_wr_bytes);
		if (status != SS_SUCCESS) {
			pr_err("Unable to write accel data. num_tx: %d. status: %d\r\n", num_tx, status);
			break;
		}

		int num_written_samples = num_wr_bytes / sizeof(accel_mode1_data);
		if (num_written_samples == 0)
			break;

		ret = queue_pop_n(&accel_output_queue, num_written_samples);
		if (ret < 0)
			mxm_assert_msg((num_tx >= 0), "Unable to popped samples out from accel queue. Something is wrong.");

		break;
	}


	switch (tmp_report_mode) {
		case read_ppg_0:
		{
			if (1


				&& (queue_len(&max8614x_queue) > 0 && sensor_algo_en_dis_.max8614x_enabled)



				&& (queue_len(&accel_input_queue) > 0 && sensor_algo_en_dis_.accel_enabled)



				&& (queue_len(&whrm_queue) > 0 && sensor_algo_en_dis_.whrm_enabled)

				)
			{

				if(sensor_algo_en_dis_.max8614x_enabled)
					dequeue(&max8614x_queue, &max8614x_sample);



				if(sensor_algo_en_dis_.accel_enabled)
					dequeue(&accel_input_queue, &accel_sample);



				if(sensor_algo_en_dis_.whrm_enabled){
					dequeue(&whrm_queue, &whrm_sample_modeX);
					instant_hr = whrm_sample_modeX.hr * 0.1;
					instant_hr_conf = whrm_sample_modeX.hr_conf;
					instant_hr_activityClass = whrm_sample_modeX.activity_class;
				}


				if (AsciiEn) {
					// mode 2 only for ASCII
					if (ds_int->algo_report_mode == 2){
						data_len = snprintf(buf, size - 1, 
						"%u,%lu,%lu,%lu,%lu,%lu,%lu,%.3f,%.3f,%.3f,%.1f,%d,%.1f,%d,%d,%lu,%lu,%lu,%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
							sample_count++,
							max8614x_sample.led1,max8614x_sample.led2,max8614x_sample.led3,max8614x_sample.led4,max8614x_sample.led5,max8614x_sample.led6,
							accel_sample.x * 0.001,accel_sample.y * 0.001,accel_sample.z * 0.001,
							whrm_sample_modeX.hr * 0.1, whrm_sample_modeX.hr_conf, whrm_sample_modeX.rr * 0.1,whrm_sample_modeX.rr_conf,whrm_sample_modeX.activity_class,
							whrm_sample_modeX.walk_steps, whrm_sample_modeX.run_steps, whrm_sample_modeX.kcal, whrm_sample_modeX.cadence,
							whrm_sample_modeX.is_led_cur_adj, whrm_sample_modeX.adj_led_cur, whrm_sample_modeX.is_t_int_cur_adj, whrm_sample_modeX.adj_t_int_cur,
							whrm_sample_modeX.is_f_smp_adj, whrm_sample_modeX.adj_f_smp,
							whrm_sample_modeX.smp_ave, whrm_sample_modeX.state, whrm_sample_modeX.is_high_motion, whrm_sample_modeX.status);						
					}
					else {
						data_len = snprintf(buf, size - 1, "%u,%lu,%lu,%lu,%lu,%lu,%lu,%.3f,%.3f,%.3f,%.1f,%d,%.1f,%d\r\n",
							sample_count++,
							max8614x_sample.led1,
							max8614x_sample.led2,
							max8614x_sample.led3,
							max8614x_sample.led4,
							max8614x_sample.led5,
							max8614x_sample.led6,
							accel_sample.x * 0.001,
							accel_sample.y * 0.001,
							accel_sample.z * 0.001,
							whrm_sample_modeX.hr * 0.1,
							whrm_sample_modeX.hr_conf,
							whrm_sample_modeX.rr * 0.1,
							whrm_sample_modeX.activity_class);
					}
				}
				else { // bin report


					mxm_assert_msg(((uint32_t)size > sizeof(ds_pkt_data_mode2_compact)), "data_report_execute buffer too small");
					ds_pkt_data_mode2_compact* data_packet = (ds_pkt_data_mode2_compact*)buf;
					data_packet->start_byte = DS_BINARY_PACKET_START_BYTE;
					data_packet->sample_cnt = sample_count++;
					data_packet->grnCnt = max8614x_sample.led1;
					data_packet->grn2Cnt = max8614x_sample.led4;
					data_packet->hr = (whrm_sample_modeX.hr * 0.1);
					data_packet->x = (int16_t)(accel_sample.x);
					data_packet->y = (int16_t)(accel_sample.y);
					data_packet->z = (int16_t)(accel_sample.z);
					data_packet->hr_confidence = whrm_sample_modeX.hr_conf;
					data_packet->rr = whrm_sample_modeX.rr;
					data_packet->status = whrm_sample_modeX.activity_class;
					data_packet->crc8 = crc8((uint8_t*)data_packet, sizeof(*data_packet) - sizeof(uint8_t));
					data_len = sizeof(*data_packet);
				}
			}
			else
			{
				//m_USB->printf("\r\nError Printing: max8614x_q:%d, max8614x_enabled:%d,  \r\n",queue_len(&max8614x_queue), sensor_algo_en_dis_.max8614x_enabled );
				//m_USB->printf("\r\n                accel_q:%d,    accel_enabled:%d,  \r\n",queue_len(&accel_input_queue), sensor_algo_en_dis_.accel_enabled);
				//m_USB->printf("\r\n                whrm_queue:%d,whrm_enabled:%d,  \r\n",queue_len(&whrm_queue) , sensor_algo_en_dis_.whrm_enabled);

			}
		} break;

		default:
		{
			m_USB->printf("\r\nError tmp_report_mode=%d\r\n", tmp_report_mode);
			return 0;
		}
	}

    if (data_len < 0) {
		pr_err("snprintf console_tx_buf failed");
	} else if (data_len > size) {
		pr_err("buffer is insufficient to hold data");
	}

	return data_len;
}

// TODO: convert this to PPG sensor test
int SSMAX8614XComm::selftest_max8614x(){
	int ret;
	uint8_t test_result;
	bool test_failed = false;
	m_USB->printf("starting selftest_max8614x\r\n");
	// configure mfio pin for self test
	ss_int->mfio_selftest();
	ret = ss_int->self_test(SS_SENSORIDX_MAX86140, &test_result, 1000);
	if(ret != SS_SUCCESS){
		m_USB->printf("ss_int->self_test(SS_SENSORIDX_MAX86140, &test_result) has failed err<-1>\r\n");
		test_failed = true;
	}
	// reset mfio pin to old state
	if(!ss_int->reset_mfio_irq()){
		m_USB->printf("smart sensor reset_mfio_irq has failed err<-1>\r\n");
		test_failed = true;
	}
	// reset the sensor to turn off the LED
	ret = ss_int->reset();
	if(test_failed | !self_test_result_evaluate("selftest_max8614x", test_result)){
		return -1;
	}else{
		return SS_SUCCESS;
	}
}

int SSMAX8614XComm::selftest_accelerometer(){
	int ret;
	uint8_t test_result;
	bool test_failed = false;
	m_USB->printf("starting selftest_accelerometer\r\n");
	ret = ss_int->self_test(SS_SENSORIDX_ACCEL, &test_result, 1000);
	if(ret != SS_SUCCESS){
		m_USB->printf("ss_int->self_test(SS_SENSORIDX_ACCEL, &test_result) has failed err<-1>\r\n");
		test_failed = true;
	}
	// reset the sensor to turn off the LED
	ret = ss_int->reset();
	if(ret != SS_SUCCESS){
		m_USB->printf("smart sensor reset has failed err<-1>\r\n");
		test_failed = true;
	}
	if(test_failed | !self_test_result_evaluate("selftest_accelerometer", test_result)){
		return -1;
	}else{
		return SS_SUCCESS;
	}
}

bool SSMAX8614XComm::self_test_result_evaluate(const char *message, uint8_t result){
	// check i2c response status
	if(result != 0x00){
		m_USB->printf("%s has failed % 02X err<-1>\r\n", message, result);
		if((result & FAILURE_COMM))
			m_USB->printf("%s communication has failed err<-1>\r\n", message);
		if(result & FAILURE_INTERRUPT)
			m_USB->printf("%s interrupt pin check has failed err<-1>\r\n", message);
		return false;
	}
	return true;
}

unsigned char SSMAX8614XComm::get_sensor_id() {
	return SENSOR_ID_SSMAX8614X; //TODO: assign the correct number
}
