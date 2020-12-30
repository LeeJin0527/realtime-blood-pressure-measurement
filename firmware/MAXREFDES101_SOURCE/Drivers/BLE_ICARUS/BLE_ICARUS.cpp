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

#include "BLE_ICARUS.h"

#include "../../Utilities/mxm_assert.h"
#include "queue.h"
#include "Peripherals.h"
#include "../../version.h"


UUID customServiceUUID("00001523-1212-efde-1523-785feabcd123");
UUID notifyCharUUID(  "00001011-1212-efde-1523-785feabcd123");
//UUID configRWCharUUID("00001027-1212-efde-1523-785feabcd123");
UUID configRWCharUUID("00007777-1212-efde-1523-785feabcd123");

const static char     DEVICE_NAME[]        = FIRMWARE_VERSION;
static const uint16_t uuid16_list[]        = {0xFFFF}; //Custom UUID, FFFF is reserved for development

// BLE defines
#define BLE_TICKER_PERIOD   0.050			//Ticker period in order of seconds
#define BLE_CONN_INT_PACKET 2			    //Ticker period in order of seconds
#define BLE_NOTIFY_CHAR_ARR_SIZE    20
#define BLE_READWRITE_CHAR_ARR_SIZE 16
#define MAX_BLE_QUEUE 128
// end of BLE defines

#if defined(USE_BLE_TICKER_TO_CHECK_TRANSFER)
Ticker TICKER_BLE;
static volatile unsigned char BLE_CAN_TRANSFER = 0;

static void Ble_Can_Transfer_Toggle(){
	BLE_CAN_TRANSFER = true;
}

static inline char Ble_Can_Transfer_Check(){
	return BLE_CAN_TRANSFER;
}

static inline void Ble_Can_Transfer_Set(unsigned char en){
	BLE_CAN_TRANSFER = en;
}

#endif

/* Set Up custom Characteristics */
static uint8_t notifyValue[BLE_NOTIFY_CHAR_ARR_SIZE] = {0};
GattCharacteristic notifyChar(notifyCharUUID, notifyValue, BLE_NOTIFY_CHAR_ARR_SIZE, BLE_NOTIFY_CHAR_ARR_SIZE, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

static uint8_t configValue[BLE_READWRITE_CHAR_ARR_SIZE] = {3,0,254,37};
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(configValue)> writeChar(configRWCharUUID, configValue);

/* Set up custom service */
GattCharacteristic *characteristics[] = {&writeChar,&notifyChar};
GattService        customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

// Temporary Fixes to be removed
volatile BLE_State bleState = BLE_STARTING;

// end of Temporary Fixes to be removed

// sc...
struct queue_t BLEQUEUE;
static uint8_t BLEOutBuffer[BLE_NOTIFY_CHAR_ARR_SIZE * MAX_BLE_QUEUE];

static DSInterface *BLE_DS_INTERFACE;



/*
 *  Handle writes to writeCharacteristic
 */
void writeCharCallback(const GattWriteCallbackParams *params)
{
	uint8_t data[BLE_READWRITE_CHAR_ARR_SIZE] = {0};
	/* Check to see what characteristic was written, by handle */
	printf("writeCharCallback %p\r\n", Thread::gettid());
	if(params->handle == writeChar.getValueHandle()) {
		printf("Data received: length = %d, data = 0x",params->len);

		if(params->data[0] == 0x0a) global_test = 0;
		if(params->data[0] == 0x02) global_test = 1;

		for(int x=0; x < params->len; x++) {
			if ((BLE_DS_INTERFACE != NULL) && (params->data[x] != 0)) {
				BLE_DS_INTERFACE->build_command((char)params->data[x]);
			}
			printf("%x-", params->data[x]);
		}
		printf("\n\r");
	}
	/* Update the notifyChar with the value of writeChar */
	BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(writeChar.getValueHandle(), data, BLE_READWRITE_CHAR_ARR_SIZE);
}

/**
 * This function is called when the ble initialization process has failed
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
	printf("errro %d\r\n", __LINE__);
	/* Avoid compiler warnings */
	(void) ble;
	(void) error;
	/* Initialization error handling should go here */
}

/* Restart Advertising on disconnection*/
void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
	pr_debug("disconnectionCallback %p\r\n", Thread::gettid());
#if defined(USE_BLE_TICKER_TO_CHECK_TRANSFER)
	TICKER_BLE.detach();
	pr_debug("detached disconnectionCallback\r\n");
	Ble_Can_Transfer_Set(false);
#endif
	bleState = BLE_DISCONNECTED;
	BLE::Instance().gap().startAdvertising();
	BLE_DS_INTERFACE->ds_set_ble_status(false);
	queue_reset(&BLEQUEUE);
}

/* Connection */
void connectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
	pr_err("connectionCallback %p\r\n", Thread::gettid());

	Gap::ConnectionParams_t newParams = {
		.minConnectionInterval = 6,			/**< Minimum Connection Interval in 1.25 ms units, see BLE_GAP_CP_LIMITS.*/
		.maxConnectionInterval = 9,			/**< Maximum Connection Interval in 1.25 ms units, see BLE_GAP_CP_LIMITS.*/
		.slaveLatency = 0,					/**< Slave Latency in number of connection events, see BLE_GAP_CP_LIMITS.*/
		.connectionSupervisionTimeout = 600 /**< Connection Supervision Timeout in 10 ms units, see BLE_GAP_CP_LIMITS.*/
	};

	BLE::Instance().gap().updateConnectionParams(params->handle, &newParams);
	BLE::Instance().gap().stopAdvertising();
	BLE_DS_INTERFACE->ds_set_ble_status(true);
#if defined(USE_BLE_TICKER_TO_CHECK_TRANSFER)
	TICKER_BLE.attach(&Ble_Can_Transfer_Toggle, BLE_TICKER_PERIOD);
	pr_debug("Attached connectionCallback\r\n");
#endif
	//m.sensor_enable(1);
	bleState = BLE_CONNECTED;
}

/**
 * Callback triggered when the ble initialization process has finished
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
	int ret;
	BLE&        ble   = params->ble;
	ble_error_t error = params->error;

	if (error != BLE_ERROR_NONE) {
		printf("errro %d\r\n", __LINE__);
		/* In case of error, forward the error handling to onBleInitError */
		onBleInitError(ble, error);
		printf("errro %d\r\n", __LINE__);
		return;
	}
	/* Ensure that it is the default instance of BLE */
	if (ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
		printf("errro %d\r\n", __LINE__);
		return;
	}
	ble.gap().onDisconnection(disconnectionCallback);
	ble.gap().onConnection(connectionCallback);
	ble.gattServer().onDataWritten(writeCharCallback);
	/* Setup advertising */
	
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE); // BLE only, no classic BT
	ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED); // advertising type
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME)); // add name
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list)); // UUID's broadcast in advertising packet
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_TAG);
	ble.gap().setAdvertisingInterval(100); // 100ms.
	
	/* Add our custom service */
	ble.gattServer().addService(customService);
	printf("bleInitComplete\n");
	ble.gap().startAdvertising();

	ret = queue_init(&BLEQUEUE, BLEOutBuffer, BLE_NOTIFY_CHAR_ARR_SIZE, BLE_NOTIFY_CHAR_ARR_SIZE * MAX_BLE_QUEUE);
	if(ret != 0)
		printf("queue_init has failed\r\n");
}

int BLE_Icarus_TransferData(uint8_t data_transfer[20]){
	int ret;
	
	ret = BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(notifyChar.getValueHandle(), data_transfer, 20);
	return ret;
}

int BLE_Icarus_TransferDataFromQueue(){
	int ret;
	uint8_t data_transfer[20];
	unsigned char i;

	if (BLEQUEUE.num_item >= 1) {
#if defined(USE_BLE_TICKER_TO_CHECK_TRANSFER)
		if(!Ble_Can_Transfer_Check())
			return 0;
#endif
		for(i = 0; i < BLE_CONN_INT_PACKET; ++i){
			ret = dequeue(&BLEQUEUE, data_transfer);
			if(ret < 0)
				break;
			pr_debug("dequeued data for tx, %d remain\r\n", BLEQUEUE.num_item);
			if(data_transfer[0] == 0x00){
				for(i = 0; i < 20; i++)data_transfer[i]=0xAF;
			}
			BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(notifyChar.getValueHandle(), data_transfer, 20);
		}
#if defined(USE_BLE_TICKER_TO_CHECK_TRANSFER)
		TICKER_BLE.attach(&Ble_Can_Transfer_Toggle, BLE_TICKER_PERIOD);
		Ble_Can_Transfer_Set(false);
#endif
	}

	return 0;
}


//TODO: check that function for memory safety (no overflow should occur)
int BLE_Icarus_AddtoQueue(uint8_t *data_transfer, int32_t buf_size, int32_t data_size) {
	int ret = 0;
	//printf("size is: %d\r\n", size);
	// TODO: Append a known character to the byte array in case size is
	// less than 20 bytes
	while ((data_size % BLE_NOTIFY_CHAR_ARR_SIZE) && data_size < buf_size)
		data_transfer[data_size++] = 0;
	mxm_assert_msg(!(data_size % 20), "BLE packet size must be multiple of 20 bytes");

	while(data_size > 0){
		ret = enqueue(&BLEQUEUE, data_transfer);
		data_size -= BLE_NOTIFY_CHAR_ARR_SIZE;
		data_transfer += BLE_NOTIFY_CHAR_ARR_SIZE;
	}

	if(ret != 0)
		printf("BLE_Icarus_AddtoQueue has failed\r\n");

	return ret;
}


int BLE_Icarus_SetDSInterface(DSInterface *comm_obj) {
	BLE_DS_INTERFACE = comm_obj;
	return 0;
}

bool BLE_Icarus_Interface_Exists()
{
	return (bleState == BLE_CONNECTED);
}

int BLE_ICARUS_Get_Mac_Address(char MacAdress[6]){
    /* Print out device MAC address to the console*/
    Gap::AddressType_t addr_type;
    Gap::Address_t address;
    if(BLE::Instance().gap().getAddress(&addr_type, address) != 0)
    	return -1;
    for (int i = 5; i >= 0; i--){
    	MacAdress[5-i] = address[i];
	}
	pr_info("BLE_ADV_NAME:%s", DEVICE_NAME);
    return 0;
}


