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
 
 
#include "MAX30205.h"


//******************************************************************************
MAX30205::MAX30205(I2C *i2c, uint8_t slaveAddress):
m_i2c(i2c), m_writeAddress(slaveAddress << 1), 
m_readAddress((slaveAddress << 1) | 1)
{

}


//******************************************************************************
MAX30205::~MAX30205(void) 
{
  //empty block
}




//******************************************************************************
int32_t MAX30205::readTemperature(uint16_t &value) 
{
  return readRegister(MAX30205::Temperature, value);
}


//******************************************************************************
int32_t MAX30205::readConfiguration(Configuration_u &config)
{
    uint16_t data;
    
    int32_t result = readRegister(MAX30205::Configuration, data);
    if(result == 0)
    {
        config.all = (0x00FF & data);
    }
    
    return result;
    
}


//******************************************************************************    
int32_t MAX30205::writeConfiguration(const Configuration_u config)
{
    uint16_t local_config = (0x00FF & config.all);
    
    return writeRegister(MAX30205::Configuration, local_config);
}


//******************************************************************************
int32_t MAX30205::readTHYST(uint16_t &value) 
{
  return readRegister(MAX30205::THYST, value);
}


//******************************************************************************
int32_t MAX30205::writeTHYST(uint16_t value) 
{
  return writeRegister(MAX30205::THYST, value);
}


//******************************************************************************
int32_t MAX30205::readTOS(uint16_t &value)
{
    return readRegister(MAX30205::TOS, value);
}


//******************************************************************************
int32_t MAX30205::writeTOS(const uint16_t value)
{
    return writeRegister(MAX30205::TOS, value);
}


//******************************************************************************
float MAX30205::toCelsius(uint32_t rawTemp) 
{
  uint8_t val1, val2;
  float result;
  
  val1 = (rawTemp >> 8);
  val2 = (rawTemp & 0xFF);
  
  result = static_cast<float>(val1 + (val2/ 256.0F));
  
  return result;
}


//******************************************************************************
float MAX30205::toFahrenheit(float temperatureC) 
{
  return((temperatureC * 1.8F) + 32.0f);
}


//******************************************************************************
int32_t MAX30205::writeRegister(Registers_e reg, uint16_t value) 
{
  int32_t result;
  
  uint8_t hi = ((value >> 8) & 0xFF);
  uint8_t lo = (value & 0xFF);
  char cmdData[3] = {reg, hi, lo};
  
  result = m_i2c->write(m_writeAddress, cmdData, 3);
  
  return result;
}


//******************************************************************************
int32_t MAX30205::readRegister(Registers_e reg, uint16_t &value) 
{
  int32_t result;
  
  char data[2];
  char cmdData[1] = {reg};
  
  result = m_i2c->write(m_writeAddress, cmdData, 1);
  if(result == 0)
  {
      result = m_i2c->read(m_readAddress, data, 2);
      if (result == 0)
      {
          value = (data[0] << 8) + data[1];
      }
  }
  
  return result;
}

int MAX30205::dump_registers(addr_val_pair *reg_vals){
    int i;
    int ret = 0;
	uint16_t val;
    for (i = 0x00; i <= 0x03; i++) {
        reg_vals[i].addr = i;
        ret |= readRegister(static_cast<Registers_e>(i), val);
		reg_vals[i].val = val;
    }

	return ret;
}

const char *MAX30205::get_sensor_part_name()
{
	return "max30205";
}

const char *MAX30205::get_sensor_algo_ver()
{
	return "dummy_algo_ver";
}


int MAX30205::sensor_enable(int enable){
	int32_t ret;
	Configuration_u config;


	ret = readConfiguration(config);
	if(ret != 0)
		return ret;

	if(enable){
		config.bits.shutdown = 0;
	}else{
		config.bits.shutdown = 1;
	}

	return writeConfiguration(config);
}

/**
* @brief	Get sensor ID.
*
* @returns	Sensor ID number.
*/
unsigned char MAX30205::get_sensor_id() {

	return( SENSOR_ID_MAX30205 );

}
