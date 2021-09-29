/**
 * 온도, PPG, ECG 버튼을 클릭할 시 cmd_str을 블루투스 장치로 write하기
 * 
 **/
package com.example.heartbeat;

import android.app.Activity;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ExpandableListView;
import android.widget.SimpleExpandableListAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class BLEControlActivity extends Activity{
    private final static String TAG = BLEControlActivity.class.getSimpleName();

    public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

    private byte[] tmp_data;

    private TextView mDisplayData;

    private TextView mConnectionState;
    private TextView mDataField;
    private String mDeviceName;
    private String mDeviceAddress;
    private ExpandableListView mGattServicesList;
    private BluetoothLeService mBluetoothLeService;
    private ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics =
            new ArrayList<ArrayList<BluetoothGattCharacteristic>>();

    private BluetoothGattCharacteristic writeCharacteristicObject;
    private BluetoothGattCharacteristic readCharacteristicObject;
    private BluetoothGattCharacteristic notifyCharacteristicObject;
    //read, write, notify를 위한 GATT characteristic

    private TextView mCountField;
    private TextView mSensorValueField;
    //UI id참조 변수 정의
    
    private boolean mConnected = false;
    private BluetoothGattCharacteristic mNotifyCharacteristic;


    private final String LIST_NAME = "NAME";
    private final String LIST_UUID = "UUID";

    //hex 배열 데이터를 처리해서 text view ui에 표시해주는 메소드
    private void process_data(byte []data){
        data = notifyCharacteristicObject.getValue();
        //StringBuilder celcius_str = new StringBuilder();
        //StringBuilder count_str = new StringBuilder();

        int count_tmp = ((data[1] & 0xff));
        double value_tmp = (((data[3] & 0xff) << 8 ) + (data[2] & 0xff))/100.0;

        String count_str = String.valueOf(count_tmp);
        String celcius_str = String.valueOf(value_tmp);

        mCountField.setText(count_str);
        mSensorValueField.setText(celcius_str);


    }

    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            mBluetoothLeService = ((BluetoothLeService.LocalBinder) service).getService();
            if (!mBluetoothLeService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                finish();
            }
            // Automatically connects to the device upon successful start-up initialization.
            mBluetoothLeService.connect(mDeviceAddress);
            Log.e(TAG,"mac 주소? : " + mDeviceAddress);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mBluetoothLeService = null;
        }
    };

    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
                mConnected = true;
                updateConnectionState(R.string.connected);
                invalidateOptionsMenu();
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                mConnected = false;
                updateConnectionState(R.string.disconnected);
                invalidateOptionsMenu();
                clearUI();
            } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                // Show all the supported services and characteristics on the user interface.
                displayGattServices(mBluetoothLeService.getSupportedGattServices());
                setupGattCharacteristic(mBluetoothLeService.getSupportedGattServices());
            } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {
                displayData(intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
                process_data(notifyCharacteristicObject.getValue());
                Log.i(TAG, intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
            }
        }
    };

    private final ExpandableListView.OnChildClickListener servicesListClickListner =
            new ExpandableListView.OnChildClickListener() {
                @Override
                public boolean onChildClick(ExpandableListView parent, View v, int groupPosition,
                                            int childPosition, long id) {
                    if (mGattCharacteristics != null) {
                        final BluetoothGattCharacteristic characteristic =
                                mGattCharacteristics.get(groupPosition).get(childPosition);
                        final int charaProp = characteristic.getProperties();
                        Log.i(TAG,"characteristic은 " + Integer.toString(charaProp));

                        //if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
                        if(charaProp == BluetoothGattCharacteristic.PROPERTY_READ){
                            // If there is an active notification on a characteristic, clear
                            // it first so it doesn't update the data field on the user interface.
                            if (mNotifyCharacteristic != null) {
                                Log.i(TAG, "read내에서 notify set 수행");

                                mBluetoothLeService.setCharacteristicNotification(mNotifyCharacteristic, true);

                                mNotifyCharacteristic = null;
                            }
                            Log.i(TAG,"Read 분기문!");
                            mBluetoothLeService.readCharacteristic(characteristic);
                        }
                        //if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
                        if(charaProp == BluetoothGattCharacteristic.PROPERTY_NOTIFY){
                            Log.i(TAG,"Notify 분기문!");
                            mNotifyCharacteristic = characteristic;
                            mBluetoothLeService.setCharacteristicNotification(
                                    characteristic, true);
                        }
                        //여기서 부터 write characteristic을 추가하면 될듯
                        //watch(Peripheral영역에서는) propertise가 write가 8, notify가 16(0x10)
                        //read가 2로 정의되어 있습니다.

                        //android에서 BluetoothGattCharacteristic 클래스의 멤버들을 보면, write가 8인데
                        //지금 Log메소드 이용해서 출력해보면 10을 가짐?? why...
                        if(charaProp == 10){
                            Log.i(TAG, "write 분기문!");
                            byte[] str_cmd = {0x72, 0x65, 0x61, 0x64, 0x20, 0x74, 0x65, 0x6d, 0x70, 0x20, 0x30, 0x0a, 0x00, 0x00, 0x00, 0x00};
                            //byte[] str_cmd = {'r', 'e', 'a', 'd'};
                            //characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                            characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                            mBluetoothLeService.writeCharacteristic(characteristic, str_cmd);
                            //공부한다고 해당 characteristic클릭하면 read temp 0를 보내도록 해놨습니다.
                            //원래는 아래 처럼 버튼 클릭시 cmd str 보내도록 해야합니다.


                        }

                        return true;
                    }
                    return false;
                }
            };

    public void btnTempClick(){
        byte[] str_cmd = {0x72, 0x65, 0x61, 0x64, 0x20, 0x74, 0x65, 0x6d, 0x70, 0x20, 0x30, 0x0a, 0x00, 0x00, 0x00, 0x00};
        //read temp 0

        writeCharacteristicObject.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        mBluetoothLeService.writeCharacteristic(writeCharacteristicObject, str_cmd);
        mBluetoothLeService.setCharacteristicNotification(notifyCharacteristicObject, true);

    }

    public void btnECGClick(){
        byte[] str_cmd = {0x72, 0x65, 0x61, 0x64, 0x20, 0x65, 0x63, 0x67, 0x20, 0x31, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00};
        //read ecg 1

        writeCharacteristicObject.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        mBluetoothLeService.writeCharacteristic(writeCharacteristicObject, str_cmd);
        mBluetoothLeService.setCharacteristicNotification(notifyCharacteristicObject, true);


    }

    public void btnPPGClick(){
        byte[] str_cmd = {0x72, 0x65, 0x61, 0x64, 0x20, 0x70, 0x70, 0x67, 0x20, 0x30, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00};
        //read ppg 0

        writeCharacteristicObject.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        mBluetoothLeService.writeCharacteristic(writeCharacteristicObject, str_cmd);
        mBluetoothLeService.setCharacteristicNotification(notifyCharacteristicObject, true);

    }

    public void btnSTOPClick(){
        byte[] str_cmd = {0x73, 0x74, 0x6f, 0x70, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        //read ppg 0
        writeCharacteristicObject.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        mBluetoothLeService.writeCharacteristic(writeCharacteristicObject, str_cmd);
        mBluetoothLeService.setCharacteristicNotification(notifyCharacteristicObject, false);

    }

    private void clearUI() {
        mGattServicesList.setAdapter((SimpleExpandableListAdapter) null);
        mDataField.setText(R.string.no_data);

    }
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gatt_services_characteristics);

        final Intent intent = getIntent();
        mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);

        // Sets up UI references.
        ((TextView) findViewById(R.id.device_address)).setText(mDeviceAddress);
        mGattServicesList = (ExpandableListView) findViewById(R.id.gatt_services_list);
        mGattServicesList.setOnChildClickListener(servicesListClickListner);
        mConnectionState = (TextView) findViewById(R.id.connection_state);
        mDataField = (TextView) findViewById(R.id.data_value);


        mCountField = (TextView) findViewById(R.id.sensor_count);
        mSensorValueField = (TextView) findViewById(R.id.sensor_value);


        Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);

        // 버튼 initialize
        Button tempbtn = (Button)findViewById(R.id.tempbtn);
        Button ecgbtn = (Button)findViewById(R.id.ecgbtn);
        Button ppgbtn = (Button)findViewById(R.id.ppgbtn);
        Button stopbtn = (Button)findViewById(R.id.stopbtn);

        tempbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                btnTempClick();
            }
        });

        ecgbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                btnECGClick();
            }
        });

        ppgbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                btnPPGClick();
            }
        });

        stopbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                btnSTOPClick();
            }
        });



    }

    @Override
    protected void onResume() {
        super.onResume();
        registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
        if (mBluetoothLeService != null) {
            final boolean result = mBluetoothLeService.connect(mDeviceAddress);
            Log.d(TAG, "Connect request result=" + result);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(mGattUpdateReceiver);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindService(mServiceConnection);
        mBluetoothLeService = null;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.gatt_services, menu);
        if (mConnected) {
            menu.findItem(R.id.menu_connect).setVisible(false);
            menu.findItem(R.id.menu_disconnect).setVisible(true);
        } else {
            menu.findItem(R.id.menu_connect).setVisible(true);
            menu.findItem(R.id.menu_disconnect).setVisible(false);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.menu_connect:
                mBluetoothLeService.connect(mDeviceAddress);
                return true;
            case R.id.menu_disconnect:
                mBluetoothLeService.disconnect();
                return true;
            case android.R.id.home:
                onBackPressed();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void updateConnectionState(final int resourceId) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mConnectionState.setText(resourceId);
            }
        });
    }


    private void displayData(String data) {
        if (data != null) {
            mDataField.setText(data);

        }
    }
    
    //특정 service들을 구성하는 characteristics를 구해주는 매소드
    private void setupGattCharacteristic(List<BluetoothGattService> gattServices){
        if (gattServices == null) return;

        for (BluetoothGattService gattService : gattServices) {
            List<BluetoothGattCharacteristic> gattCharacteristics =
                    gattService.getCharacteristics();

            for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                //notify
                if(gattCharacteristic.getUuid().toString().equals("00001011-1212-efde-1523-785feabcd123")){
                    notifyCharacteristicObject = gattCharacteristic;
                }

                //read
                if(gattCharacteristic.getUuid().toString().equals("00001234-1212-efde-1523-785feabcd123")){
                    readCharacteristicObject = gattCharacteristic;
                }

                //write
                if(gattCharacteristic.getUuid().toString().equals("00001027-1212-efde-1523-785feabcd123")){
                    writeCharacteristicObject = gattCharacteristic;
                }
            }
        }
    }

    // Demonstrates how to iterate through the supported GATT Services/Characteristics.
    // In this sample, we populate the data structure that is bound to the ExpandableListView
    // on the UI.
    private void displayGattServices(List<BluetoothGattService> gattServices) {
        if (gattServices == null) return;
        String uuid = null;
        String unknownServiceString = getResources().getString(R.string.unknown_service);
        String unknownCharaString = getResources().getString(R.string.unknown_characteristic);
        ArrayList<HashMap<String, String>> gattServiceData = new ArrayList<HashMap<String, String>>();
        ArrayList<ArrayList<HashMap<String, String>>> gattCharacteristicData
                = new ArrayList<ArrayList<HashMap<String, String>>>();
        mGattCharacteristics = new ArrayList<ArrayList<BluetoothGattCharacteristic>>();

        // Loops through available GATT Services.
        for (BluetoothGattService gattService : gattServices) {
            HashMap<String, String> currentServiceData = new HashMap<String, String>();
            uuid = gattService.getUuid().toString();

            currentServiceData.put(LIST_UUID, uuid);
            gattServiceData.add(currentServiceData);

            ArrayList<HashMap<String, String>> gattCharacteristicGroupData =
                    new ArrayList<HashMap<String, String>>();
            List<BluetoothGattCharacteristic> gattCharacteristics =
                    gattService.getCharacteristics();
            ArrayList<BluetoothGattCharacteristic> charas =
                    new ArrayList<BluetoothGattCharacteristic>();

            // Loops through available Characteristics.

            for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                charas.add(gattCharacteristic);
                HashMap<String, String> currentCharaData = new HashMap<String, String>();
                uuid = gattCharacteristic.getUuid().toString();

                currentCharaData.put(LIST_UUID, uuid);
                gattCharacteristicGroupData.add(currentCharaData);
            }

            mGattCharacteristics.add(charas);

            Log.i(TAG, "현재 특성은 : " + mGattCharacteristics.toString());
            Log.i(TAG, "0, 2번째는 : "+ mGattCharacteristics.get(0).get(2).toString());
            gattCharacteristicData.add(gattCharacteristicGroupData);
        }

        SimpleExpandableListAdapter gattServiceAdapter = new SimpleExpandableListAdapter(
                this,
                gattServiceData,
                android.R.layout.simple_expandable_list_item_2,
                new String[] {LIST_NAME, LIST_UUID},
                new int[] { android.R.id.text1, android.R.id.text2 },
                gattCharacteristicData,
                android.R.layout.simple_expandable_list_item_2,
                new String[] {LIST_NAME, LIST_UUID},
                new int[] { android.R.id.text1, android.R.id.text2 }
        );
        mGattServicesList.setAdapter(gattServiceAdapter);
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
        return intentFilter;
    }
}
