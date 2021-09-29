/***************************************************************************
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
****************************************************************************
*/

#include <ctype.h>
#include <string.h>
#include "DSInterface.h"

#include "../../Utilities/mxm_assert.h"
#include "../version.h"
#include "Peripherals.h"
#include "utils.h"
#include "Thread.h"
#include "BLE_ICARUS.h"



#define HR_ENABLED     1 << 0
#define SPO2_ENABLED   1 << 1
#define BPT_ENABLED    1 << 2
#define WHRM_ENABLED   1 << 3
#define WSPO2_ENABLED  1 << 4
#define AEC_ENABLED    1 << 5
#define SCD_ENABLED    1 << 6
#define AGC_ENABLED    1 << 7



typedef struct {
        uint8_t  fwVersion[3];
        uint16_t enAlgoDescBitField;
        char me11ProductID;
}fw_type_t;

const static fw_type_t HUB_FW_TABLE[] = {

		{{1,8,3},   HR_ENABLED   | SPO2_ENABLED  | AGC_ENABLED                             , 'A' },
		{{20,1,1},  WHRM_ENABLED | SCD_ENABLED   | AEC_ENABLED                             , 'B' },
		{{30,1,1},  WHRM_ENABLED | WSPO2_ENABLED | AGC_ENABLED | SCD_ENABLED | AEC_ENABLED , 'C' },
		{{40,0,4},  BPT_ENABLED  | SPO2_ENABLED  | AGC_ENABLED                             , 'D' }

};

static const char* ActivAlgoInfo[32] = {
		[0]   = "hr,",
		[1]   = "spo2,",
		[2]   = "bpt,",
		[3]   = "whrm,",
		[4]   = "wspo2,",
		[5]   = "aec,",
		[6]   = "scd,",
		[7]   = "agc,",
};


static const char *glbl_cmds[] = {
	"stop",
	"get_device_info",
	"silent_mode 0",
	"silent_mode 1",
	"pause 0",
	"pause 1",
	"enable console",
	"disable console",
    "set_cfg lcd time",
    "set_cfg flash log",
	"set_cfg stream ascii", // set streaming mode to ascii
	"set_cfg stream bin",   // set streaming mode to binary
	"set_cfg report 1",   	//set report mode to 1 (brief)
	"set_cfg report 2"		// set report mode to 2 (long)
};

typedef enum {
	stop=0,
	get_device_info,
	silent0_mode,
	silent1_mode,
	pause0_mode,
	pause1_mode,
	enable_console,
	disable_console,
    set_lcd_time,
    set_flash_log,
	enable_ascii_mode,
	enable_bin_mode,
	set_cfg_report_1,
	set_cfg_report_2,
	NUM_CMDS,
} glbl_cmd_state;


DSInterface::DSInterface(USBSerial* USB)
	:flash_thread_(osPriorityNormal, 2 * 1024)
{
	cmd_idx = 0;
	silent_mode = false;
	pause_mode = false;

	memset(sensor_list, 0, DS_MAX_NUM_SENSORCOMMS * sizeof(SensorComm*));
	num_sensors = 0;

	m_USB = USB;

	ds_console_interface_exists_ = false;
	recordingStarted = false;

	ds_queue_flash_ = &ds_queue_one_;
	ds_queue_fill_  = &ds_queue_two_;
	queue_init(ds_queue_flash_, buffer_one_, FLASH_MESSAGE_SIZE, sizeof(buffer_one_));
	queue_init(ds_queue_fill_, buffer_two_, FLASH_MESSAGE_SIZE, sizeof(buffer_two_));
	flash_thread_.start(callback(&event_queue_, &EventQueue::dispatch_forever));
}

DSInterface::~DSInterface()
{
}

void DSInterface::add_sensor_comm(SensorComm *s)
{
	mxm_assert_msg(num_sensors < DS_MAX_NUM_SENSORCOMMS, "Too many sensors added to DSInterface. Increase DS_MAX_NUM_SENSORCOMMS.");
	sensor_list[num_sensors++] = s;
}

void DSInterface::enable_console_interface()
{
	ds_console_interface_exists_ = true;
}

void DSInterface::ds_set_ble_status(bool en){
	SensorComm *p_sensor;
	ds_ble_interface_exists_ = en;

	for(int sensor_num = 0; sensor_num < num_sensors; ++sensor_num) {
		p_sensor = sensor_list[sensor_num];
		p_sensor->SensorComm_Set_Ble_Status(ds_ble_interface_exists_);
	}
}

void DSInterface::ds_set_flash_rec_status(bool en) {
	SensorComm *p_sensor;
	recordingStarted = en;
	for(int sensor_num = 0; sensor_num < num_sensors; ++sensor_num) {
		p_sensor = sensor_list[sensor_num];
		p_sensor->SensorComm_Set_Flash_Status(en);
	}
}

void DSInterface::build_command(char ch)
{
	if (!this->silent_mode) /* BUG: POTENTIAL BUG, what uart port to echo, not only console */
		m_USB->printf("%c", ch);

	if (ch == 0x00) {
		pr_err("Ignored char 0x00");
		return;
	}

	if ((ch == '\n') || (ch == '\r')) {
		if (cmd_idx < (int)CONSOLE_STR_BUF_SZ)
		cmd_str[cmd_idx++] = '\0';
		m_USB->printf("\r\n");
		parse_command();

		//Clear cmd_str
		while (cmd_idx > 0) /* BUG: POTENTIAL BUG for multiple port access */
			cmd_str[--cmd_idx] = '\0';

	} else if ((ch == 0x08 || ch == 0x7F) && cmd_idx > 0) {
		//Backspace character
		if (cmd_idx > 0)
			cmd_str[--cmd_idx] = '\0';
	} else {
		/* BUG: POTENTIAL BUG for multiple port access */
		if (cmd_idx < (int)CONSOLE_STR_BUF_SZ)
			cmd_str[cmd_idx++] = ch;
	}

}

void DSInterface::stopcommand() {
	for (int i = 0; i < num_sensors; i++) {
		sensor_list[i]->stop();
	}
	ds_set_flash_rec_status(false);

}

void DSInterface::parse_command_str(const char* cmd ) {
	strncpy(cmd_str, cmd , strlen(cmd) + 1);
	parse_command();
}

void DSInterface::write_to_flash_thread_funct() {
	//static Timer mytimer;
	//mytimer.reset();
	//mytimer.start();
	fwrite((uint8_t*)ds_queue_flash_->base,  1, ds_queue_flash_->item_size * ds_queue_flash_->num_item,  flashFileHandle);
	queue_reset(ds_queue_flash_);
	//mytimer.stop();
	//printf("%f seconds\n", mytimer.read());
}

void DSInterface::parse_command()
{
	int i;
	glbl_cmd_state cmd;
	char charbuf[512];
	char tempbuf[32];
	int data_len = 0;
	int data_len_log=0;
	int ret;
	bool parsed_cmd = true;

	printf("%s \n",cmd_str );

	//If top level command, then handle it
	for (i = 0; i < NUM_CMDS; i++) {
		if (starts_with(&cmd_str[0], glbl_cmds[i])) {
			cmd = (glbl_cmd_state)i;

			switch (cmd) {
				case (enable_ascii_mode): {
					sensor_list[0]->AsciiEn = true;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (enable_bin_mode): {
					sensor_list[0]->AsciiEn = false;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (stop): {
					for (int i = 0; i < num_sensors; i++) {
						sensor_list[i]->stop();
					}

    				ds_set_flash_rec_status(false);

					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (get_device_info): {

					data_len = snprintf(charbuf, sizeof(charbuf),
						"\r\n%s platform=%s firmware_ver=%s sensors=", cmd_str, platform_name, FIRMWARE_VERSION);

					//Add list of sensors
					for (int i = 0; i < num_sensors; i++) {
						if (sensor_list[i]->is_visible()) {
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											"%s", sensor_list[i]->get_type());
							if (i < (num_sensors - 1))
								data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len, ",");
						}
					}


					for (int i = 0; i < num_sensors; i++) {
						SensorComm *s = sensor_list[i];
						if (!s->is_visible())
							continue;

						//Add algo_ver
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" algo_ver_%s=%s", s->get_type(), s->get_algo_ver());

						//Add part name
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" part_name_%s=%s", s->get_type(), s->get_part_name());

						uint8_t part_id, part_rev;
						ret = s->get_part_info(&part_id, &part_rev);

						if (ret == 0) {
							//Add part id
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" part_id_%s=%d", s->get_type(), part_id);

							//Add rev id
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len,
											" part_rev_%s=%d", s->get_type(), part_rev);
						}
					}

					if(firmware_version){
						data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len, " hub_firm_ver=%s", firmware_version);
						if(algo_desc_strsz > 0)
							data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len, "%s", algo_desc_string);
					}

					data_len += snprintf(charbuf + data_len, sizeof(charbuf) - data_len, " err=0\r\n");

				} break;
				case (silent0_mode): {
					silent_mode = false;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (silent1_mode): {
					silent_mode = true;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (pause0_mode): {
					pause_mode = false;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (pause1_mode): {
					pause_mode = true;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (enable_console): {
					ds_console_interface_exists_ = true;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				case (disable_console): {
					ds_console_interface_exists_ = false;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
              case (set_lcd_time): {

                while(1)
                {
                    if(cmd_str[17+lcd_time_count] == '\0')
                    {
                        lcd_time_str[lcd_time_count] = '\0';
                        break;
                    }
                    lcd_time_str[lcd_time_count] = cmd_str[17+lcd_time_count];
                    lcd_time_count++;
                }

                sscanf(lcd_time_str,"%d",&lcd_time_val);

                set_time(lcd_time_val);

                lcd_time_count=0;

				data_len += snprintf(charbuf, sizeof(charbuf) - 1,
						"\r\n%s err=0\r\n", cmd_str);

                } break;
              case (set_flash_log): {



                    if(cmd_str[18] == '1' )
                    {



                    	time_t seconds;
                    	seconds = time(NULL);
        				strftime(tempbuf, 32, "/fs/log-%I-%M-%S.maximlog", localtime(&seconds));

                    	flashFileHandle = fopen(tempbuf, "w+");

                        if ( flashFileHandle != NULL ) {

                        	fprintf(flashFileHandle,"mxim");

                        	//1 byte to indicate the version of the log
                        	unsigned char vers = 0;
                        	//4 bytes to indicate the number of bytes used to write the format
                        	unsigned int NumByteBLE = 764;
                        	unsigned int NumByteNoBLE = 659;

                        	fwrite(&vers,sizeof(unsigned char),1,flashFileHandle);

                        	if(BLE::Instance().gap().getState().connected){
                				fwrite(&NumByteBLE,sizeof(unsigned int),1,flashFileHandle);
                        	}
                        	else
                        		fwrite(&NumByteNoBLE,sizeof(unsigned int),1,flashFileHandle);

                        	ds_set_flash_rec_status(true);

                        	//Write the formats
                        	data_len_log += snprintf(logbuf+data_len_log, sizeof(logbuf) - data_len_log- 1,"ppg 0 cs=1 format={smpleCnt,16},"
                        			"{grnCnt,20},{grn2Cnt,20},{accelX,14,3},{accelY,14,3},"
                        			"{accelZ,14,3},{hr,12},{hrconf,8},{r,11,1},{activity,8}\n"
                        			"ppg 1 cs=1 format={smpleCnt,8},{irCnt,20},{redCnt,20},{accelX,14,3},"
                        			"{accelY,14,3},{accelZ,14,3},{r,12,1},{wspo2conf,8},"
                        			"{spo2,11,1},{wspo2percentcomplete,8},{wspo2lowSNR,1},{wspo2motion,1},{status,8}\n"
                        			"bpt 0 cs=1 format={status,4},{irCnt,19},{hr,9},"
                        			"{prog,9},{sys_bp,9},{dia_bp,9}\n"
                        			"bpt 1 cs=1 format={status,4},{irCnt,19},{hr,9},"
                        			"{prog,9},{sys_bp,9},{dia_bp,9}\n");
                        	if(BLE::Instance().gap().getState().connected){
                        		data_len_log += snprintf(logbuf+data_len_log, sizeof(logbuf) -data_len_log- 1,"ecg 1 cs=1 format={smpleCnt,8},{rtor,14},{rtorbpm,8},"
                        				"{pTag.0,3},{eTag.0,3},{ecg.0,18},{pTag.1,3},{eTag.1,3},{ecg.1,18},"
                        				"{pTag.2,3},{eTag.2,3},{ecg.2,18},{pTag.3,3},{eTag.3,3},{ecg.3,18}\n");
                        	}else{
                        		data_len_log += snprintf(logbuf+data_len_log, sizeof(logbuf) -data_len_log- 1,"ecg 1 cs=1 format={smpleCnt,8},{pTag,3},{eTag,3},"
                        				"{ecg,18},{rtor,14},{rtorbpm,8}\n");
                        	}
                        	data_len_log += snprintf(logbuf+data_len_log, sizeof(logbuf)-data_len_log - 1,"ecg 2 cs=1 format={smplCnt,8},{rtor,15}\n");
                        	data_len_log += snprintf(logbuf+data_len_log, sizeof(logbuf) -data_len_log-1,"temp 0 cs=1 format={smpleCnt,8},{temp,16,2}\n");


                        	fprintf(flashFileHandle,logbuf,data_len_log);
                        }
                        else {
                        	ds_set_flash_rec_status(false);
                        }

        				data_len += snprintf(charbuf, sizeof(charbuf) - 1,
        						"\r\n%s err=0\r\n", cmd_str);



                    }

                    else if (cmd_str[18] == '0'){
                    	ds_set_flash_rec_status(false);
                    	handle_file_close();
                    	data_len += snprintf(charbuf, sizeof(charbuf) - 1,"\r\n%s err=0\r\n", cmd_str);
                    }
                    else{
                    	//print cmd not recognized string
                    	data_len += snprintf(charbuf, sizeof(charbuf) - 1,"\r\n%s err=-255\r\n", cmd_str);
                    }

                } break;
				case (set_cfg_report_1): {
					//m_USB->printf("\r\n MODE 1\r\n");
					algo_report_mode = 1;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
					
				} break;
				case (set_cfg_report_2): {
					//m_USB->printf("\r\n MODE 2\r\n");
					algo_report_mode = 2;
					data_len += snprintf(charbuf, sizeof(charbuf) - 1,
							"\r\n%s err=0\r\n", cmd_str);
				} break;
				default:
					parsed_cmd = false;
					break;
			}

			if (parsed_cmd) {
				m_USB->printf(charbuf);

				if (BLE::Instance().gap().getState().connected) {
					BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
				}

			}

			return;
		}
	}

	//Set the stream type to check the data streaming type later
	if(starts_with(&cmd_str[0],"read ppg 0")){
		stream_type=0x00;
	}
	else if(starts_with(&cmd_str[0],"read ppg 1")){
		stream_type=0x01;
	}
	else if(starts_with(&cmd_str[0],"read bpt 0")){
		stream_type=0x02;
	}
	else if(starts_with(&cmd_str[0],"read bpt 1")){
		stream_type=0x03;
	}
	else if(starts_with(&cmd_str[0],"read ecg 1")){
		stream_type=0x04;
	}
	else if(starts_with(&cmd_str[0],"read ecg 2")){
		stream_type=0x05;
	}
	else if(starts_with(&cmd_str[0],"read temp 0")){
		stream_type=0x06;
	}

	//Loop through each sensor in sensorList
	//If sensor.get_type() is contained in cmd_str, pass cmd_str to that sensor's parser
	for (int i = 0; i < num_sensors; i++) {
		if (strstr(&cmd_str[0], sensor_list[i]->get_type())) {
			if (sensor_list[i]->parse_command(cmd_str))
				return;

			break;
		}
	}

	//If we still haven't found a way to parse the command,
	//send it to every sensor until one handles it
	for (int i = 0; i < num_sensors; i++) {
		if (sensor_list[i]->parse_command(cmd_str))
			return;
	}
	

	//No one could handle the command, print cmd not recognized string
	data_len += snprintf(charbuf, sizeof(charbuf) - 1,
			"\r\n%s err=-255\r\n", cmd_str);
	m_USB->printf(charbuf);

	if (BLE::Instance().gap().getState().connected) {
		BLE_Icarus_AddtoQueue((uint8_t *)charbuf, (int32_t)sizeof(charbuf), data_len);
	}

}

void  DSInterface::swap_queues() {
	if(flash_thread_.get_state() != Thread::WaitingEventFlag){
		printf("overflow happened state %d\r\n", flash_thread_.get_state());
		return;
	}
	queue_t * temp_queue = ds_queue_flash_;
	ds_queue_flash_ = ds_queue_fill_;
	ds_queue_fill_ = temp_queue;
	event_queue_.call(Callback<void()>(this, &DSInterface::write_to_flash_thread_funct));
}

void DSInterface::handle_file_close() {
	//if recording is enabled do not close the file
	if(recordingStarted) {
		return;
	}
	if(flashFileHandle != NULL) {
		if(flash_thread_.get_state() != Thread::WaitingEventFlag)
			return;

		if(ds_queue_fill_->num_item){
			swap_queues();
			return;
		}
		printf("closing file\r\n");
		fclose(flashFileHandle);
		flashFileHandle = NULL;
	}
}


void DSInterface::force_file_close() {
	if(flashFileHandle != NULL) {
		while(flash_thread_.get_state() != Thread::WaitingEventFlag) {
			wait_ms(1);
		}

		if(ds_queue_fill_->num_item){
			swap_queues();
		}

		while(flash_thread_.get_state() != Thread::WaitingEventFlag) {
			wait_ms(1);
		}

		//printf("closing file\r\n");
		fclose(flashFileHandle);
		flashFileHandle = NULL;
	}
}

#define ECGSENSOR_ID 0x02					//TODO:FIX THESE
#define ECGSENSOR_DATA_REPORT_MODE 0x04		//TODO:FIX THESE
void DSInterface::data_report_execute() {
	char buffer[256];
	int data_len = 0;
	buffer[0] = '\0';
	int ret;
	bool all_sensors_disabled = true;

	for (int i = 0; i < num_sensors; i++) {
		//if ((*it)->is_enabled()) {

		if(!recordingStarted)
			data_len = sensor_list[i]->data_report_execute(buffer, sizeof(buffer));
		else
			data_len = sensor_list[i]->data_report_execute(buffer + FLASH_MESSAGE_OFFSET
			, sizeof(buffer) - FLASH_MESSAGE_OFFSET);

		if (!this->pause_mode && (data_len > 0)) {
			all_sensors_disabled = false;

			if ((flashFileHandle != NULL) && (recordingStarted)) {
				if(data_len + FLASH_MESSAGE_OFFSET != ds_queue_flash_->item_size) {
					printf("Reinit item size: %d\r\n", data_len);
					queue_update_items_size(ds_queue_flash_, (data_len+ FLASH_MESSAGE_OFFSET));
					queue_update_items_size(ds_queue_fill_, (data_len+ FLASH_MESSAGE_OFFSET));
				}
				//set stream type
				buffer[0] = stream_type;

				// enqueue data
				ret = enqueue(ds_queue_fill_, buffer);
				if(ret != 0) {
					printf("enqueue has failed %d\r\n", ds_queue_fill_->num_item);
				}
				if(queue_is_full(ds_queue_fill_)) {
					swap_queues();
				}
			}


			if (BLE::Instance().gap().getState().connected & !recordingStarted) {
				ds_console_interface_exists_ = false;
				BLE_Icarus_AddtoQueue((uint8_t *)buffer, (int32_t)sizeof(buffer), data_len);
			}

			if (ds_console_interface_exists_ & !recordingStarted){
				if(sensor_list[0]->AsciiEn){
					m_USB->printf(buffer);
				}
				else{
					m_USB->writeBlock((uint8_t*)buffer, data_len);
				}
			}
			data_len = 0;
		}
	}

	if(all_sensors_disabled)
		handle_file_close();
}

void DSInterface::set_fw_version(const char* version)
{
    if (version && *version){
        firmware_version = version;
        algo_desc_strsz = get_fw_algorithms();
    }
}

int DSInterface::get_fw_algorithms( void )
{

	int i, fwToReport;
    char *start, *end;
	uint8_t fwIdx[3];

    start = (char*) firmware_version;

    /*MYG: no need to optimize ,lib function call, just called once at start of execution*/

    fwIdx[0] = (uint8_t) strtol(start,&end, 10)  ; start = end;
    fwIdx[1] = (uint8_t) strtol(start+1,&end, 10); start = end;
    fwIdx[2] = (uint8_t) strtol(start+1,&end, 10);

    fwToReport = -1;
    for( i = 0 ; i < sizeof(HUB_FW_TABLE)/ sizeof(fw_type_t); i++ ){

    	if( HUB_FW_TABLE[i].fwVersion[0] == fwIdx[0]/* &
    		HUB_FW_TABLE[i].fwVersion[1] == fwIdx[1] &
			HUB_FW_TABLE[i].fwVersion[2] == fwIdx[2] */) {

    		   fwToReport = i;
               break;
    	    }
     }

    if(fwToReport == -1){
    	return -1;
    }

     i = ((fwToReport == -1)? 0:1) * ( /*32 bit*/ 32 - __builtin_clz(HUB_FW_TABLE[fwToReport].enAlgoDescBitField));

     strcpy(&algo_desc_string[0], " fw_algos=");
     int tail = strlen(" fw_algos=");

     int descStrSz      = sizeof(algo_desc_string);
     while(--i && tail < descStrSz) {

    	 if(  (HUB_FW_TABLE[fwToReport].enAlgoDescBitField >> i) & 0x0001) {
    		   strcpy( &algo_desc_string[tail], ActivAlgoInfo[i]);
               tail += strlen(ActivAlgoInfo[i]);
     	 }

     }
     strcpy( &algo_desc_string[tail-1]," ");

     return tail;
     //m_USB->printf("%d %s \r\n", tail , algo_desc_string);

}

void DSInterface::set_fw_platform(const char* platform)
{
    if (platform && *platform)
        platform_name = platform;
}
