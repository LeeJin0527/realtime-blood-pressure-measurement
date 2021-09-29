
/*******************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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

#include "mbed.h"
#include "MAX30001.h"
#include "Peripherals.h"

MAX30001 *MAX30001::instance = NULL;


//******************************************************************************
MAX30001::MAX30001(PinName mosi, PinName miso, PinName sclk, PinName cs)
{
  m_spi = new SPI(mosi, miso, sclk);
  m_cs = new DigitalOut(cs, 1);

  m_spi->frequency(3000000);
  spi_owner = true;
  functionpointer = spiHandler;
  onDataAvailableCallback = NULL;
  instance = this;

}


//******************************************************************************
MAX30001::MAX30001(SPI *spi, DigitalOut *cs)
{
  m_spi = spi;
  m_cs = cs;
  spi->frequency(3000000);
  spi_owner = false;
  functionpointer = spiHandler;
  onDataAvailableCallback = NULL;
  instance = this;


}


//******************************************************************************
MAX30001::~MAX30001(void) {

  if (spi_owner) {
    delete m_spi;
    delete m_cs;
  }
}

//******************************************************************************
int MAX30001::max30001_Rbias_FMSTR_Init(uint8_t En_rbias, uint8_t Rbiasv,
                                        uint8_t Rbiasp, uint8_t Rbiasn,
                                        uint8_t Fmstr) {
  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_rbias = En_rbias;
  max30001_cnfg_gen.bit.rbiasv   = Rbiasv;
  max30001_cnfg_gen.bit.rbiasp   = Rbiasp;
  max30001_cnfg_gen.bit.rbiasn   = Rbiasn;
  max30001_cnfg_gen.bit.fmstr    = Fmstr;

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }
  return 0;
}

//******************************************************************************
int MAX30001::max30001_CAL_InitStart(uint8_t En_Vcal, uint8_t Vmode,
                                     uint8_t Vmag, uint8_t Fcal, uint16_t Thigh,
                                     uint8_t Fifty) {
  // CNFG_CAL
  if (max30001_reg_read(CNFG_CAL, &max30001_cnfg_cal.all) == -1) {
    return -1;
  }

  max30001_cnfg_cal.bit.vmode = Vmode;
  max30001_cnfg_cal.bit.vmag  = Vmag;
  max30001_cnfg_cal.bit.fcal  = Fcal;
  max30001_cnfg_cal.bit.thigh = Thigh;
  max30001_cnfg_cal.bit.fifty = Fifty;

  if (max30001_reg_write(CNFG_CAL, max30001_cnfg_cal.all) == -1) {
    return -1;
  }

  // RTOS uses a 32768HZ clock.  32768ticks represents 1secs.  1sec/10 =
  // 100msecs.
  wait(1.0 / 10.0);

  if (max30001_reg_read(CNFG_CAL, &max30001_cnfg_cal.all) == -1) {
    return -1;
  }

  max30001_cnfg_cal.bit.en_vcal = En_Vcal;

  if (max30001_reg_write(CNFG_CAL, max30001_cnfg_cal.all) == -1) {
    return -1;
  }

  // RTOS uses a 32768HZ clock.  32768ticks represents 1secs.  1sec/10 =
  // 100msecs.
  wait(1.0 / 10.0);

  return 0;
}

//******************************************************************************
int MAX30001::max30001_CAL_Stop(void) {

  if (max30001_reg_read(CNFG_CAL, &max30001_cnfg_cal.all) == -1) {
    return -1;
  }

  max30001_cnfg_cal.bit.en_vcal = 0; // Disable VCAL, all other settings are left unaffected

  if (max30001_reg_write(CNFG_CAL, max30001_cnfg_cal.all) == -1) {
    return -1;
  }

  return 0;
}
//******************************************************************************
//******************************************************************************
int MAX30001::max30001_INT_assignment(max30001_intrpt_Location_t en_enint_loc,     max30001_intrpt_Location_t en_eovf_loc,  max30001_intrpt_Location_t en_fstint_loc,
		                      max30001_intrpt_Location_t en_dcloffint_loc, max30001_intrpt_Location_t en_bint_loc,  max30001_intrpt_Location_t en_bovf_loc,
		                      max30001_intrpt_Location_t en_bover_loc,     max30001_intrpt_Location_t en_bundr_loc, max30001_intrpt_Location_t en_bcgmon_loc,
		                      max30001_intrpt_Location_t en_pint_loc,      max30001_intrpt_Location_t en_povf_loc,  max30001_intrpt_Location_t en_pedge_loc,
		                      max30001_intrpt_Location_t en_lonint_loc,    max30001_intrpt_Location_t en_rrint_loc, max30001_intrpt_Location_t en_samp_loc,
		                      max30001_intrpt_type_t  intb_Type,           max30001_intrpt_type_t int2b_Type)


{
  // INT1

  pr_info("MAX30001::max30001_INT_assignment\r\n");
  if (max30001_reg_read(EN_INT, &max30001_en_int.all) == -1) {
    return -1;
  }

  // max30001_en_int2.bit.en_pint       = 0b1;  // Keep this off...

  max30001_en_int.bit.en_eint = 0b1 & en_enint_loc;
  max30001_en_int.bit.en_eovf = 0b1 & en_eovf_loc;
  max30001_en_int.bit.en_fstint = 0b1 & en_fstint_loc;

  max30001_en_int.bit.en_dcloffint = 0b1 & en_dcloffint_loc;
  max30001_en_int.bit.en_bint = 0b1 & en_bint_loc;
  max30001_en_int.bit.en_bovf = 0b1 & en_bovf_loc;

  max30001_en_int.bit.en_bover = 0b1 & en_bover_loc;
  max30001_en_int.bit.en_bundr = 0b1 & en_bundr_loc;
  max30001_en_int.bit.en_bcgmon = 0b1 & en_bcgmon_loc;

  max30001_en_int.bit.en_pint = 0b1 & en_pint_loc;
  max30001_en_int.bit.en_povf = 0b1 & en_povf_loc;
  max30001_en_int.bit.en_pedge = 0b1 & en_pedge_loc;

  max30001_en_int.bit.en_lonint = 0b1 & en_lonint_loc;
  max30001_en_int.bit.en_rrint = 0b1 & en_rrint_loc;
  max30001_en_int.bit.en_samp = 0b1 & en_samp_loc;

  max30001_en_int.bit.intb_type = intb_Type;

  if (max30001_reg_write(EN_INT, max30001_en_int.all) == -1) {
    return -1;
  }

  // INT2

  if (max30001_reg_read(EN_INT2, &max30001_en_int2.all) == -1) {
    return -1;
  }

  max30001_en_int2.bit.en_eint   = 0b1 & (en_enint_loc >> 1);
  max30001_en_int2.bit.en_eovf   = 0b1 & (en_eovf_loc >> 1);
  max30001_en_int2.bit.en_fstint = 0b1 & (en_fstint_loc >> 1);

  max30001_en_int2.bit.en_dcloffint = 0b1 & (en_dcloffint_loc >> 1);
  max30001_en_int2.bit.en_bint      = 0b1 & (en_bint_loc >> 1);
  max30001_en_int2.bit.en_bovf      = 0b1 & (en_bovf_loc >> 1);

  max30001_en_int2.bit.en_bover  = 0b1 & (en_bover_loc >> 1);
  max30001_en_int2.bit.en_bundr  = 0b1 & (en_bundr_loc >> 1);
  max30001_en_int2.bit.en_bcgmon = 0b1 & (en_bcgmon_loc >> 1);

  max30001_en_int2.bit.en_pint  = 0b1 & (en_pint_loc >> 1);
  max30001_en_int2.bit.en_povf  = 0b1 & (en_povf_loc >> 1);
  max30001_en_int2.bit.en_pedge = 0b1 & (en_pedge_loc >> 1);

  max30001_en_int2.bit.en_lonint = 0b1 & (en_lonint_loc >> 1);
  max30001_en_int2.bit.en_rrint  = 0b1 & (en_rrint_loc >> 1);
  max30001_en_int2.bit.en_samp   = 0b1 & (en_samp_loc >> 1);

  max30001_en_int2.bit.intb_type = int2b_Type;

  if (max30001_reg_write(EN_INT2, max30001_en_int2.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_ECG_InitStart(uint8_t En_ecg, uint8_t Openp,
                                     uint8_t Openn, uint8_t Pol,
                                     uint8_t Calp_sel, uint8_t Caln_sel,
                                     uint8_t E_fit, uint8_t Rate, uint8_t Gain,
                                     uint8_t Dhpf, uint8_t Dlpf) {

  // CNFG_EMUX

  pr_info("MAX30001::max30001_ECG_InitStart\r\n");
  if (max30001_reg_read(CNFG_EMUX, &max30001_cnfg_emux.all) == -1) {
    return -1;
  }

  max30001_cnfg_emux.bit.openp    = Openp;
  max30001_cnfg_emux.bit.openn    = Openn;
  max30001_cnfg_emux.bit.pol      = Pol;
  max30001_cnfg_emux.bit.calp_sel = Calp_sel;
  max30001_cnfg_emux.bit.caln_sel = Caln_sel;

  if (max30001_reg_write(CNFG_EMUX, max30001_cnfg_emux.all) == -1) {
    return -1;
  }

  /**** ENABLE CHANNELS ****/
  // CNFG_GEN

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_ecg = En_ecg; // 0b1

  // fmstr is default

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  /**** Wait for PLL Lock & References to settle down ****/

  max30001_timeout = 0;

  do {
    if (max30001_reg_read(STATUS, &max30001_status.all) == -1) // Wait and spin for PLL to lock...
    {
      return -1;
    }
  } while (max30001_status.bit.pllint == 1 && max30001_timeout++ <= 1000);

  // MNGR_INT

  if (max30001_reg_read(MNGR_INT, &max30001_mngr_int.all) == -1) {
    return -1;
  }

  max30001_mngr_int.bit.e_fit = E_fit; // 31

  if (max30001_reg_write(MNGR_INT, max30001_mngr_int.all) == -1) {
    return -1;
  }

  // CNFG_ECG

  if (max30001_reg_read(CNFG_ECG, &max30001_cnfg_ecg.all) == -1) {
    return -1;
  }

  max30001_cnfg_ecg.bit.rate = Rate;
  max30001_cnfg_ecg.bit.gain = Gain;
  max30001_cnfg_ecg.bit.dhpf = Dhpf;
  max30001_cnfg_ecg.bit.dlpf = Dlpf;

  if (max30001_reg_write(CNFG_ECG, max30001_cnfg_ecg.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_ECGFast_Init(uint8_t Clr_Fast, uint8_t Fast, uint8_t Fast_Th) {
  if (max30001_reg_read(MNGR_INT, &max30001_mngr_int.all) == -1) {
    return -1;
  }

  pr_info("MAX30001::max30001_ECGFast_Init\r\n");
  max30001_mngr_int.bit.clr_fast = Clr_Fast;

  if (max30001_reg_write(MNGR_INT, max30001_mngr_int.all) == -1) {
    return -1;
  }

  if (max30001_reg_read(MNGR_DYN, &max30001_mngr_dyn.all) == -1) {
    return -1;
  }

  max30001_mngr_dyn.bit.fast = Fast;
  max30001_mngr_dyn.bit.fast_th = Fast_Th;

  if (max30001_reg_write(MNGR_INT, max30001_mngr_int.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_Stop_ECG(void) {

  pr_info("MAX30001::max30001_Stop_ECG\r\n");

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_ecg = 0; // Stop ECG

  // fmstr is default

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_PACE_InitStart(uint8_t En_pace, uint8_t Clr_pedge,
                                      uint8_t Pol, uint8_t Gn_diff_off,
                                      uint8_t Gain, uint8_t Aout_lbw,
                                      uint8_t Aout, uint8_t Dacp,
                                      uint8_t Dacn) {

  /**** SET MASTER FREQUENCY, ENABLE CHANNELS ****/

  // CNFG_GEN

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_pace = En_pace; // 0b1;

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  /**** Wait for PLL Lock & References to settle down ****/
  max30001_timeout = 0;

  do {
    if (max30001_reg_read(STATUS, &max30001_status.all) ==
        -1) // Wait and spin for PLL to lock...
    {
      return -1;
    }

  } while (max30001_status.bit.pllint == 1 && max30001_timeout++ <= 1000);

  // MNGR_INT

  if (max30001_reg_read(MNGR_INT, &max30001_mngr_int.all) == -1) {
    return -1;
  }

  max30001_mngr_int.bit.clr_pedge = Clr_pedge; // 0b0;

  if (max30001_reg_write(MNGR_INT, max30001_mngr_int.all) == -1) {
    return -1;
  }

  /* Put: CNFG_PACE */

  max30001_reg_read(CNFG_PACE, &max30001_cnfg_pace.all);

  max30001_cnfg_pace.bit.pol         = Pol;
  max30001_cnfg_pace.bit.gn_diff_off = Gn_diff_off;
  max30001_cnfg_pace.bit.gain        = Gain;
  max30001_cnfg_pace.bit.aout_lbw    = Aout_lbw;
  max30001_cnfg_pace.bit.aout        = Aout;
  max30001_cnfg_pace.bit.dacp        = Dacp;
  max30001_cnfg_pace.bit.dacn        = Dacn;

  max30001_reg_write(CNFG_PACE, max30001_cnfg_pace.all);

  return 0;
}
//******************************************************************************
int MAX30001::max30001_Stop_PACE(void) {

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_pace = 0; // Stop PACE

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_BIOZ_InitStart(
    uint8_t En_bioz, uint8_t Openp, uint8_t Openn, uint8_t Calp_sel,
    uint8_t Caln_sel, uint8_t CG_mode, uint8_t B_fit, uint8_t Rate,
    uint8_t Ahpf, uint8_t Ext_rbias, uint8_t Gain, uint8_t Dhpf, uint8_t Dlpf,
    uint8_t Fcgen, uint8_t Cgmon, uint8_t Cgmag, uint8_t Phoff, uint8_t Inapow_mode) {

  // CNFG_BMUX

  if (max30001_reg_read(CNFG_BMUX, &max30001_cnfg_bmux.all) == -1) {
    return -1;
  }

  max30001_cnfg_bmux.bit.openp    = Openp;       // 0b1;
  max30001_cnfg_bmux.bit.openn    = Openn;       // 0b1;
  max30001_cnfg_bmux.bit.calp_sel = Calp_sel; // 0b10;
  max30001_cnfg_bmux.bit.caln_sel = Caln_sel; // 0b11;
  max30001_cnfg_bmux.bit.cg_mode  = CG_mode;   // 0b00;

  if (max30001_reg_write(CNFG_BMUX, max30001_cnfg_bmux.all) == -1) {
    return -1;
  }

  /**** SET MASTER FREQUENCY, ENABLE CHANNELS ****/

  // CNFG_GEN

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_bioz = En_bioz;

  // fmstr is default

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  /**** Wait for PLL Lock & References to settle down ****/

  max30001_timeout = 0;

  do {
    if (max30001_reg_read(STATUS, &max30001_status.all) ==
        -1) // Wait and spin for PLL to lock...
    {
      return -1;
    }

  } while (max30001_status.bit.pllint == 1 && max30001_timeout++ <= 1000);

  /**** Start of CNFG_BIOZ ****/

  // MNGR_INT

  if (max30001_reg_read(MNGR_INT, &max30001_mngr_int.all) == -1) {
    return -1;
  }

  max30001_mngr_int.bit.b_fit = B_fit; //;

  if (max30001_reg_write(MNGR_INT, max30001_mngr_int.all) == -1) {
    return -1;
  }

  // CNFG_BIOZ

  if (max30001_reg_read(CNFG_BIOZ, &max30001_cnfg_bioz.all) == -1) {
    return -1;
  }

  max30001_cnfg_bioz.bit.rate      = Rate;
  max30001_cnfg_bioz.bit.ahpf      = Ahpf;
  max30001_cnfg_bioz.bit.ext_rbias = Ext_rbias;
  max30001_cnfg_bioz.bit.gain      = Gain;
  max30001_cnfg_bioz.bit.dhpf      = Dhpf;
  max30001_cnfg_bioz.bit.dlpf      = Dlpf;
  max30001_cnfg_bioz.bit.fcgen     = Fcgen;
  max30001_cnfg_bioz.bit.cgmon     = Cgmon;
  max30001_cnfg_bioz.bit.cgmag     = Cgmag;
  max30001_cnfg_bioz.bit.phoff     = Phoff;
  max30001_cnfg_bioz.bit.inapow_mode = Inapow_mode;

  if (max30001_reg_write(CNFG_BIOZ, max30001_cnfg_bioz.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_Stop_BIOZ(void) {

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_bioz = 0; // Stop BIOZ

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_BIOZ_InitBist(uint8_t En_bist, uint8_t Rnom,
                                     uint8_t Rmod, uint8_t Fbist) {

  // CNFG_BMUX

  if (max30001_reg_read(CNFG_BMUX, &max30001_cnfg_bmux.all) == -1) {
    return -1;
  }

  max30001_cnfg_bmux.bit.en_bist = En_bist;
  max30001_cnfg_bmux.bit.rnom = Rnom;
  max30001_cnfg_bmux.bit.rmod = Rmod;
  max30001_cnfg_bmux.bit.fbist = Fbist;

  if (max30001_reg_write(CNFG_BMUX, max30001_cnfg_bmux.all) == -1) {
    return -1;
  }

  return 0;
}
//******************************************************************************
int MAX30001::max30001_RtoR_InitStart(uint8_t En_rtor, uint8_t Wndw,
                                      uint8_t Gain, uint8_t Pavg, uint8_t Ptsf,
                                      uint8_t Hoff, uint8_t Ravg, uint8_t Rhsf,
                                      uint8_t Clr_rrint) {

  // MNGR_INT

  if (max30001_reg_read(MNGR_INT, &max30001_mngr_int.all) == -1) {
    return -1;
  }

  max30001_mngr_int.bit.clr_rrint =
      Clr_rrint; // 0b01 & 0b00 are for interrupt mode...
  // 0b10 is for monitoring mode... it just overwrites the data...

  if (max30001_reg_write(MNGR_INT, max30001_mngr_int.all) == -1) {
    return -1;
  }

  // RTOR1
  if (max30001_reg_read(CNFG_RTOR1, &max30001_cnfg_rtor1.all) == -1) {
    return -1;
  }

  max30001_cnfg_rtor1.bit.wndw = Wndw;
  max30001_cnfg_rtor1.bit.gain = Gain;
  max30001_cnfg_rtor1.bit.en_rtor = En_rtor;
  max30001_cnfg_rtor1.bit.pavg = Pavg;
  max30001_cnfg_rtor1.bit.ptsf = Ptsf;

  if (max30001_reg_write(CNFG_RTOR1, max30001_cnfg_rtor1.all) == -1) {
    return -1;
  }
  // RTOR2

  if (max30001_reg_read(CNFG_RTOR2, &max30001_cnfg_rtor2.all) == -1) {
    return -1;
  }
  max30001_cnfg_rtor2.bit.hoff = Hoff;
  max30001_cnfg_rtor2.bit.ravg = Ravg;
  max30001_cnfg_rtor2.bit.rhsf = Rhsf;

  if (max30001_reg_write(CNFG_RTOR2, max30001_cnfg_rtor2.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_Stop_RtoR(void) {

  if (max30001_reg_read(CNFG_RTOR1, &max30001_cnfg_rtor1.all) == -1) {
    return -1;
  }

  max30001_cnfg_rtor1.bit.en_rtor = 0; // Stop RtoR

  if (max30001_reg_write(CNFG_RTOR1, max30001_cnfg_rtor1.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_PLL_lock(void) {
  // Spin to see PLLint become zero to indicate a lock.

  max30001_timeout = 0;

  do {
    if (max30001_reg_read(STATUS, &max30001_status.all) ==
        -1) // Wait and spin for PLL to lock...
    {
      return -1;
    }

  } while (max30001_status.bit.pllint == 1 && max30001_timeout++ <= 1000);

  return 0;
}

//******************************************************************************
int MAX30001::max30001_sw_rst(void) {
  // SW reset for the MAX30001 chip

  if (max30001_reg_write(SW_RST, 0x000000) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_synch(void) { // For synchronization
  if (max30001_reg_write(SYNCH, 0x000000) == -1) {
    return -1;
  }
  return 0;
}

//******************************************************************************
int MAX30001::max300001_fifo_rst(void) { // Resets the FIFO
  if (max30001_reg_write(FIFO_RST, 0x000000) == -1) {
    return -1;
  }
  return 0;
}

//******************************************************************************
// int MAX30001::max30001_reg_write(uint8_t addr, uint32_t data)
int MAX30001::max30001_reg_write(MAX30001_REG_map_t addr, uint32_t data) {

  uint8_t result[4];
  uint8_t data_array[4];
  int32_t success = 0;

  data_array[0] = (addr << 1) & 0xff;

  data_array[3] = data & 0xff;
  data_array[2] = (data >> 8) & 0xff;
  data_array[1] = (data >> 16) & 0xff;

  success = SPI_Transmit(&data_array[0], 4, &result[0], 4);

  //pr_info("MAX30001 write\treg[0x%.2X]=0x%X\r\n", addr, data);

  if (success != 0) {
    return -1;
  } else {
    return 0;
  }
}

//******************************************************************************
// int MAX30001::max30001_reg_read(uint8_t addr, uint32_t *return_data)
int MAX30001::max30001_reg_read(MAX30001_REG_map_t addr,
                                uint32_t *return_data) {
  uint8_t result[4];
  uint8_t data_array[1];
  int32_t success = 0;

  data_array[0] = ((addr << 1) & 0xff) | 1; // For Read, Or with 1
  success = SPI_Transmit(&data_array[0], 1, &result[0], 4);
  *return_data = /*result[0] + */ (uint32_t)(result[1] << 16) +
                 (result[2] << 8) + result[3];

  //pr_info("MAX30001 read\treg[0x%.2X]=0x%X\r\n", addr, *return_data);

  if (success != 0) {
    return -1;
  } else {
    return 0;
  }
}

//******************************************************************************
int MAX30001::max30001_Enable_DcLeadOFF_Init(int8_t En_dcloff, int8_t Ipol,
                                             int8_t Imag, int8_t Vth) {
  //  the leads are not touching the body

  // CNFG_EMUX, Set ECGP and ECGN for external hook up...

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_dcloff = En_dcloff;
  max30001_cnfg_gen.bit.ipol = Ipol;
  max30001_cnfg_gen.bit.imag = Imag;
  max30001_cnfg_gen.bit.vth = Vth;

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_Disable_DcLeadOFF(void) {
  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_dcloff = 0; // Turned off the dc lead off.

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_BIOZ_Enable_ACLeadOFF_Init(uint8_t En_bloff,
                                                  uint8_t Bloff_hi_it,
                                                  uint8_t Bloff_lo_it) {

  // CNFG_GEN
  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_bloff = En_bloff;

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  // MNGR_DYN
  if (max30001_reg_read(MNGR_DYN, &max30001_mngr_dyn.all) == -1) {
    return -1;
  }

  max30001_mngr_dyn.bit.bloff_hi_it = Bloff_hi_it;
  max30001_mngr_dyn.bit.bloff_lo_it = Bloff_lo_it;

  if (max30001_reg_write(MNGR_DYN, max30001_mngr_dyn.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_BIOZ_Disable_ACleadOFF(void) {
  // CNFG_GEN
  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_bloff = 0b0; // Turns of the BIOZ AC Lead OFF feature

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  return 0;
}

//******************************************************************************
int MAX30001::max30001_BIOZ_Enable_BCGMON(void) {
  // CNFG_BIOZ
  if (max30001_reg_read(CNFG_BIOZ, &max30001_cnfg_bioz.all) == -1) {
    return -1;
  }

  max30001_cnfg_bioz.bit.cgmon = 1;

  if (max30001_reg_write(CNFG_BIOZ, max30001_cnfg_bioz.all) == -1) {
    return -1;
  }

  max30001_reg_read(CNFG_BIOZ, &max30001_cnfg_bioz.all);

  return 0;
}

#if 1
//******************************************************************************
int MAX30001::max30001_Enable_LeadON(int8_t Channel) // Channel: ECG = 0b01, BIOZ = 0b10, Disable = 0b00
{

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_ecg  = 0b0;
  max30001_cnfg_gen.bit.en_bioz = 0b0;
  max30001_cnfg_gen.bit.en_pace = 0b0;

  max30001_cnfg_gen.bit.en_ulp_lon = Channel; // BIOZ ULP lead on detection...

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all);

  max30001_reg_read(STATUS, &max30001_status.all);

  return 0;
}
//******************************************************************************
int MAX30001::max30001_Disable_LeadON(void) {

  if (max30001_reg_read(CNFG_GEN, &max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  max30001_cnfg_gen.bit.en_ulp_lon = 0b0;

  if (max30001_reg_write(CNFG_GEN, max30001_cnfg_gen.all) == -1) {
    return -1;
  }

  return 0;
}
#endif
//******************************************************************************
#define LEADOFF_SERVICE_TIME 0x2000 // 0x1000 = 1 second
#define LEADOFF_NUMSTATES 2
uint32_t leadoffState = 0;
uint32_t max30001_LeadOffoldTime = 0;
void MAX30001::max30001_ServiceLeadoff(uint32_t currentTime) {

  uint32_t delta_Time;

  delta_Time = currentTime - max30001_LeadOffoldTime;

  if (delta_Time > LEADOFF_SERVICE_TIME) {
    switch (leadoffState) {
    case 0: /* switch to ECG DC Lead OFF */
      max30001_Enable_DcLeadOFF_Init(0b01, 0b0, 0b001, 0b00);
      break;

    case 1: /* switch to BIOZ DC Lead OFF */
      max30001_Enable_DcLeadOFF_Init(0b10, 0b0, 0b001, 0b00);
      break;
    }

    leadoffState++;
    leadoffState %= LEADOFF_NUMSTATES;

    max30001_LeadOffoldTime = currentTime;
  }
}
//******************************************************************************
#define LEADON_SERVICE_TIME 0x2000 // 0x1000 = 1 second
#define LEADON_NUMSTATES 2
uint32_t leadOnState = 0;
uint32_t max30001_LeadOnoldTime = 0;
void MAX30001::max30001_ServiceLeadON(uint32_t currentTime) {

  uint32_t delta_Time;

  delta_Time = currentTime - max30001_LeadOnoldTime;

  if (delta_Time > LEADON_SERVICE_TIME) {
    switch (leadOnState) {
    case 0: /* switch to ECG DC Lead ON */
      max30001_Enable_LeadON(0b01);
      break;

    case 1: /* switch to BIOZ DC Lead ON */
      max30001_Enable_LeadON(0b10);
      break;
    }

    leadOnState++;
    leadOnState %= LEADON_NUMSTATES;

    max30001_LeadOnoldTime = currentTime;
  }
}

//******************************************************************************
int MAX30001::max30001_FIFO_LeadONOff_Read(void) {

  uint8_t result[32 * 3]; // 32words - 3bytes each
  uint8_t paceResult[6 * 3];
  uint8_t data_array[4];
  int32_t success = 0;
  int i, j;

  uint32_t total_databytes;
  uint8_t i_index;
  uint8_t data_chunk;
  uint8_t loop_logic;

  uint8_t etag = 0;
  uint8_t ptag = 0;
  uint8_t btag = 0;

  uint8_t adr;

  //int8_t ReadAllPaceOnce;
  bool anyPaceDetected = false;
  static uint8_t dcloffint_OneShot = 0;
  static uint8_t acloffint_OneShot = 0;
  static uint8_t bcgmon_OneShot = 0;
  static uint8_t acleadon_OneShot = 0;

  int8_t ret_val;

  pr_info("MAX30001::max30001_FIFO_LeadONOff_Read\r\n");

  if (max30001_status.bit.eint == 1) {
    adr = ECG_FIFO_BURST;
    data_array[0] = ((adr << 1) & 0xff) | 1;

    // The SPI routine only sends out data of 32 bytes in size.  Therefore the
    // data is being read in
    // smaller chunks in this routine...

    total_databytes = (max30001_mngr_int.bit.e_fit + 1) * 3;

    i_index = 0;
    loop_logic = 1;

    while (loop_logic) {
      if (total_databytes > 30) {
        data_chunk = 30;
        total_databytes = total_databytes - 30;
      } else {
        data_chunk = total_databytes;
        loop_logic = 0;
      }

      /* The extra 1 byte is for the extra byte that comes out of the SPI */
      success = SPI_Transmit(&data_array[0], 1, &result[i_index], (data_chunk + 1)); // Make a copy of the FIFO over here...

      if (success != 0) {
        return -1;
      }

      /* This is important, because every transaction above creates an empty
       * redundant data at result[0] */
      for (j = i_index; j < (data_chunk + i_index); j++) /* get rid of the 1 extra byte by moving the whole array up one */
      {
        result[j] = result[j + 1];
      }

      i_index = i_index + 30; /* point to the next array location to put the data in */
    }

    //ReadAllPaceOnce = 0;

    /* Put the content of the FIFO based on the EFIT value, We ignore the
     * result[0] and start concatenating indexes: 1,2,3 - 4,5,6 - 7,8,9 -  */
    for (i = 0, j = 0; i < max30001_mngr_int.bit.e_fit + 1; i++, j = j + 3) // index1=23-16 bit, index2=15-8 bit, index3=7-0 bit
    {
      max30001_ECG_FIFO_buffer[i] = ((uint32_t)result[j] << 16) + (result[j + 1] << 8) + result[j + 2];

      etag = (0b00111000 & result[j + 2]) >> 3;
      ptag = 0b00000111 & result[j + 2];

      if (ptag != 0b111 ){//&& ReadAllPaceOnce == 0) {

        //ReadAllPaceOnce = 1; // This will prevent extra read of PACE, once group
                             // 0-5 is read ONCE.
		readPace(ptag, paceResult); // BUG: result data from ECG is being overwritten by the PACE data
	anyPaceDetected = true;
      }
    }

    if (anyPaceDetected)
      dataAvailable(MAX30001_DATA_PACE, max30001_PACE, 18); // Send out the Pace data once only

    if (etag != 0b110) {

      dataAvailable(MAX30001_DATA_ECG, max30001_ECG_FIFO_buffer, (max30001_mngr_int.bit.e_fit + 1));
    }

  } /* End of ECG init */

  /* RtoR */

  if (max30001_status.bit.rrint == 1) {
    if (max30001_reg_read(RTOR, &max30001_RtoR_data) == -1) {
      return -1;
    }

    max30001_RtoR_data = (0x00FFFFFF & max30001_RtoR_data) >> 10;

    hspValMax30001.R2R = (uint16_t)max30001_RtoR_data;
    hspValMax30001.fmstr = (uint16_t)max30001_cnfg_gen.bit.fmstr;

    dataAvailable(MAX30001_DATA_RTOR, &max30001_RtoR_data, 1);
  }

  // Handling BIOZ data...

  if (max30001_status.bit.bint == 1) {
    adr = 0x22;
    data_array[0] = ((adr << 1) & 0xff) | 1;

    /* [(BFIT+1)*3byte]+1extra byte due to the addr */

    if (SPI_Transmit(&data_array[0], 1, &result[0],((max30001_mngr_int.bit.b_fit + 1) * 3) + 1) == -1) // Make a copy of the FIFO over here...

    {
      return -1;
    }

    btag = 0b00000111 & result[3];

    /* Put the content of the FIFO based on the BFIT value, We ignore the
     * result[0] and start concatenating indexes: 1,2,3 - 4,5,6 - 7,8,9 -  */
    for (i = 0, j = 0; i < max30001_mngr_int.bit.b_fit + 1; i++, j = j + 3) // index1=23-16 bit, index2=15-8 bit, index3=7-0 bit
    {
      max30001_BIOZ_FIFO_buffer[i] = ((uint32_t)result[j + 1] << 16) + (result[j + 2] << 8) + result[j + 3];
    }

    if (btag != 0b110) {
      dataAvailable(MAX30001_DATA_BIOZ, max30001_BIOZ_FIFO_buffer, 8);
    }
  }

  ret_val = 0;

  if (max30001_status.bit.dcloffint == 1) // ECG/BIOZ Lead Off
  {
    dcloffint_OneShot = 1;
    max30001_DCLeadOff = 0;
    max30001_DCLeadOff = max30001_DCLeadOff | (max30001_cnfg_gen.bit.en_dcloff << 8) | (max30001_status.all & 0x00000F);
    dataAvailable(MAX30001_DATA_LEADOFF_DC, &max30001_DCLeadOff, 1);

    ret_val = 0b100;

  } else if (dcloffint_OneShot == 1 && max30001_status.bit.dcloffint == 0) // Just send once when it comes out of dc lead off
  {
    max30001_DCLeadOff = 0;
    max30001_DCLeadOff = max30001_DCLeadOff | (max30001_cnfg_gen.bit.en_dcloff << 8) | (max30001_status.all & 0x00000F);
    dataAvailable(MAX30001_DATA_LEADOFF_DC, &max30001_DCLeadOff, 1);
    dcloffint_OneShot = 0;
  }

  if (max30001_status.bit.bover == 1 || max30001_status.bit.bundr == 1) // BIOZ AC Lead Off
  {
    acloffint_OneShot = 1;
    max30001_ACLeadOff = 0;
    max30001_ACLeadOff =
        max30001_ACLeadOff | ((max30001_status.all & 0x030000) >> 16);
    dataAvailable(MAX30001_DATA_LEADOFF_AC, &max30001_ACLeadOff, 1);

    ret_val = 0b1000;
  } else if (acloffint_OneShot == 1 && max30001_status.bit.bover == 0 && max30001_status.bit.bundr == 0) // Just send once when it comes out of ac lead off
  {
    max30001_ACLeadOff = 0;
    max30001_ACLeadOff = max30001_ACLeadOff | ((max30001_status.all & 0x030000) >> 16);
    dataAvailable(MAX30001_DATA_LEADOFF_AC, &max30001_ACLeadOff, 1);
    acloffint_OneShot = 0;
  }

  if (max30001_status.bit.bcgmon == 1) // BIOZ BCGMON check
  {
    bcgmon_OneShot = 1;
    max30001_bcgmon = 0;
    max30001_bcgmon = max30001_bcgmon | ((max30001_status.all & 0x000030) >> 4);
    dataAvailable(MAX30001_DATA_BCGMON, &max30001_bcgmon, 1);

    ret_val = 0b10000;
  } else if (bcgmon_OneShot == 1 && max30001_status.bit.bcgmon == 0) {
    max30001_bcgmon = 0;
    max30001_bcgmon = max30001_bcgmon | ((max30001_status.all & 0x000030) >> 4);
    bcgmon_OneShot = 0;
    dataAvailable(MAX30001_DATA_BCGMON, &max30001_bcgmon, 1);
  }

#if 0
if(max30001_status.bit.lonint == 1)   // AC LeadON Check
{
	max30001_LeadOn = 0;
	max30001_reg_read(STATUS,&max30001_status.all);        // Reading is important
	max30001_LeadOn = max30001_LeadOn | (max30001_cnfg_gen.bit.en_ulp_lon << 8) | ((max30001_status.all & 0x000800) >> 11);  // 0b01 will mean ECG Lead On, 0b10 will mean BIOZ Lead On
    // LEAD ON has been detected... Now take actions
}
#endif

  if (max30001_status.bit.lonint == 1 &&
      acleadon_OneShot == 0) // AC LeadON Check, when lead is on
  {
    max30001_LeadOn = 0;
    max30001_reg_read(STATUS, &max30001_status.all); // Reading is important
    max30001_LeadOn =
        max30001_LeadOn | (max30001_cnfg_gen.bit.en_ulp_lon << 8) |
        ((max30001_status.all & 0x000800) >>
         11); // 0b01 will mean ECG Lead On, 0b10 will mean BIOZ Lead On

    // LEAD ON has been detected... Now take actions
    acleadon_OneShot = 1;
    dataAvailable(MAX30001_DATA_ACLEADON, &max30001_LeadOn, 1); // One shot data will be sent...
  } else if (max30001_status.bit.lonint == 0 && acleadon_OneShot == 1) {
    max30001_LeadOn = 0;
    max30001_reg_read(STATUS, &max30001_status.all);
    max30001_LeadOn =
        max30001_LeadOn | (max30001_cnfg_gen.bit.en_ulp_lon << 8) | ((max30001_status.all & 0x000800) >> 11); // 0b01 will mean ECG Lead On, 0b10 will mean BIOZ Lead On
    dataAvailable(MAX30001_DATA_ACLEADON, &max30001_LeadOn, 1); // One shot data will be sent...
    acleadon_OneShot = 0;
  }

  return ret_val;
}

//******************************************************************************
uint32_t MAX30001::readPace(int group, uint8_t* result) {
  uint8_t data_array[4];
  uint32_t success;
  int adr = PACE0_FIFO_BURST + group*4;

  if (group >= 6)
    return (uint32_t)-1;

  data_array[0] = ((adr << 1) & 0xff) | 1; // For Read Or with 1
  success = SPI_Transmit(&data_array[0], 1, &result[0], 10);

  max30001_PACE[group * 3 + 0] = (uint32_t)(result[1] << 16) + (result[2] << 8) + result[3];
  max30001_PACE[group * 3 + 1] = (uint32_t)(result[4] << 16) + (result[5] << 8) + result[6];
  max30001_PACE[group * 3 + 2] = (uint32_t)(result[7] << 16) + (result[8] << 8) + result[9];

  return success;
}

//******************************************************************************

//******************************************************************************

int MAX30001::max30001_int_handler(void) {

  static uint32_t InitReset = 0;

  int8_t return_value;
  bool check_one_more = true;

  status_check:
  max30001_reg_read(STATUS, &max30001_status.all);

  // Inital Reset and any FIFO over flow invokes a FIFO reset
  if (InitReset == 0 || max30001_status.bit.eovf == 1 || max30001_status.bit.bovf == 1) {
    // Do a FIFO Reset
    max30001_reg_write(FIFO_RST, 0x000000);

    InitReset++;
    return 2;
  }

  return_value = 0;

  // The four data handling goes on over here
  if (max30001_status.bit.eint == 1 || max30001_status.bit.pint == 1 || max30001_status.bit.bint == 1 || max30001_status.bit.rrint == 1
	|| max30001_status.bit.dcloffint == 1 || max30001_status.bit.bover == 1 || max30001_status.bit.bundr == 1
	|| max30001_status.bit.bcgmon == 1 || max30001_status.bit.lonint == 1) {
    return_value = return_value | max30001_FIFO_LeadONOff_Read();
  }
/*
  // ECG/BIOZ DC Lead Off test
  if (max30001_status.bit.dcloffint == 1) {
    return_value = return_value | max30001_FIFO_LeadONOff_Read();
  }

  // BIOZ AC Lead Off test
  if (max30001_status.bit.bover == 1 || max30001_status.bit.bundr == 1) {
    return_value = return_value | max30001_FIFO_LeadONOff_Read();
  }

  // BIOZ DRVP/N test using BCGMON.
  if (max30001_status.bit.bcgmon == 1) {
    return_value = return_value | max30001_FIFO_LeadONOff_Read();
  }

  if (max30001_status.bit.lonint == 1) // ECG Lead ON test: i.e. the leads are touching the body...
  {

    max30001_FIFO_LeadONOff_Read();
  }
*/
  if (check_one_more) {
    check_one_more = false;
    goto status_check;
  }
  return return_value;
}

/// function pointer to the async callback
static event_callback_t functionpointer;
/// flag used to indicate an async xfer has taken place
static volatile int xferFlag = 0;

/**
* @brief Callback handler for SPI async events
* @param events description of event that occurred
*/


static void spiHandler(int events) { xferFlag = 1; }

/**
* @brief Transmit and recieve QUAD SPI data
* @param tx_buf pointer to transmit byte buffer
* @param tx_size number of bytes to transmit
* @param rx_buf pointer to the recieve buffer
* @param rx_size number of bytes to recieve
*/
int MAX30001::SPI_Transmit(const uint8_t *tx_buf, uint32_t tx_size, uint8_t *rx_buf, uint32_t rx_size)
{
    m_cs->write(0);
    for(uint32_t i = 0; i < tx_size; i++)
    {
        m_spi->write(tx_buf[i]);
    }
    for(uint32_t i = 0; i < (rx_size - tx_size); i++)
    {
	rx_buf[i + 1] = m_spi->write(0xFF);
    }
    m_cs->write(1);

    return 0;
}

//******************************************************************************
void MAX30001::max30001_ReadHeartrateData(max30001_t *_hspValMax30001) {
  _hspValMax30001->R2R = hspValMax30001.R2R;
  _hspValMax30001->fmstr = hspValMax30001.fmstr;
}

//******************************************************************************
void MAX30001::onDataAvailable(PtrFunction _onDataAvailable) {
  onDataAvailableCallback = _onDataAvailable;
}

/**
* @brief Used to notify an external function that interrupt data is available
* @param id type of data available
* @param buffer 32-bit buffer that points to the data
* @param length length of 32-bit elements available
*/
void MAX30001::dataAvailable(uint32_t id, uint32_t *buffer, uint32_t length) {
  if (onDataAvailableCallback != NULL) {
    (*onDataAvailableCallback)(id, buffer, length);
  }
}
/**
* @brief Callback handler for SPI async events
* @param events description of event that occurred
*/
void MAX30001::spiHandler(int events) { xferFlag = 1; }

//******************************************************************************
static int allowInterrupts = 0;

void MAX30001Mid_IntB_Handler(void) {
	if (allowInterrupts == 0) return;
	//MAX30001::instance->max30001_int_handler();
	MAX30001::instance->m_max30001_int_happened_ = true;
}

void MAX30001Mid_Int2B_Handler(void) {
	if (allowInterrupts == 0) return;
	//MAX30001::instance->max30001_int_handler();
	MAX30001::instance->m_max30001_int_happened_ = true;
}

void MAX30001_AllowInterrupts(int state) {
	allowInterrupts = state;
}
