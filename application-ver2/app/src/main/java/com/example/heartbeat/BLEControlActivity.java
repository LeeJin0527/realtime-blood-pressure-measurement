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
import android.graphics.Color;
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

import androidx.appcompat.app.AppCompatActivity;


import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.charts.LineChart;

public class BLEControlActivity extends Activity{
    private final static String TAG = BLEControlActivity.class.getSimpleName();

    public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

    private LineChart chart;

    private byte[] tmp_data;
    private double test_data = 0;

    private TextView mDisplayData;

    private Thread thread;

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

        test_data = value_tmp;

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
                //clearUI();
            } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                // Show all the supported services and characteristics on the user interface.
                //displayGattServices(mBluetoothLeService.getSupportedGattServices());
                setupGattCharacteristic(mBluetoothLeService.getSupportedGattServices());
            } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {

                process_data(notifyCharacteristicObject.getValue());
                Log.i(TAG, intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
            }
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


        mConnectionState = (TextView) findViewById(R.id.connection_state);



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

        creatGraph();

        //chart init

        new Thread(new Runnable() {
            @Override
            public void run() {
                while(true){
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            addEntry(test_data);
                        }
                    });
                }
            }
        }).start();


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
  /**  private void displayGattServices(List<BluetoothGattService> gattServices) {
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


    }
**/
    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
        return intentFilter;
    }

    private void creatGraph(){
        chart = (LineChart)findViewById(R.id.LineChart);

        chart.setDrawGridBackground(true);
        chart.setBackgroundColor(Color.BLACK);
        chart.setGridBackgroundColor(Color.BLACK);

        chart.getXAxis().setDrawGridLines(true);
        chart.getXAxis().setDrawAxisLine(false);

        chart.getXAxis().setEnabled(true);
        chart.getXAxis().setDrawGridLines(false);

        YAxis leftAxis = chart.getAxisLeft();
        leftAxis.setEnabled(true);
        //leftAxis.setTextColor(getResources().getColor(R.color.colorGrid));
        leftAxis.setDrawGridLines(true);
        //leftAxis.setGridColor(getResources().getColor(R.color.colorGrid));

        YAxis rightAxis = chart.getAxisRight();
        rightAxis.setEnabled(false);

        chart.invalidate();


    }
    private void addEntry(double num) {
        Log.i(TAG, "addEntry 호출!");
        LineData data = chart.getData();

        if (data == null) {
            data = new LineData();
            chart.setData(data);
        }

        ILineDataSet set = data.getDataSetByIndex(0);
        // set.addEntry(...); // can be called as well

        if (set == null) {
            set = createSet();
            data.addDataSet(set);
        }



        data.addEntry(new Entry((float)set.getEntryCount(), (float)num), 0);
        data.notifyDataChanged();

        // let the chart know it's data has changed
        chart.notifyDataSetChanged();

        chart.setVisibleXRangeMaximum(150);
        // this automatically refreshes the chart (calls invalidate())
        chart.moveViewTo(data.getEntryCount(), 50f, YAxis.AxisDependency.LEFT);

    }

    private LineDataSet createSet() {



        LineDataSet set = new LineDataSet(null, "Real-time Line Data");
        set.setLineWidth(1f);
        set.setDrawValues(false);
        //set.setValueTextColor(getResources().getColor(Color.WHITE));
        //set.setColor(getResources().getColor(Color.WHITE));
        set.setMode(LineDataSet.Mode.LINEAR);
        set.setDrawCircles(false);
        set.setHighLightColor(Color.rgb(190, 190, 190));

        return set;
    }
}
