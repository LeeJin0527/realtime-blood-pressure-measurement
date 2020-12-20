/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
* 
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
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

#include "MaximSensor.h"
#include "Peripherals.h"

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
int MaximSensor::readRegister(uint8_t reg, uint8_t *data, int len){
	pr_err("Empty function is called");
	return -1;
}

	/**
	* @brief	Writes data to Maxim Sensor register.
	* @details	Writes data to specific Maxim Sensor register via SPI bus.
	*
	* @param[in]	reg Address of a register to be wrote.
	* @param[in]	data Data to write on register.
	*
	* @returns	0 on success, negative error code on failure.
	*/
int MaximSensor::writeRegister(uint8_t reg,	const uint8_t data){
	pr_err("Empty function is called");
	return -1;
}

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
int MaximSensor::get_part_info(uint8_t *part_id,	uint8_t *rev_id){
	pr_err("Empty function is called");
	return -1;
}

	/**
	* @brief	Enables Maxim Sensor.
	* @details	Enable IRQ, enable LEDs, enable AGC
	*
	* @param[in]	enable Any value to enable, 0 to disable.
	*
	* @returns	0 on success, negative error code on failure.
	*/
int MaximSensor::sensor_enable(int enable){
	pr_err("Empty function is called");
	return -1;
}

	/**
	* @brief	Enables AGC.
	* @details	Enable Maxim Sensor automatic gain controller.
	*	AGC automatically adjusts sampling rates and LED currents to save energy.
	*
	* @param[in]	agc_enable Any value to enable, 0 to disable.
	*
	* @returns	0 on success, negative error code on failure.
	*/
int MaximSensor::agc_enable(int agc_enable){
	pr_err("Empty function is called");
	return -1;
}

	/**
	* @brief	Get sensor part name.
	*
	* @returns	Sensor part name string.
	*/
const char *MaximSensor::get_sensor_part_name(){
	pr_err("Empty function is called");
	return "";
}
#if 0
int dump_registers(addr_val_pair *reg_values) {
	pr_err("Empty function is called");
	return 0;
}
#endif
	/**
	* @brief	Get sensor name.
	*
	* @returns	Sensor name string.
	*/
const char *MaximSensor::get_sensor_name(){
	pr_err("Empty function is called");
	return "";
}


/**
* @brief	Get sensor ID.
*
* @returns	Sensor ID number.
*/
unsigned char MaximSensor::get_sensor_id() {

	// Base class returns 255
	return( 255 );

}


/**
* @brief	Get sensor name.
*
* @returns	Sensor name string.
*/
const char *MaximSensor::get_sensor_algo_ver(){
pr_err("Empty function is called");
return "";
}

int MaximSensor::MS_Max30001_ECG_InitStart(uint8_t En_ecg, uint8_t Openp, uint8_t Openn,
						 uint8_t Pol, uint8_t Calp_sel, uint8_t Caln_sel,
									 uint8_t E_fit, uint8_t Rate, uint8_t Gain,
									 uint8_t Dhpf, uint8_t Dlpf){
	pr_err("Empty function is called");
	return -1;
}

int MaximSensor::MS_Max30001_ECG_Stop(){
	pr_err("Empty function is called");
	return -1;
}

int MaximSensor::MS_Max30001_RtoR_InitStart(uint8_t En_rtor, uint8_t Wndw, uint8_t Gain,
										    uint8_t Pavg, uint8_t Ptsf, uint8_t Hoff,
											uint8_t Ravg, uint8_t Rhsf, uint8_t Clr_rrint){
	pr_err("Empty function is called");
	return -1;
}

int MaximSensor::MS_Max30001_RtoR_Stop(){
	pr_err("Empty function is called");
	return -1;
}

// Max30001 Interrupt Assignment Function
int MaximSensor::MS_max30001_INT_assignment(uint8_t en_enint_loc,     uint8_t en_eovf_loc,  uint8_t en_fstint_loc,
											uint8_t en_dcloffint_loc, uint8_t en_bint_loc,  uint8_t en_bovf_loc,
									        uint8_t en_bover_loc,     uint8_t en_bundr_loc, uint8_t en_bcgmon_loc,
									        uint8_t en_pint_loc,      uint8_t en_povf_loc,  uint8_t en_pedge_loc,
									        uint8_t en_lonint_loc,    uint8_t en_rrint_loc, uint8_t en_samp_loc,
									        uint8_t intb_Type,        uint8_t int2b_Type){
	pr_err("Empty function is called");
	return -1;
}

int  MaximSensor::MS_max30001readRegister(uint8_t addr, uint32_t *return_data){
	pr_err("Empty function is called");
	return -1;
}

int  MaximSensor::MS_max30001writeRegister(uint8_t addr, uint32_t data){
	pr_err("Empty function is called");
	return -1;
}

int MaximSensor::MS_max30001sync(){
	pr_err("Empty MS_max30001sync is called");
	return -1;
}
