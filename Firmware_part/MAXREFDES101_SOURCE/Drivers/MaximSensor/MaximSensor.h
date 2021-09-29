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

#ifndef _MAXIMSENSOR_H_
#define _MAXIMSENSOR_H_
#include "mbed.h"
#include <list>


typedef struct {
    uint8_t addr;
    uint32_t val;
} addr_val_pair;


/**
 * @brief	MaximSensor is Maxim Sensor base class.
 * @details	MaximSensor includes base functions for to create new
 *	sensor classes. All sensor classes should implement this class.
 */
class MaximSensor
{
public:
	/* PUBLIC FUNCTION DECLARATIONS */
	/**
	* @brief	Reads from register.
	* @details	Reads specific Maxim Sensor register via SPI bus.
	*
	* @param[in]	reg Beginning address of a register to be read.
	* @param[out]	data Buffer space to save result value.
	* @param[in]	len Number of consecutive bytes to be read.
	*
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int readRegister(uint8_t reg, uint8_t *data, int len);

	/**
	* @brief	Writes data to Maxim Sensor register.
	* @details	Writes data to specific Maxim Sensor register via SPI bus.
	*
	* @param[in]	reg Address of a register to be wrote.
	* @param[in]	data Data to write on register.
	*
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int writeRegister(uint8_t reg,	const uint8_t data);

	/**
	* @brief	Get Maxim Sensor part and revision info.
	* @details	Reads Maxim Sensor part and revision info from device.
	*
	* @param[in]	reg Beginning address of a register to be read.
	* @param[out]	data Buffer space to save result value.
	* @param[in]	len Number of consecutive bytes to be read.
	*
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int get_part_info(uint8_t *part_id,	uint8_t *rev_id);

	/**
	* @brief	Enables Maxim Sensor.
	* @details	Enable IRQ, enable LEDs, enable AGC
	*
	* @param[in]	enable Any value to enable, 0 to disable.
	*
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int sensor_enable(int enable);

	/**
	* @brief	Enables AGC.
	* @details	Enable Maxim Sensor automatic gain controller.
	*	AGC automatically adjusts sampling rates and LED currents to save energy.
	*
	* @param[in]	agc_enable Any value to enable, 0 to disable.
	*
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int agc_enable(int agc_enable);

	/**
	* @brief	Get sensor part name.
	*
	* @returns	Sensor part name string.
	*/
	virtual const char *get_sensor_part_name();

	/**
	* @brief	Get sensor algorithm version.
	*
	* @returns	Sensor algorithm version string.
	*/
	virtual const char *get_sensor_algo_ver();

	/**
	* @brief	Get sensor name.
	*
	* @returns	Sensor name string.
	*/
	virtual const char *get_sensor_name();

	/**
	* @brief	Get sensor ID.
	*
	* @returns	Sensor ID number.
	*/
	virtual unsigned char get_sensor_id();

	/**
	* @brief	Dump Maxim Sensor registers.
	* @details	Print all Maxim Sensor register addresses and containing values.
	*
	* @param[in]    reg_values Pointer to array of 256 addr_val_pairs
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int dump_registers(addr_val_pair *reg_values)=0;

	// *********************** Maxim Sensor ECG Max30001 related functions ***********************
	virtual int MS_Max30001_ECG_InitStart(uint8_t En_ecg, uint8_t Openp, uint8_t Openn,
						 uint8_t Pol, uint8_t Calp_sel, uint8_t Caln_sel,
										 uint8_t E_fit, uint8_t Rate, uint8_t Gain,
										 uint8_t Dhpf, uint8_t Dlpf);

	virtual int MS_Max30001_ECG_Stop();

	// ECG Max30001 RtoR Initialization Function
	virtual  int MS_Max30001_RtoR_InitStart(uint8_t En_rtor, uint8_t Wndw, uint8_t Gain,
			uint8_t Pavg, uint8_t Ptsf, uint8_t Hoff,
			uint8_t Ravg, uint8_t Rhsf, uint8_t Clr_rrint);

	virtual int MS_Max30001_RtoR_Stop();

	// Max30001 Interrupt Assignment Function
	virtual int MS_max30001_INT_assignment(uint8_t en_enint_loc,     uint8_t en_eovf_loc,  uint8_t en_fstint_loc,
										   uint8_t en_dcloffint_loc, uint8_t en_bint_loc,  uint8_t en_bovf_loc,
										   uint8_t en_bover_loc,     uint8_t en_bundr_loc, uint8_t en_bcgmon_loc,
										   uint8_t en_pint_loc,      uint8_t en_povf_loc,  uint8_t en_pedge_loc,
										   uint8_t en_lonint_loc,    uint8_t en_rrint_loc, uint8_t en_samp_loc,
										   uint8_t intb_Type,        uint8_t int2b_Type);

	virtual int MS_max30001readRegister(uint8_t addr, uint32_t *return_data);

	virtual int MS_max30001writeRegister(uint8_t addr, uint32_t data);

	virtual int MS_max30001sync();

	// *********************** end of Maxim Sensor ECG Max30001 related functions ****************



	// *********************** Max30205 related functions ***********************


	// *********************** end of Max30205 related functions ****************


};

#endif /* _MAXIMSENSOR_H_ */
