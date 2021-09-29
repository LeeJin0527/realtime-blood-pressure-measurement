#include "MaximSensor.h"
#include "MAX30001_Helper.h"
#include "Peripherals.h"
#include "EcgComm_Defines.h"




/// define stream out fifo
#define MAX_ECG_REPORT_COUNT 256
/// allocate a large fifo buffer for streaming out
static uint8_t streamOutBuffer[sizeof(ecg_sensor_report) * MAX_ECG_REPORT_COUNT];


MAX30001_Helper::MAX30001_Helper(MAX30001 *m, InterruptIn *ir_B, InterruptIn *ir_2B)
:m_max30001(m), m_max30001_InterruptB(ir_B), m_max30001_Interrupt2B(ir_2B){
	queue_init(&mmax30001_queue, streamOutBuffer, sizeof(ecg_sensor_report), sizeof(streamOutBuffer));
	printf("Constructor is called\n");
	m_max30001->m_max30001_int_happened_ = false;
}


int MAX30001_Helper::readRegister(uint8_t reg, uint8_t *data, int len){
	printf("Empty Function is Called %s", __func__);
	return -1;
}

int MAX30001_Helper::writeRegister(uint8_t reg,	const uint8_t data){
	printf("Empty Function is Called %s", __func__);
	return -1;
}

int MAX30001_Helper::MS_max30001readRegister(uint8_t addr, uint32_t *return_data){
	return m_max30001->max30001_reg_read(static_cast<MAX30001::MAX30001_REG_map_t> (addr), return_data);
}

int MAX30001_Helper::MS_max30001writeRegister(uint8_t addr, uint32_t data){
	return m_max30001->max30001_reg_write(static_cast<MAX30001::MAX30001_REG_map_t> (addr), data);
}

int MAX30001_Helper::get_part_info(uint8_t *part_id,	uint8_t *rev_id){
	return -1;
}

void MAX30001_Helper::Max30001_Helper_SetInterrupts(char en){
	if(en){
		m_max30001_InterruptB->disable_irq();
		m_max30001_Interrupt2B->disable_irq();
		m_max30001_InterruptB->mode(PullUp);
		m_max30001_InterruptB->fall(&MAX30001Mid_IntB_Handler);
		m_max30001_Interrupt2B->mode(PullUp);
		m_max30001_Interrupt2B->fall(&MAX30001Mid_Int2B_Handler);
		m_max30001_InterruptB->enable_irq();
		m_max30001_Interrupt2B->enable_irq();
		MAX30001_AllowInterrupts(1);
		//ret |= m_max30001->max30001_sw_rst(); // Do a software reset of the MAX30001
		m_max30001->onDataAvailable(&StreamPacketUint32);
		m_max30001->m_max30001_int_happened_ = (false);
	}else{
		m_max30001_InterruptB->disable_irq();
		m_max30001_Interrupt2B->disable_irq();
		MAX30001_AllowInterrupts(0);
		//ret |= m_max30001->max30001_sw_rst(); // Do a software reset of the MAX30001
		m_max30001->m_max30001_int_happened_ = (false);
	}
}

int MAX30001_Helper::Max30001Helper_InitializeECGandRtoR(){
      int ret = 0;
	 /**** ENABLE CHANNELS for ECG ****/
	  // CNFG_GEN
	  if (m_max30001->max30001_reg_read(MAX30001::CNFG_GEN, &m_max30001->max30001_cnfg_gen.all) == -1) {
	    return -1;
	  }
	  m_max30001->max30001_cnfg_gen.bit.en_ecg = 0x1; // 0b1
	  // fmstr is default
	  if (m_max30001->max30001_reg_write(MAX30001::CNFG_GEN, m_max30001->max30001_cnfg_gen.all) == -1) {
	    return -1;
	  }
	  ret = m_max30001->max30001_PLL_lock();
	  // MNGR_INT
	  if (m_max30001->max30001_reg_read(MAX30001::MNGR_INT, &m_max30001->max30001_mngr_int.all) == -1) {
	    return -1;
	  }
	  m_max30001->max30001_mngr_int.bit.e_fit = E_FIT; // 31
//	  if (m_max30001->max30001_reg_write(MAX30001::MNGR_INT, m_max30001->max30001_mngr_int.all) == -1) {
//	    return -1;
//	  }
	  // end of MNGR_INT

	  // Configure RtoR Parameters
	  // MNGR_INT
//	  if (m_max30001->max30001_reg_read(MAX30001::MNGR_INT, &m_max30001->max30001_mngr_int.all) == -1) {
//	    return -1;
//	  }
	  m_max30001->max30001_mngr_int.bit.clr_rrint = 0x01; // 0b01 & 0b00 are for interrupt mode...
	  // 0b10 is for monitoring mode... it just overwrites the data...
	  if (m_max30001->max30001_reg_write(MAX30001::MNGR_INT, m_max30001->max30001_mngr_int.all) == -1) {
	    return -1;
	  }
	  // RTOR1
	  if (m_max30001->max30001_reg_read(MAX30001::CNFG_RTOR1, &m_max30001->max30001_cnfg_rtor1.all) == -1) {
	    return -1;
	  }
	  m_max30001->max30001_cnfg_rtor1.bit.en_rtor = 0x1;
	  if (m_max30001->max30001_reg_write(MAX30001::CNFG_RTOR1, m_max30001->max30001_cnfg_rtor1.all) == -1) {
	    return -1;
	  }
	  return ret;
}

int MAX30001_Helper::Max30001Helper_SetECGSampleRate(Max30001_Helper_ECG_Sample_Rate ecg_rate){
	  // ECG Rate
	  if (m_max30001->max30001_reg_read(MAX30001::CNFG_ECG, &m_max30001->max30001_cnfg_ecg.all) == -1) {
	    return -1;
	  }
	  m_max30001->max30001_cnfg_ecg.bit.rate = ecg_rate;
	  if (m_max30001->max30001_reg_write(MAX30001::CNFG_ECG, m_max30001->max30001_cnfg_ecg.all) == -1) {
	    return -1;
	  }
	  pr_debug("Setting reg 0x%02X to value 0x%02X\r\n", MAX30001::CNFG_ECG, ecg_rate);
	  return 0;
}


int MAX30001_Helper::sensor_enable(int enable){
	int ret = 0;
	queue_reset(&mmax30001_queue);
	memset(&m_ecg_packet_, 0, sizeof(m_ecg_packet_));
	m_rtor_last_bpm_ = 0;
	Max30001_Helper_SetInterrupts(enable);
	if(enable){
		ret = Max30001Helper_InitializeECGandRtoR();
		ret |= MS_Max30001_RtoR_InitStart(EN_RTOR_OP,WNDW_OP,GAIN_RTOR_OP,PAVG_OP,PTSF_OP,HOFF_OP,RAVG_OP,RHSF_OP,CLR_RRINT_OP);
		ret |= MS_max30001_INT_assignment(EN_ENINT_LOC, EN_EOVF_LOC,	EN_FSTINT_LOC,
										   EN_DCLOFFINT_LOC, EN_BINT_LOC,EN_BOVF_LOC,
										   EN_BOVER_LOC,EN_BUNDR_LOC,EN_BCGMON_LOC,
										   EN_PINT_LOC,EN_POVF_LOC,EN_PEDGE_LOC,
										   EN_LONINT_LOC,EN_RRINT_LOC,	EN_SAMP_LOC,
										   INTB_TYPE,INT2B_TYPE);
		ret |= MS_max30001sync();
	}else{
		ret  = m_max30001->max30001_Stop_ECG();
		ret |= m_max30001->max30001_Stop_RtoR();
		ret |= MS_max30001sync();
	}
	return ret;
}

int MAX30001_Helper::agc_enable(int agc_enable){
	return -1;
}

const char *MAX30001_Helper::get_sensor_part_name(){
	return "max30001";
}

const char *MAX30001_Helper::get_sensor_name(){
	return "max30001";
}

int MAX30001_Helper::get_sensor_report(ecg_sensor_report &sensor_report)
{
	return dequeue(&mmax30001_queue, &sensor_report);
}

int MAX30001_Helper::dump_registers(addr_val_pair *reg_vals){
	int i, j = 0;
	int ret = 0;
	//43 registers total
	for (i = 0x00; i <= 0x05; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x08; i <= 0x0A; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x0F; i <= 0x10; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x12; i <= 0x12; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x14; i <= 0x15; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x17; i <= 0x18; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x1A; i <= 0x1A; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x1D; i <= 0x1E; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x20; i <= 0x25; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}
	for (i = 0x30; i <= 0x47; i++, j++) {
		reg_vals[j].addr = i;
		ret |= MS_max30001readRegister(i, &(reg_vals[j].val));
	}

	return ret;
}


// This function starts ECG processing
int MAX30001_Helper::MS_Max30001_ECG_InitStart(uint8_t En_ecg, uint8_t Openp, uint8_t Openn,
						 uint8_t Pol, uint8_t Calp_sel, uint8_t Caln_sel,
									 uint8_t E_fit, uint8_t Rate, uint8_t Gain,
									 uint8_t Dhpf, uint8_t Dlpf){
	int ret = 0;
	ret = m_max30001->max30001_ECG_InitStart(En_ecg, Openp, Openn, Pol, Calp_sel, Caln_sel, E_fit, Rate, Gain, Dhpf, Dlpf);
	ret |= m_max30001->max30001_synch();
	return ret;
}

// This functions stops ECG processing
int MAX30001_Helper::MS_Max30001_ECG_Stop(){
	int ret = 0;
	ret =  m_max30001->max30001_Stop_ECG();
	return ret;
}

// This functions starts R_to_R processing
int MAX30001_Helper::MS_Max30001_RtoR_InitStart(uint8_t En_rtor, uint8_t Wndw, uint8_t Gain,
												uint8_t Pavg, uint8_t Ptsf, uint8_t Hoff,
												uint8_t Ravg, uint8_t Rhsf, uint8_t Clr_rrint){
	int ret = 0;
	ret =  m_max30001->max30001_RtoR_InitStart(En_rtor, Wndw, Gain,
							   Pavg, Ptsf, Hoff,
											   Ravg, Rhsf, Clr_rrint);
	ret |= m_max30001->max30001_synch();
	return ret;
}

// This functions stops R_to_R processing
int MAX30001_Helper::MS_Max30001_RtoR_Stop(){
	int ret = 0;
	ret =  m_max30001->max30001_Stop_RtoR();
	return ret;
}

//This function enables the interrupts for specific parameters
int MAX30001_Helper::MS_max30001_INT_assignment(uint8_t en_enint_loc,     uint8_t en_eovf_loc,  uint8_t en_fstint_loc,
												uint8_t en_dcloffint_loc, uint8_t en_bint_loc,  uint8_t en_bovf_loc,
									            uint8_t en_bover_loc,     uint8_t en_bundr_loc, uint8_t en_bcgmon_loc,
									            uint8_t en_pint_loc,      uint8_t en_povf_loc,  uint8_t en_pedge_loc,
									            uint8_t en_lonint_loc,    uint8_t en_rrint_loc, uint8_t en_samp_loc,
									            uint8_t intb_Type,        uint8_t int2b_Type){

	return m_max30001->max30001_INT_assignment(static_cast<MAX30001::max30001_intrpt_Location_t> (en_enint_loc),     static_cast<MAX30001::max30001_intrpt_Location_t> (en_eovf_loc),  static_cast<MAX30001::max30001_intrpt_Location_t> (en_fstint_loc),
											   static_cast<MAX30001::max30001_intrpt_Location_t> (en_dcloffint_loc), static_cast<MAX30001::max30001_intrpt_Location_t> (en_bint_loc),  static_cast<MAX30001::max30001_intrpt_Location_t> (en_bovf_loc),
											   static_cast<MAX30001::max30001_intrpt_Location_t> (en_bover_loc),     static_cast<MAX30001::max30001_intrpt_Location_t> (en_bundr_loc), static_cast<MAX30001::max30001_intrpt_Location_t> (en_bcgmon_loc),
											   static_cast<MAX30001::max30001_intrpt_Location_t> (en_pint_loc),      static_cast<MAX30001::max30001_intrpt_Location_t> (en_povf_loc),  static_cast<MAX30001::max30001_intrpt_Location_t> (en_pedge_loc),
											   static_cast<MAX30001::max30001_intrpt_Location_t> (en_lonint_loc),    static_cast<MAX30001::max30001_intrpt_Location_t> (en_rrint_loc), static_cast<MAX30001::max30001_intrpt_Location_t> (en_samp_loc),
											   static_cast<MAX30001::max30001_intrpt_type_t> (intb_Type),            static_cast<MAX30001::max30001_intrpt_type_t> (int2b_Type));

}

// call sync function of the sensor
int MAX30001_Helper::MS_max30001sync(){
	return m_max30001->max30001_synch();
}

// That Function Should Only Be called from interrupt context
void MAX30001_Helper::Max30001Helper_AddDataToQueue(uint32_t id, uint32_t *buffer, uint32_t number){
	for(uint32_t i = 0; i != number; ++i){
		if(id == MAX30001_DATA_ECG){
			m_ecg_packet_.ecg = buffer[i];
			enqueue(&mmax30001_queue, &m_ecg_packet_);
			m_ecg_packet_.rtor = 0;
			m_ecg_packet_.rtor_bpm = 0;
		}else if(id == MAX30001_DATA_RTOR){
			m_ecg_packet_.rtor = buffer[i];
			Max30001_Helper_ConvertRtoRtoBPM(m_ecg_packet_.rtor);
			m_ecg_packet_.rtor_bpm = m_rtor_last_bpm_;
		}
		else{
		}
	}
}

void MAX30001_Helper::Max30001Helper_setInterruptStatus(bool status) {
	m_max30001->m_max30001_int_happened_ = status;
}

bool MAX30001_Helper::Max30001Helper_getInterruptStatus() {
	return m_max30001->m_max30001_int_happened_;
}

void MAX30001_Helper::Max30001Helper_max30001_int_handler(void){
	m_max30001->max30001_int_handler();
}

int MAX30001_Helper::MAX30001_Helper_Queue_Size(void){
	return queue_len(&mmax30001_queue);
}

int MAX30001_Helper:: MAX30001_Helper_Invert_Waveform(void) {
	int ret;
	uint32_t return_data;
	ret = m_max30001->max30001_reg_read(MAX30001::CNFG_EMUX, &return_data);
	if(ret != 0)
		return ret;

	return_data ^= ((1 << 23));
	ret = m_max30001->max30001_reg_write(MAX30001::CNFG_EMUX, return_data);
	return ret;
}

void MAX30001_Helper::Max30001_Helper_ConvertRtoRtoBPM(uint16_t rtor_val) {
	const float rtor_constant = 0.0078;
	if(rtor_val != 0)
		m_rtor_last_bpm_ = (uint8_t)(60 /(rtor_constant* rtor_val));
}

void StreamPacketUint32(uint32_t id, uint32_t *buffer, uint32_t number) {
	Peripherals::max30001Helper()->Max30001Helper_AddDataToQueue(id, buffer, number);
}

/**
* @brief	Get sensor ID.
*
* @returns	Sensor ID number.
*/
unsigned char MAX30001_Helper::get_sensor_id() {

	return( SENSOR_ID_MAX30001 );

}
