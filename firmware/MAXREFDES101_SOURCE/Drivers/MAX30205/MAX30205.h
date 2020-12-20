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
#ifndef __MAX30205_H_
#define __MAX30205_H_

#include "mbed.h"
#include "MaximSensor.h"

#define	SENSOR_ID_MAX30205	0x03

/**
 * @brief Library for the MAX30205\n
 * The MAX30205 temperature sensor accurately measures temperature and provide 
 * an overtemperature alarm/interrupt/shutdown output. This device converts the 
 * temperature measurements to digital form using a high-resolution, 
 * sigma-delta, analog-to-digital converter (ADC). Accuracy meets clinical 
 * thermometry specification of the ASTM E1112 when soldered on the final PCB. 
 * Communication is through an I2C-compatible 2-wire serial interface.
 *
 * @code
 * #include "mbed.h"
 * #include "max32630fthr.h"
 * #include "MAX30205.h"
 * 
 * MAX32630FTHR pegasus(MAX32630FTHR::VIO_3V3);
 *
 * //Get I2C instance
 * I2C i2cBus(I2C1_SDA, I2C1_SCL);
 *
 * //Get temp sensor instance
 * MAX30205 bodyTempSensor(i2cBus, 0x4D); //Constructor takes 7-bit slave adrs
 *
 * int main(void) 
 * {
 *	 //use sensor
 * }
 * @endcode
 */

class MAX30205: public MaximSensor
{
	
public:
	/// MAX30205 Register Addresses
	enum Registers_e 
	{
		Temperature   = 0x00,
		Configuration = 0x01,
		THYST		 = 0x02,
		TOS		   = 0x03
	};
	
	///MAX30205 Configuration register bitfields
	union Configuration_u
	{
		uint8_t all;
		struct BitField_s
		{
			uint8_t shutdown	: 1;
			uint8_t comp_int	: 1;
			uint8_t os_polarity : 1;
			uint8_t fault_queue : 2;
			uint8_t data_format : 1;
			uint8_t timeout	 : 1;
			uint8_t one_shot	: 1;
		}bits;
	};
	
	/**
	* @brief  Constructor using reference to I2C object
	* @param i2c - Reference to I2C object
	* @param slaveAddress - 7-bit I2C address
	*/
	MAX30205(I2C *i2c, uint8_t slaveAddress);

	/** @brief Destructor */
	~MAX30205(void);

	/**
	* @brief Read the temperature from the device into a 16 bit value
	* @param[out] value - Raw temperature data on success
	* @return 0 on success, non-zero on failure
	*/
	int32_t readTemperature(uint16_t &value);
	
	/**
	* @brief Read the configuration register
	* @param config - Reference to Configuration type
	* @return 0 on success, non-zero on failure
	*/
	int32_t readConfiguration(Configuration_u &config);
	
	/**
	* @brief Write the configuration register with given configuration
	* @param config - Configuration to write
	* @return 0 on success, non-zero on failure
	*/
	int32_t writeConfiguration(const Configuration_u config);

	/**
	* @brief Read the THYST value from a specified device instance
	* @param[out] value - THYST register value on success
	* @return 0 on success, non-zero on failure
	*/
	int32_t readTHYST(uint16_t &value);

	/**
	* @brief Write the THYST to a device instance
	* @param value - 16-bit value to write
	* @return 0 on success, non-zero on failure
	*/
	int32_t writeTHYST(const uint16_t value);
	
	/**
	* @brief Read the TOS value from device
	* @param[out] value - TOS register value on success
	* @return 0 on success, non-zero on failure
	*/
	int32_t readTOS(uint16_t &value);

	/**
	* @brief Write the TOS register
	* @param value - 16-bit value to write
	* @return 0 on success, non-zero on failure
	*/
	int32_t writeTOS(const uint16_t value);

	/**
	* @brief Convert a raw temperature value into a float
	* @param rawTemp - raw temperature value to convert
	* @return the convereted value in degrees C
	*/
	float toCelsius(uint32_t rawTemp);

	/**
	* @brief Convert the passed in temperature in C to Fahrenheit
	* @param temperatureC Temperature in C to convert
	* @returns Returns the converted Fahrenheit value
	*/
	float toFahrenheit(float temperatureC);
	
//protected:

	/** 
	* @brief Write register of device at slave address
	* @param reg - Register address
	* @param value - Value to write
	* @return 0 on success, non-zero on failure
	*/
	int32_t writeRegister(Registers_e reg, uint16_t value);

	/**
	* @brief  Read register of device at slave address
	* @param reg - Register address
	* @param[out] value - Read data on success
	* @return 0 on success, non-zero on failure
	*/
	int32_t readRegister(Registers_e reg, uint16_t &value);

	/**
	* @brief	Dump Maxim Sensor registers.
	* @details  Fill out register values into list
	*
	* @returns  0 on success, negative error code on failure.
	*/
	virtual int dump_registers(addr_val_pair *reg_values);


	/**
	* @brief	Get sensor ID.
	*
	* @returns	Sensor ID number.
	*/
	unsigned char get_sensor_id();

	const char *get_sensor_part_name();
	const char *get_sensor_algo_ver();

									// sc... added functions
	// *************************************************************************** //
	/* @brief	Enables Maxim Sensor.
	* @details	Enable IRQ, enable LEDs, enable AGC
	*
	* @param[in]	enable Any value to enable, 0 to disable.
	*
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int sensor_enable(int enable);


	// *************************************************************************** //

private:
	/// I2C object
	I2C  *m_i2c;
	/// Device slave addresses
	uint8_t m_writeAddress, m_readAddress;
};

#endif /* __MAX30205_H_ */
