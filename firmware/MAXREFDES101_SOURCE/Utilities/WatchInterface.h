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

#define BATTERY_GAUGE_COUNT 50
#define BATTERY_CRITICAL_LEVEL 5




#ifndef UTILITIES_WATCHINTERFACE_H_
#define UTILITIES_WATCHINTERFACE_H_
#include "mbed.h"
#include "LS013B7DH03.h"
#include "version.h"
#include "BLE_ICARUS.h"

#include "MAX20303.h"


    #define DISPLAYMODE_TIME	 		    0
    #define DISPLAYMODE_PPG		 		    1
#ifdef ENABLE_SPO2
    #define DISPLAYMODE_SPO2_ONESHOT	    2
    #define DISPLAYMODE_SPO2_CONTINUOUS	    3
#else
	#define DISPLAYMODE_TEMP                4
#endif
    #define DISPLAYMODE_INFO	 		    5
    #define DISPLAYMODE_ECG		 		    6
    #define	DISPLAYMODE_TETHERED 		    7
    #define DISPLAYMODE_TETHERED_USB 	    8
    #define DISPLAYMODE_LOGO	 		    9
    #define DISPLAYMODE_RECORDING		    10
    #define DISPLAYMODE_RECORDING_STOP	    11
    #define DISPLAYMODE_AFTERBOOT		    12


typedef enum {

	BUTTONEVENT_NONE,
	BUTTONEVENT_UP_PRE,
	BUTTONEVENT_DOWN_PRE,
	BUTTONEVENT_UP,
	BUTTONEVENT_DOWN

} Button_Event;

class WatchInterface
{
public:


	typedef enum {
		/** No activity, or very light activity. */
		ACTD_REST_ACTIVITY = 0,
		/** Non-rhythmic activities that cannot be classified in the other categories. */
		ACTD_OTHER_ACTIVITY = 1,
		/** Walking activity. */
		ACTD_WALK_ACTIVITY = 2,
		/** Running activity. */
		ACTD_RUN_ACTIVITY = 3,
		/** Biking activity. */
		ACTD_BIKE_ACTIVITY = 4,
		/** Rhythmic activities that cannot be classified in the other categories. */
		ACTD_OTHER_RYTHMIC_ACTIVITY = 5,
	} actd_ActivityClass_te;


	typedef enum {
		BUTTON_PRESSED  = 1,
		BUTTON_RELEASED = 0

	} ButtonStatus;


	typedef enum {
		BUTTON_POWER = 0,
		BUTTON_UP    = 1,
		BUTTON_DOWN  = 2

	} hspButton;


	/** Create an Watch Interface controller
	 *
	 *  @param buttonUp    PinName UP button
	 *  @param buttonDown  PinName DOWN button
	 *  @param buttonPower PinName POWER button
	 */
	WatchInterface(PinName buttonUp, PinName buttonDown, PinName buttonPower, PinName displayCS, PinName displayEXTCOM, PinName displayDISP, SPI* displaySPI);

	void updateDisplay(int batteryLevelToBeScaled);
	void execute(void);
	void displayOff(bool off);
	void DisplayStatus();
	void DisplayModeUpdated();

	static void refreshCallback(void);

	unsigned char connection_indicator;

	ButtonStatus			getButtonStatus(hspButton button);

	uint16_t 	 			heartRate;
	uint16_t 	 			heartRateConfidence;
	volatile bool 			BLE_Interface_Exists, BLE_Interface_Exists_old;
	volatile bool			USB_Interface_Exists, USB_Interface_Exists_old;
	volatile bool			USB_Interface_MSD;
	volatile bool			recording, recording_old;

	bool					recordingStopFlag;

	volatile bool 			modeUpdated;

	unsigned char			batteryLevel;

	volatile Button_Event 	buttonEvent_down;
	volatile Button_Event 	buttonEvent_up;

	unsigned char 			displayMode;
	unsigned char 			displayMode_old;
	bool					isDisplayOff;

	volatile bool			BLEWasActive;
	volatile bool			USBWasActive;

	bool					ecg_enabled;
	bool					ppg_enabled;

	bool					bootComplete;

	uint16_t				last_good_hr;
	uint16_t 				instant_hr;
	uint16_t 				instant_hr_conf;
	uint8_t 				instant_hr_activityClass;
	float					instant_temp_celsius;

	char 					m_mac_addr_[6];

	MAX20303*				m_max20303_;



protected:

private:



	//Ticker 		m_watchInterface_ticker_;

	InterruptIn btnUp;
	InterruptIn btnDown;
	DigitalIn   btnPower;
	DigitalOut 	dispCS;
	PinName		dispEXTCOM;
	DigitalOut  dispDISP;

	PwmOut		DisplayPWM;

	Timer		interfaceTimer;

	volatile int	buttonTimer;

	void		isr_buttonup();
	void		isr_buttondown();

	bool		display_BLEMode_Active;
	bool		display_USBMode_Active;

	silabs::LS013B7DH03 display;

	unsigned char ppg_lcd_count; //a variable used to print "MEASURING" while the ppg settling

	static  bool 			refreshed;

	static const uint16_t kHeartRateUpperLimit = 240;
	static const uint16_t kHeartRateLowerLimit = 40;
	static const uint16_t ConfThresLevelRest = 50;
	static const uint16_t ConfThresLevelOther = 10;


};


#endif
