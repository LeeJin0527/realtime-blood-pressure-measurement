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

#include "mbed.h"
#include "USBSerial.h"
#include "version.h"
#include "DSInterface.h"
#include "Peripherals.h"
#include "usb_regs.h"

#define GPIO_PRIOINVERSION_MASK     NVIC_SetPriority(GPIO_P0_IRQn, 5); \
	                                NVIC_SetPriority(GPIO_P1_IRQn, 5); \
	                                NVIC_SetPriority(GPIO_P2_IRQn, 5); \
									NVIC_SetPriority(GPIO_P3_IRQn, 5); \
									NVIC_SetPriority(GPIO_P4_IRQn, 5); \
									NVIC_SetPriority(GPIO_P5_IRQn, 5); \
									NVIC_SetPriority(GPIO_P6_IRQn, 5);


#if defined(MBED_MEM_TRACING_ENABLED)
#include "mbed_mem_trace.h"
#endif /* MBED_MEM_TRACING_ENABLED */

#if defined(MBED_HEAP_STATS_ENABLED)
#include "mbed_stats.h"
#endif /* MBED_HEAP_STATS_ENABLED */

#if defined(MBED_STACK_STATS_ENABLED)
#include "cmsis_os.h"
#endif /* MBED_STACK_STATS_ENABLED */





//
// BOARD
//
#include "max32630hsp.h"
DigitalIn   button(P6_5);
InterruptIn interruptIn_PowerButton(P7_6);
MAX32630HSP icarus(MAX32630HSP::VIO_1V8, &interruptIn_PowerButton);

//
// LED
//
#include "LEDStatus.h"
LEDStatus ledStatus(LED1, LED_ON, LED2, LED_ON, LED3, LED_OFF);

// Virtual serial port over USB
USBSerial microUSB(0x1f00, 0x2012, 0x0001, false);
#define IS_USB_HOST_CONNECTED()  ((uint8_t)((MXC_USB->dev_intfl & MXC_F_USB_DEV_INTFL_VBUS_ST) >> MXC_F_USB_DEV_INTFL_VBUS_ST_POS))

//
// DS INTERFACE. COMMAND IN DATA OUT POINT FOR HOST. ROOF INTERFACE TO ALL SUB MODULES
//
DSInterface dsInterface(&microUSB);

//
// BLE
//
#include "BLE.h"
#include "BLE_ICARUS.h"
#ifdef ENABLE_BLETEST_COMM
#include "BleTestComm.h"
BleTestComm bleTestComm(&microUSB);
#endif




//
// DISPLAY INTERFACE.
//
#include "LS013B7DH03.h"
#include "mbed_logo.h"
SPI displaySPI(P0_5, P0_6, P0_4, NC);

#include "WatchInterface.h"
#define	PIN_BUTTON_UP		P2_3
#define PIN_BUTTON_DOWN		P6_5
#define	PIN_BUTTON_PWR		P7_6
#define	PIN_displayCS		P0_7
#define	PIN_displayEXTCOM	P6_4
#define	PIN_displayDISP		P6_6
WatchInterface watchInterface(PIN_BUTTON_UP, PIN_BUTTON_DOWN, PIN_BUTTON_PWR, PIN_displayCS, PIN_displayEXTCOM, PIN_displayDISP, &displaySPI);


//
// HOST ACCELEROMETER.
//
#include "bmi160.h"
//#include "C:\Users\mehmet.gok\Desktop\DEV\BPTDEMO\wearables_MBED\Drivers\BMI160\bmi160.h"
InterruptIn bmi160_int_pin(P3_6);
I2C I2CM2(P5_7, P6_0); /* SDA, SCL */
BMI160_I2C bmi160_dev(&I2CM2, BMI160_I2C::I2C_ADRS_SDO_LO, &bmi160_int_pin);


//
// ECG SENSOR
//

#include "EcgComm.h"
EcgComm ecgCommHandler(&microUSB);

#include "MAX30001.h"
#include "MAX30001_Helper.h"
SPI max30001_spi(P5_1, P5_2, P5_0);
DigitalOut max30001_cs(P5_3);
MAX30001 max30001(&max30001_spi, &max30001_cs);
InterruptIn max30001_InterruptB(P5_5);
InterruptIn max30001_Interrupt2B(P6_2);


//
// TEMP SENSOR
//
#include "TempComm.h"
TempComm tpCommHandler(&microUSB);
#include "MAX30205.h"
I2C i2c(P3_4, P3_5);
MAX30205 max30205(&i2c, (0x90 >> 1));
//enable the LDO for temp sensor
DigitalOut max30205_LDO_EN(P7_1, 1);


//
// ME11 SMART SENSOR INTERFACE
//
#include "SSInterface.h"
I2C ssI2C(P3_4, P3_5);
PinName ss_mfio(P5_4);
PinName ss_reset(P5_6);
SSInterface ssInterface(ssI2C, ss_mfio, ss_reset);

#include "SSBootloaderComm.h"
SSBootloaderComm ssBoot(&microUSB, &ssInterface, &dsInterface);

#include "SSMAX8614XComm.h"
SSMAX8614XComm ssMAX8614X(&microUSB, &ssInterface, &dsInterface);

#include "SSGenericCmd.h"
SSGenericCmd ssGenericCmd(&microUSB, &ssInterface, &dsInterface);


//
// MX25U Flash Driver Definition
//
#define MHZ_VALUE 1000000
#define SPI_MHZ   16
#define SPI_FREQUENCY (SPI_MHZ * MHZ_VALUE)
#include "SPIFBlockDevice.h"
#include "FATFileSystem.h"

DigitalOut flash_nHOLD(P1_5);
SPIFBlockDevice spif(P1_1, P1_2, P1_0, P1_3, SPI_FREQUENCY);
FATFileSystem filesystem("fs");
// USB MSD
#include "USBMSD_BD.h"



//
// Hardware serial port over DAPLink
//
Serial daplink(USBTX, USBRX, 115200);



static void setup_ble(void);
static void process_ble(void);
static void print_build_version(void);
static void HSP_run_in_usbmcd_mode(void);
static void HSP_mount_filesystem(void);

#ifdef ENABLE_MEMORY_DEBUG
void print_memory_info();
#endif


int global_test = 1;

int main()
{
	
	wait_ms(100);

	HSP_mount_filesystem();

	if (watchInterface.getButtonStatus(WatchInterface::BUTTON_UP) == WatchInterface::BUTTON_RELEASED) {
		HSP_run_in_usbmcd_mode();
	}
	wait_ms(500);


	GPIO_PRIOINVERSION_MASK;

	// used by the MAX30001
	NVIC_SetPriority(SPIM2_IRQn, 0);


	watchInterface.bootComplete = true;
	print_build_version();
	daplink.printf("daplink serial port\r\n");
	microUSB.printf("micro USB serial port\r\n");

	//dsInterface.set_fw_version(FIRMWARE_VERSION);
	//dsInterface.set_fw_platform(MAXIM_PLATFORM_NAME);
	dsInterface.set_fw_platform("MAX32630FTHR");
	Peripherals::setUSBSerial(&microUSB);

	icarus.max20303.Max20303_IsBattery_Connected();
	watchInterface.m_max20303_ = &icarus.max20303;


	setup_ble();
#ifdef ENABLE_BLETEST_COMM
	dsInterface.add_sensor_comm(&bleTestComm);
#endif

	//
	// MAX30001
	//

	printf("Init MAX30001 callbacks, interrupts...\r\n");
	MAX30001_Helper m_max30001helper(&max30001, &max30001_InterruptB, &max30001_Interrupt2B);
	Peripherals::setMAX30001(&max30001);
	Peripherals::setMAX30001Helper(&m_max30001helper);
	ecgCommHandler.comm_init(&m_max30001helper);
	dsInterface.add_sensor_comm(&ecgCommHandler);

	//
	//MAX30205
	//
	tpCommHandler.comm_init(&max30205);
	dsInterface.add_sensor_comm(&tpCommHandler);

	//Configure mfio as a level based interrupt (no mbed API for this, must use Maxim-specific code)
	//gpio_cfg_t mfio_gpio_cfg = {PORT_5, PIN_4, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP};
	//GPIO_IntConfig(&mfio_gpio_cfg, GPIO_INT_LOW_LEVEL);
	ssI2C.frequency(400000);
	dsInterface.set_fw_platform(ssInterface.get_ss_platform_name());
	dsInterface.set_fw_version(ssInterface.get_ss_fw_version());

	//
	//REGISTER BOOTLOADER API TO SS INTERFACE
	//
    dsInterface.add_sensor_comm(&ssBoot);

	//
	//REGISTER 8614X PPG SENSOR API TO SS INTERFACE
	//
	dsInterface.add_sensor_comm(&ssMAX8614X);
    ssMAX8614X.setBMI160(&bmi160_dev);

	//
	//REGISTER GENERIC COMMAND API TO SS INTERFACE
	//
    dsInterface.add_sensor_comm(&ssGenericCmd);


    //Blink green if SmartSensor is present, yellow otherwise
	
	SS_STATUS status = ssInterface.ss_comm_check();
	if (status == SS_SUCCESS)
		//ledStatus.set_state(LED_OFF, LED_ON, LED_OFF);
		ledStatus.set_state(LED_ON, LED_OFF, LED_OFF);
		//바꿔봄, red is present
	else
		ledStatus.set_state(LED_ON, LED_ON, LED_OFF);
	//ledStatus.blink(100, 1900);
    //ledStatus.blink(100, 1900);
	ledStatus.blink(500, 500);
	
	//
	//MAIN CONTEXT LOOP
	//
	while (1) {

		USBSerial* serial = &microUSB;
		uint8_t ch;
		while (serial->readable()) {
			ch = serial->_getc();
			dsInterface.enable_console_interface();
			dsInterface.build_command(ch);
		}


		if(dsInterface.recordingStarted) {
			if((icarus.status_powerButton == MAX32630HSP::BUTTONSTATUS_LONG_PRESS_WAITING) ||
			   (watchInterface.batteryLevel <= BATTERY_CRITICAL_LEVEL)) {
				dsInterface.stopcommand();
				dsInterface.force_file_close();
			}
		}

		icarus.Max32630HSP_CheckInterrupt_Status();
		//
        // DSINTERFACE CONTEXT as all other interfaces is run by call to its data_report_execute function!!
		//
		dsInterface.data_report_execute();

		if (ecgCommHandler.is_enabled() != watchInterface.ecg_enabled) {

			if (ecgCommHandler.is_enabled()) {

				if (!dsInterface.recordingStarted) {
					watchInterface.displayMode = DISPLAYMODE_ECG;
				}

    			// Turn off LEDs
				ledStatus.set_state(1, 1, 1);
				ledStatus.solid();
				ledStatus.set_state(1, 1, 1);
			} else {

				if (!dsInterface.recordingStarted) {
					watchInterface.displayMode = DISPLAYMODE_TIME;
				}

			}
			watchInterface.ecg_enabled = ecgCommHandler.is_enabled();

		}


		// Sensor Interface Updates on Watch display mode changes
		if (watchInterface.modeUpdated) {

			watchInterface.modeUpdated = false;
			watchInterface.DisplayModeUpdated();
			//여기서 ppg_lcd_count를 0으로 초기화하는데...
			//why..?
			//ppg_lcd_count는 Watchinterface.cpp에서만 관리하기때문?

			// Tethered mode
			if ((watchInterface.BLE_Interface_Exists) || (watchInterface.USB_Interface_Exists)) {


				// Stop all sensors
				if(!dsInterface.recordingStarted){
					dsInterface.stopcommand();
				}


			} else
				// Not in tethered mode
			{

				switch (watchInterface.displayMode) {

				    case DISPLAYMODE_INFO :

						// Before switching to INFO screen, stop all sensors
						if (!dsInterface.recordingStarted) {
							dsInterface.stopcommand();
						}

						break;
					case DISPLAYMODE_TIME :

						// Before switching to TIME screen, stop all sensors
						if (!dsInterface.recordingStarted) {
							dsInterface.stopcommand();
						}

						break;
						
					case DISPLAYMODE_PPG :

						// Before switching to PPG screen, stop all sensors
						if (!dsInterface.recordingStarted) {
							dsInterface.stopcommand();
							dsInterface.parse_command_str("set_reg ppg 2a 10");
							dsInterface.parse_command_str("set_reg ppg 23 ff");
							dsInterface.parse_command_str("read ppg 0");
						}

						break;
						

					case DISPLAYMODE_TEMP :

						// Before switching to TEMP screen, stop all sensors
						if (!dsInterface.recordingStarted)  {
							dsInterface.stopcommand();
							dsInterface.parse_command_str("set_cfg temp sr 500");
							dsInterface.parse_command_str("read temp 0");
						}

						break;

				}
			}
		}

		// Update the watch interface with the latest data
		watchInterface.instant_temp_celsius	= tpCommHandler.TempComm_instant_temp_celsius;
		watchInterface.instant_hr           = ssMAX8614X.instant_hr;
		watchInterface.instant_hr_conf      = ssMAX8614X.instant_hr_conf;

		watchInterface.BLE_Interface_Exists = BLE::Instance().gap().getState().connected;
		watchInterface.USB_Interface_Exists = IS_USB_HOST_CONNECTED();

		if (watchInterface.recordingStopFlag) {

			watchInterface.recordingStopFlag = false;
			watchInterface.recording         = false;
			watchInterface.recording_old     = false;
			dsInterface.stopcommand();
			ledStatus.set_state(LED_OFF, LED_ON, LED_OFF);
			ledStatus.blink(100, 1900);
		} else {
			watchInterface.recording            = dsInterface.recordingStarted;
		}


		if (watchInterface.BLE_Interface_Exists || watchInterface.USB_Interface_Exists) {
			watchInterface.connection_indicator++;
			if (watchInterface.connection_indicator == 1) {

				if (!dsInterface.recordingStarted) {
					dsInterface.stopcommand();
				}

			} else if (watchInterface.connection_indicator > 50) {
				watchInterface.connection_indicator = 2;
			}

		}

		watchInterface.execute();
		process_ble();
		ledStatus.update();

#ifdef ENABLE_MEMORY_DEBUG
        print_memory_info();
#endif
	}
}




static void setup_ble(void)
{

	//Set up BLE communication
	BLE& ble = BLE::Instance();
	ble.init(bleInitComplete);
	while (BLE::Instance().hasInitialized() == false) { /* spin loop */ }
	BLE_Icarus_SetDSInterface(&dsInterface);

	char addr[6];
	BLE_ICARUS_Get_Mac_Address(addr);
	printf("BLE MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\r\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);


	memcpy(watchInterface.m_mac_addr_, addr, 6);



}

static void process_ble(void)
{	
int tmp;

	if (BLE::Instance().gap().getState().connected) {
		tmp = BLE_Icarus_TransferDataFromQueue();
		if(global_test == 1){
			global_test =0;
		printf("tmp = %d\r\n", tmp);
		printf("BLE가 연결되었습니다.\r\n");
		}
		
		//if(global_test == 1)ledStatus.set_state(LED_OFF, LED_ON, LED_OFF);
		//if(global_test == 0)ledStatus.set_state(LED_OFF, LED_OFF, LED_ON);
		//시리얼 포트로 디버깅이 안될때, led로 디버깅 했던 흔적입니다.
		//추후에 혹시 시리얼 포트가 안될경우를 대비해서 주석처리 해놨습니다.
		
		
	}
	else{
		if(global_test == 1)ledStatus.set_state(LED_ON, LED_OFF, LED_OFF);

	}
	
	BLE::Instance().waitForEvent();	
	//여기서 블루투스 연결및 해제관련된게 일어남

	//BLE::Instance().processEvents();
	//BLE::Instance().onEventsToProcess(BLE::);


}

static void print_build_version(void)
{
	printf("\n\nICARUS mBED EVKit\r\n");
	printf("Fw version: %s, mbed version: %d\r\n", FIRMWARE_VERSION, MBED_VERSION);
	printf("Build source: (%s)  %s\r\n", BUILD_SOURCE_BRANCH, BUILD_SOURCE_HASH);
	printf("Build time: %s  %s\r\n\n", __TIME__, __DATE__);
}

static void HSP_run_in_usbmcd_mode(void){

	int err;

	const int writecyclecount = 256;
	watchInterface.USB_Interface_Exists = false;
	watchInterface.USB_Interface_MSD   = true;
	watchInterface.bootComplete        = true;
	watchInterface.displayMode = DISPLAYMODE_TETHERED_USB;
	watchInterface.updateDisplay(4);

	for(int i = 0; i < writecyclecount; ++i) {
		watchInterface.execute();
		wait_ms(1);
	}

	USBMSD_BD msd(&spif);
	printf("Starting MSD... ");
	msd.disk_initialize();
	err = msd.connect();
	ledStatus.set_state(1, 1, 0);
	printf("%s\n", (err < 0 ? "Fail :(" : "OK"));

	msd.disk_initialize();
	err = msd.connect();

	GPIO_PRIOINVERSION_MASK;

	while (1) {

		icarus.Max32630HSP_CheckInterrupt_Status();

		wait_ms(1);
	}
}

static void HSP_mount_filesystem(void){

	// For ISSI part, in 3-wire SPI mode, HOLD pin should be tied to high to un-pause communication
	flash_nHOLD = 1;
	// Try to mount the filesystem
	printf("Mounting the filesystem... ");
	fflush(stdout);
	int err = filesystem.mount(&spif);
	printf("%s\n", (err ? "Fail :(" : "OK"));
	if (err) {
		// Reformat if we can't mount the filesystem
		// this should only happen on the first boot
		printf("No filesystem found, formatting... ");
		fflush(stdout);
		err = filesystem.reformat(&spif);
		printf("%s\n", (err ? "Fail :(" : "OK"));
	}

}



#ifdef ENABLE_MEMORY_DEBUG
void print_memory_info() {
    static int threadStackSize[8] = {0};
    static int heapSize = 0;
    // allocate enough room for every thread's stack statistics
    int cnt = osThreadGetCount();
    mbed_stats_stack_t *stats = (mbed_stats_stack_t*) malloc(cnt * sizeof(mbed_stats_stack_t));

    cnt = mbed_stats_stack_get_each(stats, cnt);
    for (int i = 0; i < cnt; i++) {
        if(threadStackSize[i] < stats[i].max_size){
            printf("Thread: 0x%lX, Stack size: %lu / %lu\r\n", stats[i].thread_id, stats[i].max_size, stats[i].reserved_size);
            threadStackSize[i] = stats[i].max_size;
        }
    }
    free(stats);

    // Grab the heap statistics
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);
    if(heapSize < heap_stats.current_size){
        printf("Heap size: %lu / %lu bytes\r\n", heap_stats.current_size, heap_stats.reserved_size);
        heapSize = heap_stats.current_size;
    }
}
#endif
