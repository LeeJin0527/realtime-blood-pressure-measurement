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

#ifndef _ECGCOMM_DEFINES_H
#define _ECGCOMM_DEFINES_H




/// Initialization values for ECG_InitStart()
#define EN_ECG	 	  0b1
#define OPENP	 	  0b0
#define OPENN	 	  0b0
#define POL			  0b0
#define CALP_SEL     0b00
#define CALN_SEL     0b00
#define E_FIT	       15
#define RATE	        2
#define GAIN	     0b00
#define DHPF	     0b01
#define DLPF	     0b01

// Default Initialization Parameters for RtoR_InitStart
#define EN_RTOR     0x01
#define WNDW		0x03
#define GAIN_RTOR	0x0F
#define PAVG		0x02
#define PTSF		0x03
#define HOFF		0x20
#define RAVG		0x02
#define RHSF		0x04
#define CLR_RRINT	0x01

// Optimal Initialization Parameters for RtoR_InitStart
#define EN_RTOR_OP     0x01
#define WNDW_OP		0x02
#define GAIN_RTOR_OP	0x06
#define PAVG_OP		0x01
#define PTSF_OP		0x06
#define HOFF_OP		0x1E
#define RAVG_OP		0x02
#define RHSF_OP		0x00
#define CLR_RRINT_OP	0x01


// Default Initialization Parameters for INT_assignment
#define EN_ENINT_LOC		0x01
#define EN_EOVF_LOC			0x00
#define EN_FSTINT_LOC		0x00
#define EN_DCLOFFINT_LOC	0x02
#define EN_BINT_LOC			0x01
#define EN_BOVF_LOC			0x00
#define EN_BOVER_LOC		0x02
#define EN_BUNDR_LOC		0x02
#define EN_BCGMON_LOC		0x00
#define EN_PINT_LOC			0x00
#define EN_POVF_LOC			0x00
#define EN_PEDGE_LOC		0x00
#define EN_LONINT_LOC		0x00
#define EN_RRINT_LOC		0x02
#define EN_SAMP_LOC			0x00
#define INTB_TYPE			0x03
#define INT2B_TYPE			0x03




#endif
