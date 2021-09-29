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

#ifndef _DSINTERFACE_H_
#define _DSINTERFACE_H_

#include "mbed.h"
#include <stdint.h>
#include "SensorComm.h"
#include "MaximSensor.h"
#include "USBSerial.h"
#include "../../Utilities/queue.h"
#include "FATFileSystem.h"


#define COMM_SUCCESS        0
#define COMM_GENERAL_ERROR  -1
#define COMM_INVALID_PARAM  -254
#define COMM_NOT_RECOGNIZED -255

#define FLASH_ERR_GENERAL   -1
#define FLASH_ERR_CHECKSUM  -2
#define FLASH_ERR_AUTH      -3

#define DS_MAX_NUM_SENSORCOMMS	8

#define DS_BINARY_PACKET_START_BYTE	0xAA

#define FLASH_MESSAGE_SIZE 	   32
#define CIRCULAR_QUEUE_COUNT  256
#define FLASH_MESSAGE_OFFSET    1


/**
 * @brief   DSInterface is Maxim's DeviceStudio Interface class
 */
class DSInterface
{
public:

    /* PUBLIC FUNCTION DECLARATIONS */
    /**
    * @brief    DSInterface constructor.
    *
    * @param[in]    fw_version Firmware version number.
    * @param[in]    fw_platform Firmware platform name.
	* @param[in]	USB	Pointer to USBSerial device
    */
    DSInterface(USBSerial* USB);

	/**
    * @brief    DSInterface destructor.
    *
    */
    virtual ~DSInterface();

    /**
    * @brief    Add SensorComm for DSInterface to pass commands to
    *
    * @param[in]    s SensorComm instance to handle sensor commands.
    */
    void add_sensor_comm(SensorComm *s);

    /**
    * @brief    Command builder from i/o device.
    * @details  Reads character and builds command from DeviceStudio commands.
    *
    * @param[in]    ch Input character from i/o device.
    */
    void build_command(char ch);

	/**
	* @brief 	Prints latest sensor data to USB stream
	* @details	Should be called on every pass thorugh the main loop in main.cpp
	*
	*/
	void data_report_execute();

	void handle_file_close();

	void force_file_close();

	void swap_queues();

    /**
    * @brief    Set the fw version which DSInterface will replay with for "get_device_info" command
    *
    * @param[in]    fw_version Firmware version number.
    */
    void set_fw_version(const char *fw_version);

    /**
     * @brief    get the algorithms define within sesnor hub fw version
     *
     * @param[out]    algorihm description string
     */
    int get_fw_algorithms( void );
    /**
    * @brief    Set the fw platform which DSInterface will replay with for "get_device_info" command
    *
    * @param[in]    fw_platform Firmware platform name.
    */
    void set_fw_platform(const char *fw_platform);

	/**
	 * @brief	Enable data output on the console interface (USB)
	 */
	void enable_console_interface();

	void ds_set_ble_status(bool en);

	void ds_set_flash_rec_status(bool en);

	void stopcommand();
    void parse_command_str(const char* cmd );
	uint8_t  algo_report_mode = 1; // sets the mode for report 1/2


 //   FATFileSystem *logfilesystem;
    FILE *flashFileHandle;


    bool recordingStarted;


protected:

    /* PROTECTED FUNCTION DECLARATIONS */
    /**
    * @brief    Parses DeviceStudio commands.
    * @details  Parses and executes commands. Prints return code to i/o device.
    */
    void parse_command();

    void write_to_flash_thread_funct();


    /* PROTECTED VARIABLES */
	USBSerial *m_USB;
    Mutex comm_mutex;

	int num_sensors;
	SensorComm* sensor_list[DS_MAX_NUM_SENSORCOMMS];

	volatile uint8_t ds_console_interface_exists_;
	volatile uint8_t ds_ble_interface_exists_;

    const char* platform_name;
    const char* firmware_version;

    char algo_desc_string[128];
    int algo_desc_strsz;

    static const uint32_t CONSOLE_STR_BUF_SZ = 1024;

    char cmd_str[CONSOLE_STR_BUF_SZ];
    int cmd_idx;
    bool silent_mode;
    bool pause_mode;
    char lcd_time_str[30];
    int lcd_time_count=0;
    int lcd_time_val;

    //buffer to hold string that will be written into the log file
    char logbuf[1024];

    //Flag to check which type of data streaming will occur
    char stream_type;

    // flash logging variables
    queue_t ds_queue_one_;
    queue_t ds_queue_two_;

    queue_t *ds_queue_flash_;
    queue_t *ds_queue_fill_;

    unsigned char buffer_one_[CIRCULAR_QUEUE_COUNT * FLASH_MESSAGE_SIZE];
    unsigned char buffer_two_[CIRCULAR_QUEUE_COUNT * FLASH_MESSAGE_SIZE];

    EventQueue event_queue_;
    Thread flash_thread_;
};

#endif /* _DSINTERFACE_H_ */
