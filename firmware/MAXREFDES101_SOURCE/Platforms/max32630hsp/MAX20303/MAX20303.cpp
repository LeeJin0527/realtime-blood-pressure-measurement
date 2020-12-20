/*******************************************************************************
 * Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
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


#include "MAX20303.h"



//******************************************************************************
MAX20303::MAX20303(I2C *i2c):
	m_i2c(i2c), m_writeAddress(MAX20303_SLAVE_WR_ADDR),
	m_readAddress(MAX20303_SLAVE_RD_ADDR)
{
}


//******************************************************************************
MAX20303::~MAX20303(void)
{
  //empty block
}


//******************************************************************************
int MAX20303::LDO1Config()
{
	int32_t ret = 0;
//	uint8_t val;
//	ret |= writeReg(MAX20303::REG_AP_CMDOUT, 0x40);
//	ret |= writeReg(MAX20303::REG_AP_DATOUT0, 0x05);
//	ret |= writeReg(MAX20303::REG_AP_DATOUT1, 0x34);
//
//	readReg(MAX20303::REG_AP_CMDOUT, val);
//	readReg(MAX20303::REG_AP_DATOUT0, val);
//	readReg(MAX20303::REG_AP_DATOUT1, val);
	appcmdoutvalue_ = 0x40;
	appdatainoutbuffer_[0] = 0x05;
	appdatainoutbuffer_[1] = 0x34;
	AppWrite(2);

	return ret;
}

//******************************************************************************
int MAX20303::LDO2Config()
{
	int32_t ret = 0;
//	uint8_t val;
	appcmdoutvalue_ = 0x42;
	appdatainoutbuffer_[0] = 0x01;
	appdatainoutbuffer_[1] = 0x15;     // 0.9V + (0.1V * number)   =  3V
	AppWrite(2);

	return ret;
}


//******************************************************************************
int MAX20303::writeReg(registers_t reg, uint8_t value)
{
	int ret;

	char cmdData[2] = {reg, value};

	ret = m_i2c->write(m_writeAddress, cmdData, sizeof(cmdData));
	//printf("MAX20303 write reg[0x%X]=0x%X, ret=%d\r\n", (uint32_t)reg, value, ret)

	if (ret != 0)
		return MAX20303_ERROR;

	return MAX20303_NO_ERROR;
}


//******************************************************************************
int MAX20303::readReg(registers_t reg, uint8_t &value)
{
	int ret;

	char data = reg;

	ret = m_i2c->write(m_writeAddress, &data, sizeof(data));
	if (ret != 0) {
		printf("%s - failed - ret: %d\n", __func__, ret);
		return MAX20303_ERROR;
	}

	ret = m_i2c->read(m_readAddress, &data, sizeof(data));
	if (ret != 0) {
		printf("%s - failed - ret: %d\n", __func__, ret);
		return MAX20303_ERROR;
	}

	value = data;
	printf("MAX20303 read reg[0x%X]=0x%X, ret=%d\r\n", (unsigned int)reg,  (unsigned int)value, ret);
	return MAX20303_NO_ERROR;
}

//******************************************************************************
int MAX20303::readRegMulti(registers_t reg, uint8_t *value, uint8_t len){
	int ret;
	char data = reg;

	ret = m_i2c->write(m_writeAddress, &data, sizeof(data));
	if (ret != 0) {
		printf("%s - failed - ret: %d\n", __func__, ret);
		return MAX20303_ERROR;
	}

	ret = m_i2c->read(m_readAddress, (char *)value, len);
	if (ret != 0) {
		printf("%s - failed - ret: %d\n", __func__, ret);
		return MAX20303_ERROR;
	}

	printf("MAX20303 read reg[0x%X]=0x%X, ret=%d\r\n", (unsigned int)reg, (unsigned int)value, ret);
	return MAX20303_NO_ERROR;
}

//******************************************************************************
int MAX20303::writeRegMulti(registers_t reg, uint8_t *value, uint8_t len){
	int32_t ret;
	i2cbuffer_[0] = reg;
	memcpy(&i2cbuffer_[1], value, len);

	ret = m_i2c->write(m_writeAddress, (char *)i2cbuffer_, (len+1));
	//printf("MAX20303 write reg[0x%X]=0x%X, ret=%d\r\n", (uint32_t)reg, value, ret)

	if (ret != 0)
		return MAX20303_ERROR;

	return MAX20303_NO_ERROR;
}
//******************************************************************************
int MAX20303::mv2bits(int mV)
{
    int regBits;

    if (( MAX20303_LDO_MIN_MV <= mV) && (mV <= MAX20303_LDO_MAX_MV)) {
        regBits = (mV -  MAX20303_LDO_MIN_MV) /  MAX20303_LDO_STEP_MV;
    } else {
        return -1;
    }

    return regBits;
}
//******************************************************************************
int MAX20303::PowerOffthePMIC(){
	int ret;
	appdatainoutbuffer_[0] = 0xB2;
	appcmdoutvalue_ = 0x80;
	ret = AppWrite(1);

	if(appcmdoutvalue_ != 0x80){
		ret |= MAX20303_ERROR;
	}

	return ret;
}
//******************************************************************************
int MAX20303::PowerOffDelaythePMIC(){
	int ret;
	appdatainoutbuffer_[0] = 0xB2;
	appcmdoutvalue_ = 0x84;
	ret = AppWrite(1);

	if(appcmdoutvalue_ != 0x80){
		ret |= MAX20303_ERROR;
	}

	return ret;
}

//******************************************************************************
int MAX20303::SoftResetthePMIC(){
	int ret;
	appdatainoutbuffer_[0] = 0xB3;
	appcmdoutvalue_ = 0x81;
	ret = AppWrite(1);

	if(appcmdoutvalue_ != 0x81){
		ret |= MAX20303_ERROR;
	}

	return ret;
}
//******************************************************************************
int MAX20303::HardResetthePMIC(){
	int ret;
	appdatainoutbuffer_[0] = 0xB4;
	appcmdoutvalue_ = 0x82;
	ret = AppWrite(1);

	if(appcmdoutvalue_ != 0x82){
		ret |= MAX20303_ERROR;
	}

	return ret;
}

//******************************************************************************
int MAX20303::AppWrite(uint8_t dataoutlen){
	int ret;

	ret = writeRegMulti(MAX20303::REG_AP_DATOUT0, appdatainoutbuffer_, dataoutlen);
	ret |= writeReg(MAX20303::REG_AP_CMDOUT, appcmdoutvalue_);
	wait_ms(10);
	ret |= readReg(MAX20303::REG_AP_RESPONSE, appcmdoutvalue_);

	if(ret != 0)
		return MAX20303_ERROR;

	return MAX20303_NO_ERROR;
}


//******************************************************************************
int MAX20303::AppRead(uint8_t datainlen){
	int ret;

	ret = writeReg(MAX20303::REG_AP_CMDOUT, appcmdoutvalue_);
	wait_ms(10);
	ret |= readRegMulti(MAX20303::REG_AP_RESPONSE, i2cbuffer_, datainlen);
	if(ret != 0)
		return MAX20303_ERROR;

	return MAX20303_NO_ERROR;
}

//******************************************************************************
char MAX20303::CheckPMICHWID(){
	int ret;
	uint8_t value = 0x00;

	ret = readReg(MAX20303::REG_HARDWARE_ID, value);
	if(ret != MAX20303_NO_ERROR)
		return false;

	if(value == 0x02)
		return true;
	else
		return false;
}

//******************************************************************************
int MAX20303::CheckPMICStatusRegisters(unsigned char buf_results[5]){
	int ret;
	ret  = readReg(MAX20303::REG_STATUS0, buf_results[0]);
	ret |= readReg(MAX20303::REG_STATUS1, buf_results[1]);
	ret |= readReg(MAX20303::REG_STATUS2, buf_results[2]);
	ret |= readReg(MAX20303::REG_STATUS3, buf_results[3]);
	ret |= readReg(MAX20303::REG_SYSTEM_ERROR, buf_results[4]);
	return ret;
}

//******************************************************************************
int MAX20303::Max20303_BatteryGauge(unsigned char *batterylevel){
	int ret;
	char data[2];
	//uint8_t value;
	//printf("m_battery_is_connected is:%d\r\n", m_battery_is_connected);
	if(!m_battery_is_connected) {
		*batterylevel = 0;
		return 0;
	}

	data[0] = 0x04;
	ret = m_i2c->write(MAX20303_I2C_ADDR_FUEL_GAUGE, data, 1);
	if(ret != 0){
		printf("Max20303_FuelGauge has failed\r\n");
	}

	ret = m_i2c->read(MAX20303_I2C_ADDR_FUEL_GAUGE | 1, data, 2);
	if(ret != 0){
		printf("Max20303_FuelGauge has failed\r\n");
	}
	//printf("battery level is:%d\r\n", data[0]);
	*batterylevel = data[0];

	return 0;
}

//******************************************************************************
char MAX20303::Max20303_IsBattery_Connected(){
	AnalogIn   ain(AIN_5);
	float adc_value;
	int ret;
	char result;
	// config the mux for the monitor pin
	appcmdoutvalue_ = 0x50;
	appdatainoutbuffer_[0] = 0x80;
	AppWrite(1);
	// disable the charger
	appcmdoutvalue_ = 0x1A;
	appdatainoutbuffer_[0] = 0x02;
	AppWrite(1);
	wait_ms(250);
	// adc measure launch
	appcmdoutvalue_ = 0x53;
	appdatainoutbuffer_[0] = 0x09;
	AppWrite(1);
	wait_ms(10);
	ret = readRegMulti(MAX20303::REG_AP_RESPONSE, i2cbuffer_, 5);
	adc_value = ain.read();
	if(ret != 0){
		result = false;
	} else {
		if((i2cbuffer_[4] < 0x40) | (adc_value < 0.3)) {
			result = false;
		} else {
			result = true;
		}
	}
	// enable the charger
	appcmdoutvalue_ = 0x1A;
	appdatainoutbuffer_[0] = 0x03;
	AppWrite(1);
//	for(int i = 1; i < 5; ++i){
//		printf("reg value at:%d is:%02X\r\n", i, i2cbuffer_[i]);
//	}
	m_battery_is_connected = result;
	return result;
}


//******************************************************************************
int MAX20303::led0on(char enable) {

	if(enable)
		return writeReg(REG_LED0_DIRECT, 0x21);
	else
		return writeReg(REG_LED0_DIRECT, 0x01);
}

//******************************************************************************
int MAX20303::led1on(char enable) {
	if(enable)
		return writeReg(REG_LED1_DIRECT, 0x21);
	else
		return writeReg(REG_LED1_DIRECT, 0x01);
}

//******************************************************************************
int MAX20303::led2on(char enable) {
	if(enable)
		return writeReg(REG_LED2_DIRECT, 0x21);
	else
		return writeReg(REG_LED2_DIRECT, 0x01);
}


//******************************************************************************
int MAX20303::BoostEnable(void) {
	writeReg(REG_AP_DATOUT3, 0x00);	// 00 : 5V
	writeReg(REG_AP_DATOUT0, 0x01);	// Boost Enabled
	writeReg(REG_AP_CMDOUT, 0x30);
	return MAX20303_NO_ERROR;
}

//******************************************************************************
int MAX20303::BuckBoostEnable(void)
{
	int ret = 0;

	ret |= writeReg( REG_AP_DATOUT0,  0x00);    // Reserved = 0x00
	ret |= writeReg( REG_AP_DATOUT1,  0x04);    // BBstlSet = 0b'100   Buck Boost Peak current Limit = 200mA
	ret |= writeReg( REG_AP_DATOUT2,  0x19);    // BBstVSet = 0b'11001  Buck Boost Output Voltage = 5V
	ret |= writeReg( REG_AP_DATOUT3,  0x01);    // BBstRipRed = 1 Ripple Reduction
	// BBstAct    = 1 Actively discharged in Hard-Reset or Enable Low
	// BBstPas    = 1 Passively discharged in Hard-Reset or Enable Low
	// BBstMd     = 1 Damping Enabled
	// BBstInd    = 0  Inductance is 4.7uH
	// BBstEn     = 0b'01 Enabled
	ret |= writeReg( REG_AP_CMDOUT, 0x70);
	if (ret != 0)
		return MAX20303_ERROR;

	return MAX20303_NO_ERROR;
}
