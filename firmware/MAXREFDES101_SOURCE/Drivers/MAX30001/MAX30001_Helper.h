#ifndef _MAX30001_HELPER_H_
#define _MAX30001_HELPER_H_


#include "mbed.h"
#include "MaximSensor.h"
#include "MAX30001.h"
#include "queue.h"
#include "EcgComm.h"

#define	SENSOR_ID_MAX30001	0x02

class MAX30001_Helper: public MaximSensor
{
public:
	MAX30001_Helper(MAX30001 *m, InterruptIn *ir_B, InterruptIn *ir_2B);
	// Inherited function implementations from MaximSensor
	virtual int readRegister(uint8_t reg, uint8_t *data, int len);

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
	* @brief	Get sensor name.
	*
	* @returns	Sensor name string.
	*/
	virtual const char *get_sensor_name();

	/**
	* @brief	Get sensor data
	*
	* @param[in]	data_report sensor_report struct to fill out
	# @returns		0 on success, less than 0 otherwise
	*/
	virtual int get_sensor_report(ecg_sensor_report &data_report);

	/**
	* @brief	Dump Maxim Sensor registers.
	* @details	Fill out register values into list
	*
	* @returns	0 on success, negative error code on failure.
	*/
	virtual int dump_registers(addr_val_pair *reg_values);


	/**
	* @brief	Get sensor ID.
	*
	* @returns	Sensor ID number.
	*/
	unsigned char get_sensor_id();

	// Implemented functions transferred mostly from HSP....

	virtual int MS_max30001readRegister(uint8_t addr, uint32_t *return_data);

	virtual int MS_max30001writeRegister(uint8_t addr, uint32_t data);

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

	// This function enables the interrupts for specific parameters
	virtual int MS_max30001_INT_assignment(uint8_t en_enint_loc,     uint8_t en_eovf_loc,  uint8_t en_fstint_loc,
										   uint8_t en_dcloffint_loc, uint8_t en_bint_loc,  uint8_t en_bovf_loc,
										   uint8_t en_bover_loc,     uint8_t en_bundr_loc, uint8_t en_bcgmon_loc,
										   uint8_t en_pint_loc,      uint8_t en_povf_loc,  uint8_t en_pedge_loc,
										   uint8_t en_lonint_loc,    uint8_t en_rrint_loc, uint8_t en_samp_loc,
										   uint8_t intb_Type,        uint8_t int2b_Type);

	// call sync function of the sensor
	int MS_max30001sync();

	// That Function Should Only Be called from interrupt context
	void Max30001Helper_AddDataToQueue(uint32_t id, uint32_t *buffer, uint32_t number);


   /**
    *
    * @param[in] status the value to set the interrupt status
    * 			        variable
    */
	void  Max30001Helper_setInterruptStatus(bool status);

	/**
	 *
	 * @return status of the interrupt
	 */
	bool  Max30001Helper_getInterruptStatus();

	/**
	 * @brief Call the interrupt handler of Max30001
	 */
	void Max30001Helper_max30001_int_handler(void);

	int MAX30001_Helper_Queue_Size(void);

	int  MAX30001_Helper_Invert_Waveform(void);

	enum Max30001_Helper_ECG_Sample_Rate {
		k512SPS = 0b00,
		k256SPS = 0b01,
		k128SPS = 0b10
	};

	/**
	 *
	 * @param ecg_rate
	 * @return
	 */
	int Max30001Helper_SetECGSampleRate(Max30001_Helper_ECG_Sample_Rate ecg_rate);

private:

	/***
	 *
	 * @return
	 */
	void Max30001_Helper_ConvertRtoRtoBPM(uint16_t rtor_val);

	uint8_t m_rtor_last_bpm_;

	void Max30001_Helper_SetInterrupts(char en);

	/**
	 *
	 */
	int Max30001Helper_InitializeECGandRtoR();

	ecg_sensor_report m_ecg_packet_;
	MAX30001 *m_max30001;
	InterruptIn *m_max30001_InterruptB;
	InterruptIn *m_max30001_Interrupt2B;
	queue_t mmax30001_queue;
};


void StreamPacketUint32(uint32_t id, uint32_t *buffer, uint32_t number);

#endif
