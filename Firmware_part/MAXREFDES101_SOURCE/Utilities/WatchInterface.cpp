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

#include "WatchInterface.h"
#include "Peripherals.h"
#include "LS013B7DH03.h"
#include "mbed_logo.h"

#include "UbuntuCondensed38x47.h"
#include "ubuntucondensed20x26.h"
#include "ubuntucondensed16x21.h"
#include "Arial10x10.h"
#include "Neu44x36.h"
#include "mxm_assert.h"
#include "utils.h"
#include "../version.h"

//Non-const static members must be both declared in the class definition and defined with global scope to properly give the linker something to reference.
bool WatchInterface::refreshed = true;


WatchInterface::WatchInterface(PinName buttonUp, PinName buttonDown, PinName buttonPower, PinName displayCS, PinName displayEXTCOM, PinName displayDISP, SPI* displaySPI):
	btnUp(buttonUp), btnDown(buttonDown), btnPower(buttonPower), dispCS(displayCS), dispEXTCOM(displayEXTCOM), dispDISP(displayDISP), display(displaySPI, &dispCS), DisplayPWM(displayEXTCOM)
{
	dispDISP = 0;


	buttonEvent_up   = BUTTONEVENT_NONE;
	buttonEvent_down = BUTTONEVENT_NONE;


	btnUp.fall(callback(this, &WatchInterface::isr_buttonup));
	btnUp.mode(PullUp);
	btnUp.enable_irq();

	btnDown.fall(callback(this, &WatchInterface::isr_buttondown));
	btnDown.mode(PullUp);
	btnDown.enable_irq();

	displaySPI->frequency(1000000);

	DisplayPWM.period_ms(16);
	DisplayPWM.pulsewidth_ms(8);

	//updateDisplay(4);

	recording_old 	  = false;
	recording    	  = false;
	recordingStopFlag = false;
	isDisplayOff      = false;

	interfaceTimer.start();

	displayMode_old = DISPLAYMODE_AFTERBOOT;
	displayMode     = DISPLAYMODE_TIME;



}

void WatchInterface::isr_buttonup()
{
	if (buttonEvent_up == BUTTONEVENT_NONE) {
		buttonEvent_up = BUTTONEVENT_UP_PRE;
	}
}

void WatchInterface::isr_buttondown()
{
	if (buttonEvent_down == BUTTONEVENT_NONE) {
		buttonEvent_down = BUTTONEVENT_DOWN_PRE;
	}
}

WatchInterface::ButtonStatus	WatchInterface::getButtonStatus(WatchInterface::hspButton button)
{

	switch (button) {

		// TODO: fill button power status
		case WatchInterface::BUTTON_POWER :
			return (BUTTON_RELEASED);
			break;
		case WatchInterface::BUTTON_UP:
			if (btnUp.read()) {
				return (WatchInterface::BUTTON_PRESSED);
			} else {
				return (WatchInterface::BUTTON_RELEASED);
			}
			break;

		case WatchInterface::BUTTON_DOWN:
			if (btnDown.read()) {
				return (WatchInterface::BUTTON_PRESSED);
			} else {
				return (WatchInterface::BUTTON_RELEASED);
			}
			break;

	}

}


void WatchInterface::refreshCallback(void)
{
	refreshed = true;
}



void WatchInterface::execute(void)
{
	unsigned int current_timer;
	static int tickCount;
	static char tickCount_battery;
	static int firstDetect_up;
	static int firstDetect_down;


	// Display is OFF in ECG Mode
	if (ecg_enabled) {
		if (recording & (buttonEvent_down == BUTTONEVENT_DOWN_PRE)) {
			dispDISP = 1;
			if (isDisplayOff) {
				displayOff(false);
			}
			displayMode = DISPLAYMODE_RECORDING_STOP;
		} else {
			displayOff(true);
		}
	} else {
		dispDISP = 1;
		if (isDisplayOff) {
			displayOff(false);
			displayMode = DISPLAYMODE_RECORDING_STOP;
		}
	}


	if (isDisplayOff != true) {
		display.executeOnceLCD();

		if (USB_Interface_MSD) {
			return;
		}

		current_timer = interfaceTimer.read_ms();

		// Button Debouncing
		if (current_timer > 50) {

			interfaceTimer.reset();
			interfaceTimer.start();

			// Increase tick counter in every 50ms
			tickCount++;

			// tick counter reset will occur in every 15x50 = 750ms. This is used for display updates
			if (tickCount == 15) {
				tickCount = 0;
			}

			if (buttonEvent_up == BUTTONEVENT_UP_PRE) {

				if (firstDetect_up < 2) {
					firstDetect_up++;
				} else {
					firstDetect_up = 0;
					if (btnUp.read() == 0) {
						buttonEvent_up = BUTTONEVENT_UP;
					} else {
						buttonEvent_up = BUTTONEVENT_NONE;
					}
				}

			} else {
				firstDetect_up = 0;
			}

			if (buttonEvent_down == BUTTONEVENT_DOWN_PRE) {

				if (firstDetect_down < 2) {
					firstDetect_down++;
				} else {

					firstDetect_down = 0;

					if (btnDown.read() == 0) {
						buttonEvent_down = BUTTONEVENT_DOWN;

					} else {
						buttonEvent_down = BUTTONEVENT_NONE;
					}
				}

			} else {
				firstDetect_down = 0;
			}

		}


		switch (displayMode) {


			case DISPLAYMODE_TIME:

				// Detect BLE Connected Event
				if (BLE_Interface_Exists) {

					BLE_Interface_Exists_old = true;
					display_BLEMode_Active   = false;
					displayMode = DISPLAYMODE_TETHERED;

					tickCount = 8;

				} else
					// Detect USB Connected Event
					if (USB_Interface_Exists) {

						USB_Interface_Exists_old = true;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TETHERED_USB;

						tickCount = 8;

					} else
						// Next menu item
						if (buttonEvent_up == BUTTONEVENT_UP) {

							buttonEvent_up = BUTTONEVENT_NONE;
							displayMode = DISPLAYMODE_PPG;
							modeUpdated = true;
							tickCount = 8;

						} else
							// Next menu item
							if (buttonEvent_down == BUTTONEVENT_DOWN) {

								buttonEvent_down = BUTTONEVENT_NONE;
								displayMode = DISPLAYMODE_INFO;
								modeUpdated = true;
								tickCount = 8;

							} else {


							}
				break;

#ifdef ENABLE_SPO2
			case DISPLAYMODE_PPG:		// Handle both PPG and ECG
			case DISPLAYMODE_ECG:

				// Detect BLE Connected Event
				if (BLE_Interface_Exists) {

					BLE_Interface_Exists_old = true;
					display_BLEMode_Active   = false;
					displayMode = DISPLAYMODE_TETHERED;

					tickCount = 8;

				} else
					// Detect USB Connected Event
					if (USB_Interface_Exists) {

						USB_Interface_Exists_old = true;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TETHERED_USB;

						tickCount = 8;

					} else
						// Next menu item
						if (buttonEvent_up == BUTTONEVENT_UP) {

							buttonEvent_up = BUTTONEVENT_NONE;
							displayMode = DISPLAYMODE_SPO2_ONESHOT;
							modeUpdated = true;
							tickCount = 8;

						} else
							// Next menu item
							if (buttonEvent_down == BUTTONEVENT_DOWN) {

								buttonEvent_down = BUTTONEVENT_NONE;
								displayMode = DISPLAYMODE_TIME;
								modeUpdated = true;
								tickCount = 8;

							}
				break;

			case DISPLAYMODE_SPO2_ONESHOT:

				// Detect BLE Connected Event
				if (BLE_Interface_Exists) {

					BLE_Interface_Exists_old = true;
					display_BLEMode_Active   = false;
					displayMode = DISPLAYMODE_TETHERED;

					tickCount = 8;

				} else
					// Detect USB Connected Event
					if (USB_Interface_Exists) {

						USB_Interface_Exists_old = true;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TETHERED_USB;

						tickCount = 8;

					} else
						// Next menu item
						if (buttonEvent_up == BUTTONEVENT_UP) {

							buttonEvent_up = BUTTONEVENT_NONE;
							displayMode = DISPLAYMODE_SPO2_CONTINUOUS;
							modeUpdated = true;
							tickCount = 8;

						} else
							// Next menu item
							if (buttonEvent_down == BUTTONEVENT_DOWN) {

								buttonEvent_down = BUTTONEVENT_NONE;
								displayMode = DISPLAYMODE_PPG;
								modeUpdated = true;
								tickCount = 8;

							}

				break;

			case DISPLAYMODE_SPO2_CONTINUOUS:

				// Detect BLE Connected Event
				if (BLE_Interface_Exists) {

					BLE_Interface_Exists_old = true;
					display_BLEMode_Active   = false;
					displayMode = DISPLAYMODE_TETHERED;

					tickCount = 8;

				} else
					// Detect USB Connected Event
					if (USB_Interface_Exists) {

						USB_Interface_Exists_old = true;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TETHERED_USB;

						tickCount = 8;

					} else
						// Next menu item
						if (buttonEvent_up == BUTTONEVENT_UP) {

							buttonEvent_up = BUTTONEVENT_NONE;
							displayMode = DISPLAYMODE_INFO;
							modeUpdated = true;
							tickCount = 8;

						} else
							// Next menu item
							if (buttonEvent_down == BUTTONEVENT_DOWN) {

								buttonEvent_down = BUTTONEVENT_NONE;
								displayMode = DISPLAYMODE_SPO2_ONESHOT;
								modeUpdated = true;
								tickCount = 8;

							}

				break;
#else
			case DISPLAYMODE_PPG:		// Handle both PPG and ECG
			case DISPLAYMODE_ECG:

				// Detect BLE Connected Event
				if (BLE_Interface_Exists) {

					BLE_Interface_Exists_old = true;
					display_BLEMode_Active   = false;
					displayMode = DISPLAYMODE_TETHERED;

					tickCount = 8;

				} else
					// Detect USB Connected Event
					if (USB_Interface_Exists) {

						USB_Interface_Exists_old = true;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TETHERED_USB;

						tickCount = 8;

					} else
						// Next menu item
						if (buttonEvent_up == BUTTONEVENT_UP) {

							buttonEvent_up = BUTTONEVENT_NONE;
							displayMode = DISPLAYMODE_TEMP;
							modeUpdated = true;
							tickCount = 8;

						} else
							// Next menu item
							if (buttonEvent_down == BUTTONEVENT_DOWN) {

								buttonEvent_down = BUTTONEVENT_NONE;
								displayMode = DISPLAYMODE_TIME;
								modeUpdated = true;
								tickCount = 8;

							}
				break;

			case DISPLAYMODE_TEMP:

				// Detect BLE Connected Event
				if (BLE_Interface_Exists) {

					BLE_Interface_Exists_old = true;
					display_BLEMode_Active   = false;
					displayMode = DISPLAYMODE_TETHERED;

					tickCount = 8;

				} else
					// Detect USB Connected Event
					if (USB_Interface_Exists) {

						USB_Interface_Exists_old = true;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TETHERED_USB;

						tickCount = 8;

					} else
						// Next menu item
						if (buttonEvent_up == BUTTONEVENT_UP) {

							buttonEvent_up = BUTTONEVENT_NONE;
							displayMode = DISPLAYMODE_INFO;
							modeUpdated = true;
							tickCount = 8;

						} else
							// Next menu item
							if (buttonEvent_down == BUTTONEVENT_DOWN) {

								buttonEvent_down = BUTTONEVENT_NONE;
								displayMode = DISPLAYMODE_PPG;
								modeUpdated = true;
								tickCount = 8;

							}
				break;
#endif
			case DISPLAYMODE_INFO:

				// Detect BLE Connected Event
				if (BLE_Interface_Exists) {

					BLE_Interface_Exists_old = true;
					display_BLEMode_Active   = false;
					displayMode = DISPLAYMODE_TETHERED;

					tickCount = 8;

				} else
					// Detect USB Connected Event
					if (USB_Interface_Exists) {

						USB_Interface_Exists_old = true;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TETHERED_USB;

						tickCount = 8;

					} else
						// Next menu item
						if (buttonEvent_up == BUTTONEVENT_UP) {

							buttonEvent_up = BUTTONEVENT_NONE;
							displayMode = DISPLAYMODE_TIME;
							modeUpdated = true;
							tickCount = 8;

						} else
							// Next menu item
							if (buttonEvent_down == BUTTONEVENT_DOWN) {

								buttonEvent_down = BUTTONEVENT_NONE;
#ifdef ENABLE_SPO2
								displayMode = DISPLAYMODE_SPO2_CONTINUOUS;
#else
	                            displayMode = DISPLAYMODE_TEMP;
#endif
								modeUpdated = true;
								tickCount = 8;

							}
				break;
			case DISPLAYMODE_TETHERED:

				// Detect Recording start event
				if ((recording)  && (!recording_old)) {

					recording_old = true;
					displayMode = DISPLAYMODE_RECORDING;

					tickCount = 8;

				} else
					// Detect BLE Disconnected Event
					if (!BLE_Interface_Exists) {

						BLE_Interface_Exists_old = false;
						display_BLEMode_Active  = false;
						displayMode = DISPLAYMODE_TIME;

						tickCount = 8;

					}

				break;

			case DISPLAYMODE_TETHERED_USB:

				// Detect Recording start event
				if ((recording)  && (!recording_old)) {

					printf("START EVENT");
					recording_old = recording;
					displayMode = DISPLAYMODE_RECORDING;

					tickCount = 8;

				} else
					// Detect USB Disconnected Event
					if (!USB_Interface_Exists) {

						USB_Interface_Exists_old = false;
						display_USBMode_Active   = false;
						displayMode = DISPLAYMODE_TIME;

						tickCount = 8;

					}


				break;

			case DISPLAYMODE_RECORDING: {
				// Detect stop recording
				if ((!recording) && (recording_old)) {
					printf("STOP EVENT");
					recording_old = recording;
					displayMode = DISPLAYMODE_TIME;
					tickCount = 8;
				} else {
					if (buttonEvent_down == BUTTONEVENT_DOWN) {
						buttonEvent_down = BUTTONEVENT_NONE;
						displayMode = DISPLAYMODE_RECORDING_STOP;
						tickCount = 8;
					}
				}
			}
			break;

			case DISPLAYMODE_RECORDING_STOP: {
				recordingStopFlag = true;
				recording         = false;
				recording_old     = false;
				displayMode = DISPLAYMODE_TIME;
			}
			break;
		}



		/*
		 if ( recording ) {

		  if ( ( buttonEvent_down == BUTTONEVENT_DOWN ) &&  ( !BLE_Interface_Exists ) &&  ( !USB_Interface_Exists ) ) {


			  buttonEvent_down = BUTTONEVENT_NONE;

			  if ( displayMode == DISPLAYMODE_RECORDING )
					displayMode = DISPLAYMODE_RECORDING_STOP;

			  tickCount = 8;
		  }
		 }
		 else {

		 // Handle Up/Down Button press Events

			  // Process DOWN Button Press Event
			  if ( ( buttonEvent_down == BUTTONEVENT_DOWN ) &&  ( !BLE_Interface_Exists ) &&  ( !USB_Interface_Exists ) ) {


				  buttonEvent_down = BUTTONEVENT_NONE;

				  if ( displayMode > 0)
						displayMode--;
				  else
						displayMode = 3;

				  tickCount = 8;
			  }
			  else
				  // Process UP Button Press Event
			  if ( ( buttonEvent_up == BUTTONEVENT_UP ) &&  ( !BLE_Interface_Exists ) &&  ( !USB_Interface_Exists ) ) {

				  if ( displayMode < 3)
						displayMode++;
				  else
						displayMode = 0;

				  buttonEvent_up = BUTTONEVENT_NONE;

				  tickCount = 8;
			  }





				// Detect BLE Connected Event
				if ( ( BLEWasActive == false )  &&  ( BLE_Interface_Exists ) ) {

					BLEWasActive = true;
					display_BLEMode_Active = false;
					displayMode = DISPLAYMODE_TETHERED;
					modeUpdated = true;

					tickCount = 8;

				}
				else
				// Detect BLE Disconnected Event
				if ( ( BLEWasActive == true )  &&  ( !BLE_Interface_Exists ) ) {

					BLEWasActive = false;
					display_BLEMode_Active = false;
					displayMode = DISPLAYMODE_TIME;
					//should_stop = true;
					modeUpdated = true;

					tickCount = 8;

				}
				else
				// Detect USB Connected Event
				if ( ( USBWasActive == false )  &&  ( USB_Interface_Exists ) ) {

					USBWasActive = true;
					display_USBMode_Active = false;
					displayMode = DISPLAYMODE_TETHERED_USB;
					modeUpdated = true;

					tickCount = 8;

				}
				else
				// Detect USB Disconnected Event
				if ( ( USBWasActive == true )  &&  ( !USB_Interface_Exists ) ) {

					USBWasActive = false;
					display_USBMode_Active = false;
					displayMode = DISPLAYMODE_TIME;
					//should_stop = true;
					modeUpdated = true;

					tickCount = 8;

				}

		 }
		*/

		// Update the display in every 500ms (if necessary)
		if (tickCount == 0) {

			tickCount = 1;

			if (displayMode_old != displayMode) {

				modeUpdated = true;
#ifdef ENABLE_SPO2
				if((displayMode == DISPLAYMODE_SPO2_ONESHOT) || (displayMode == DISPLAYMODE_SPO2_CONTINUOUS))
				{
					instant_spo2_percentage=0;
				}
#endif
			}

			//if ( (!display_BLEMode_Active) &&  (!display_USBMode_Active) ) {
			btnDown.disable_irq();
			btnUp.disable_irq(); // Disable Interrupts


			if (tickCount_battery <= 0) {
				m_max20303_->Max20303_BatteryGauge(&batteryLevel);
				tickCount_battery = BATTERY_GAUGE_COUNT;
			}
			tickCount_battery--;

			updateDisplay(batteryLevel);

			btnDown.enable_irq();
			btnUp.enable_irq(); // Enable Interrupts
			//}

			displayMode_old = displayMode;
		}

	}
}

void WatchInterface::displayOff(bool off)
{
	if (off) {
		// Display Turned Off
		DisplayPWM.pulsewidth_ms(0);
		dispDISP = 0;
		isDisplayOff = true;
	} else {
		// Display Turned on
		DisplayPWM.pulsewidth_ms(8);
		dispDISP = 1;
		isDisplayOff = false;
	}
}

void  WatchInterface::DisplayModeUpdated(){
	ppg_lcd_count = 0;
}

void WatchInterface::updateDisplay(int batteryLevelToBeScaled) {
	char buffer[32];
	time_t seconds;
	int hrDisplay_confidenceThreshold;
	static int hr_timeout;

	// Clear Entire Screen
	display.cls();
	display.fill(0, 0, 128, 128, Black);

	// Draw Maxim Integrated Logo
	display.showBMP((uint8_t*)maxim128Bitmaps, 120, 36, 8, 90);

	if (!bootComplete) {
		return;
	}

	// Show BLE logo if it is connected


	if (displayMode != DISPLAYMODE_TETHERED_USB) {
		// Draw battery outline
		display.fillrect(94,  6, 27, 12, White);
		display.fillrect(97,  9, 21, 6, Black);
		display.fillrect(122, 9, 2,  6, White);

		if (batteryLevelToBeScaled < 10) {
			batteryLevelToBeScaled = 0;
		} else if (batteryLevelToBeScaled <= 25) {
			batteryLevelToBeScaled = 1;
		} else if (batteryLevelToBeScaled <= 50) {
			batteryLevelToBeScaled = 2;
		} else if (batteryLevelToBeScaled <= 75) {
			batteryLevelToBeScaled = 3;
		} else {
			batteryLevelToBeScaled = 4;
		}

		// Draw battery level ticks
		for (int i = 0 ; i < batteryLevelToBeScaled ; i++) {
			display.fillrect(99 + (i * 5),  10, 3, 4, White);
		}
	}

	// Show BLE logo if it is connected
	if (BLE_Interface_Exists) {
		display.showBMP((uint8_t*)BLELogoBitmap, 16, 15, 72, 13);
		//displayMode = DISPLAYMODE_TETHERED;
		//BLEWasActive = true;
	} else {
		// Show USB logo if it is connected
		if (USB_Interface_Exists) {
			display.showBMP((uint8_t*)USBLogoBitmap, 16, 15, 72, 13);
			//displayMode = DISPLAYMODE_TETHERED_USB;
			//USBWasActive = true;
		}
	}

	display.set_font(UbuntuCondensed20x26);
	display.locate(3, 10);
	display.foreground((uint16_t)Black);
	display.background((uint16_t)White);

	switch (displayMode) {

		case DISPLAYMODE_RECORDING:

			display.fillrect(8,  46, 27,  23, White);
			display.fillrect(9,  45, 25,  25, White);
			display.fillrect(10, 44, 23,  27, White);

			if (ppg_lcd_count == 0) {
				display.fillellipse(21, 57, 8, 8, (uint16_t) Black);
			}

			snprintf(buffer, 32, "REC");
			display.set_font(UbuntuCondensed38x47);
			display.locate(40, 36);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf(buffer);


			ppg_lcd_count++;
			if (ppg_lcd_count > 1) {
				ppg_lcd_count = 0;
			}

			break;


		case DISPLAYMODE_RECORDING_STOP:
			display.printf("CONFIRM");
			snprintf(buffer, 32, "STOP");
			display.set_font(UbuntuCondensed38x47);
			display.locate(5, 36);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf(buffer);

			break;

		case DISPLAYMODE_INFO:
			display.printf("INFO");


			display.set_font(Arial10x10);

			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			snprintf(buffer, 32, "BLE MAC:");
			display.locate(4, 40);
			display.printf(buffer);
			snprintf(buffer, 32, "%02X:%02X:%02X:%02X:%02X:%02X", m_mac_addr_[0],  m_mac_addr_[1], m_mac_addr_[2],  m_mac_addr_[3], m_mac_addr_[4],  m_mac_addr_[5]);
			display.locate(4, 50);
			display.printf(buffer);

			snprintf(buffer, 32, "FW Version:");
			display.locate(4, 66);
			display.printf(buffer);

			snprintf(buffer, 32, "%s", FIRMWARE_VERSION);
			display.locate(4, 76);
			display.printf(buffer);

			break;

		case DISPLAYMODE_TIME:
			display.printf("TIME");
			seconds = time(NULL);
			strftime(buffer, 32, "%I:%M", localtime(&seconds));
			display.set_font(UbuntuCondensed38x47);
			display.locate(6, 40);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf(buffer);
			snprintf(buffer, 32, "MAC:%02X:%02X", m_mac_addr_[4],  m_mac_addr_[5]);
			display.set_font(Arial10x10);
			display.locate(60, 20);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf(buffer);

			while (1) {
				if (seconds >= 86400) {
					seconds = seconds % 86400;
				} else {
					break;
				}
			}

			if (seconds < 43200) {
				display.showBMP((uint8_t*)AM_Bitmap, 16, 9, 104, 45);
			} else {
				display.showBMP((uint8_t*)PM_Bitmap, 16, 9, 104, 45);
			}


			break;

		case DISPLAYMODE_PPG:
			display.printf("PPG");

			//Blink "MEASURING" for the first 13 seconds (settling time of ppg)

			display.set_font(UbuntuCondensed16x21);
			display.locate(12, 52);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);

			if(ppg_lcd_count<25)
			{
				if(ppg_lcd_count & 0x01)
				{
					display.printf("         ");
				}
				else
				{
					display.printf("MEASURING");
				}
			}

			//print the heart rate (it is settled now)
			else {
				if (ppg_lcd_count > 80) {
					ppg_lcd_count = 70;
				}

				display.set_font(UbuntuCondensed38x47);
				display.locate(8, 40);
				display.foreground((uint16_t)Black);
				display.background((uint16_t)White);

				if (instant_hr_activityClass == ACTD_REST_ACTIVITY) {
					hrDisplay_confidenceThreshold = ConfThresLevelRest;
				} else {
					hrDisplay_confidenceThreshold = ConfThresLevelOther;
				}
				if ((instant_hr_conf > hrDisplay_confidenceThreshold) && (instant_hr > kHeartRateLowerLimit) && (instant_hr < kHeartRateUpperLimit)) {
					display.printf("%d", instant_hr);
					last_good_hr = instant_hr;
					hr_timeout = 0;
				} else {
					if (hr_timeout < 200) {
						hr_timeout++;
					}
					if (hr_timeout < 12) {
						display.printf("%d", last_good_hr);
					} else {
						display.printf("--");
					}

				}

				display.set_font(UbuntuCondensed20x26);
				display.locate(72, 57);
				display.foreground((uint16_t)Black);
				display.background((uint16_t)White);
				display.printf("BPM");
			}

			ppg_lcd_count++;

			break;
#ifdef ENABLE_SPO2
		case DISPLAYMODE_SPO2_ONESHOT:

			display.printf("SPO2");
			display.set_font(Arial10x10);
			display.locate(60,23);
			display.printf("ONE-SHOT");
			display.set_font(UbuntuCondensed16x21);
			display.locate(12,52);
			display.foreground(Black);
			display.background(White);
			if (instant_spo2_percentage != 100 ) {
				if(ppg_lcd_count & 0x01) {
					display.printf("         ");
				} else {
					if(instant_lowSignalQualityFlag)
						display.printf("LOWSIGNAL");
					else
						display.printf("MEASURING");
				}
			} else {
				if(instant_lowSignalQualityFlag){
					display.printf("LOWSIGNAL");
				} else {
					display.set_font(UbuntuCondensed38x47);
					display.locate(8,40);
					display.foreground(Black);
					display.background(White);
					display.printf("%d", instant_spo2);
					display.set_font(UbuntuCondensed20x26);
					display.locate(72,57);
					display.foreground(Black);
					display.background(White);
					display.printf("%%");
				}
			}
			ppg_lcd_count++;

			break;
		case DISPLAYMODE_SPO2_CONTINUOUS:

			display.printf("SPO2");
			display.set_font(Arial10x10);
			display.locate(60,23);
			display.printf("NON-STOP");
			display.set_font(UbuntuCondensed16x21);
			display.locate(12,52);
			display.foreground(Black);
			display.background(White);
			if (instant_spo2 == 0 ) {
				if(ppg_lcd_count & 0x01) {
					display.printf("         ");
				} else {
					if(instant_lowSignalQualityFlag)
						display.printf("LOWSIGNAL");
					else
						display.printf("MEASURING");
				}
			} else {
				if(instant_lowSignalQualityFlag){
					display.printf("LOWSIGNAL");
				} else {
					display.set_font(UbuntuCondensed38x47);
					display.locate(8,40);
					display.foreground(Black);
					display.background(White);
					display.printf("%d", instant_spo2);
					display.set_font(UbuntuCondensed20x26);
					display.locate(72,57);
					display.foreground(Black);
					display.background(White);
					display.printf("%%");
				}
			}
			ppg_lcd_count++;

			break;
#endif
		case DISPLAYMODE_ECG:
			display.printf("ECG");
			display.set_font(UbuntuCondensed38x47);
			display.locate(8, 40);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf("%d", 70);

			display.set_font(UbuntuCondensed20x26);
			display.locate(72, 57);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf("BPM");

			break;
#ifndef ENABLE_SPO2
		case DISPLAYMODE_TEMP:
			display.printf("TEMP");
			display.set_font(UbuntuCondensed38x47);
			display.locate(8, 40);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf("%2.1f", instant_temp_celsius);

			display.set_font(UbuntuCondensed20x26);
			display.locate(92, 55);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf("%cC", 127);


			break;
#endif
		case DISPLAYMODE_TETHERED:
			display.printf("CONN");
			display.set_font(UbuntuCondensed38x47);
			display.locate(30, 40);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf("BLE", 24);

			display_BLEMode_Active = true;

			break;

		case DISPLAYMODE_TETHERED_USB:
			if (USB_Interface_MSD) {
				display.printf("MSD");
			} else {
				display.printf("CONN");
			}
			display.set_font(UbuntuCondensed38x47);
			display.locate(30, 40);
			display.foreground((uint16_t)Black);
			display.background((uint16_t)White);
			display.printf("USB", 24);

			display_USBMode_Active = true;

			break;

	}


	display.showBMP((uint8_t*)maxim128Bitmaps, 120, 36, 8, 90);


	if (refreshed) {
		refreshed = false;
		display.update(this->refreshCallback);
	}


}


