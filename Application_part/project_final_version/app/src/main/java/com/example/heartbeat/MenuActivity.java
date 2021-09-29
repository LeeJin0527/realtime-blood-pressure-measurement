/**
 * 온도, PPG, ECG 버튼을 클릭할 시 cmd_str을 블루투스 장치로 write하기
 *
 **/
package com.example.heartbeat;

import android.Manifest;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.TextView;


import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.fragment.app.FragmentTransaction;


import com.example.heartbeat.ECG.FragmentECG;
import com.example.heartbeat.PPG.FragmentPPG;
import com.example.heartbeat.ECG.ECGData;
import com.example.heartbeat.PPG.PPGData;
import com.example.heartbeat.Temperature.FragmentTemp;
import com.example.heartbeat.Temperature.TempData;

public class MenuActivity extends AppCompatActivity{
    private final static String TAG = BLEControlActivity.class.getSimpleName();

    private final Command MyCmd = new Command();

    //public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

    private String mDeviceAddress;

    private BluetoothLeService mBluetoothLeService;

    private BluetoothGattCharacteristic writeCharacteristicObject;
    private BluetoothGattCharacteristic readCharacteristicObject;
    private BluetoothGattCharacteristic notifyCharacteristicObject;
    //read, write, notify를 위한 GATT characteristic

    public String mode;
    //현재 temp, ppg, or ecg인지 알 수 있는 flag

    private TextView tempField;
    private TextView tempFsField;

    private TextView ecgCountField;
    private TextView ecgRtoRField;
    private TextView ecgRtoRBpmField;
    private TextView ecg1Field;
    private TextView ecg2Field;
    private TextView ecg3Field;

    private TextView ppgField;
    private TextView ppgConfidenceField;
    private TextView ppgActivityField;

    public TempData tempData;
    public ECGData ecgData;
    public PPGData ppgData;

    public String AppAbsolutePath;

    public double valueForGraph = 0;

    private CSVFile tempCsvFile;
    private CSVFile ppgCsvFile;
    public CSVFile ecgCSVFile;




    private boolean mConnected = false;

    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            mBluetoothLeService = ((BluetoothLeService.LocalBinder) service).getService();
            if (!mBluetoothLeService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                finish();
            }
            mBluetoothLeService.connect(mDeviceAddress);

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
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                mConnected = false;
            } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                setupGattCharacteristic(mBluetoothLeService.getSupportedGattServices());
            } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {
                //Log.i(TAG, intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
                String tmp_data = intent.getStringExtra(BluetoothLeService.EXTRA_DATA);
                byte[] hexaData = hexaStringToByteArray(tmp_data);
                StringBuilder sb = new StringBuilder();
                for(final byte tmp : hexaData){
                    sb.append(String.format("%02x", tmp&0xff));
                }
                //Log.i("원본", tmp_data);
                Log.i("변환", sb.toString());


                process_data(hexaData);
            }
        }
    };

    public byte[] hexaStringToByteArray(String hexaString){
        int length = hexaString.length();
        byte [] data = new byte[length/2];

        for(int i = 0; i  < length; i += 2){
            data[i / 2] = (byte) ((Character.digit(hexaString.charAt(i), 16) << 4)
                    + Character.digit(hexaString.charAt(i+1), 16));
        }
        return data;
    }



    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.menu);

        final Intent intent = getIntent();
        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
        
        Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);
        //bindService 메서드가 호출되면 onServiceConnected 호출되는거 같음

        // 버튼 initialize
        Button tempbtn = (Button)findViewById(R.id.tempbtn);
        Button ecgbtn = (Button)findViewById(R.id.ecgbtn);
        Button ppgbtn = (Button)findViewById(R.id.ppgbtn);

        Log.i("path 확인용", getFilesDir().toString());
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, MODE_PRIVATE);//사용자에게 권한 요구
        AppAbsolutePath = getFilesDir().toString();

        tempCsvFile = new CSVFile("Temp");
        ppgCsvFile= new CSVFile("PPG");
        ecgCSVFile = new CSVFile("ECG");


        // 데이터 핸들링 객체 initialize
        tempData = new TempData();
        ppgData = new PPGData();
        ecgData = new ECGData();

        tempbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){


                FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
                FragmentTemp fragmentTemp = new FragmentTemp();

                transaction.replace(R.id.menu_fragment, fragmentTemp);
                FrameLayout frameLayout = (FrameLayout)findViewById(R.id.menu_fragment);
                frameLayout.setVisibility(View.VISIBLE);
                transaction.addToBackStack(null);

                transaction.commit();
            }
        });

        ecgbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){

                FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
                FragmentECG fragmentECG = new FragmentECG();

                transaction.replace(R.id.menu_fragment, fragmentECG);
                FrameLayout frameLayout = (FrameLayout)findViewById(R.id.menu_fragment);
                frameLayout.setVisibility(View.VISIBLE);
                transaction.addToBackStack(null);

                transaction.commit();
            }
        });

        ppgbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){

                FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
                FragmentPPG fragmentPPG = new FragmentPPG();

                transaction.replace(R.id.menu_fragment, fragmentPPG);
                FrameLayout frameLayout = (FrameLayout)findViewById(R.id.menu_fragment);
                frameLayout.setVisibility(View.VISIBLE);
                transaction.addToBackStack(null);
                //뒤로가기

                transaction.commit();
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


    //특정 service들을 구성하는 characteristics를 구해주는 매소드
    private void setupGattCharacteristic(List<BluetoothGattService> gattServices){
        if (gattServices == null) return;

        for (BluetoothGattService gattService : gattServices) {
            List<BluetoothGattCharacteristic> gattCharacteristics =
                    gattService.getCharacteristics();

            for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                Log.i("GATT 확인", gattCharacteristic.getUuid().toString());
                //notify
                if(gattCharacteristic.getUuid().toString().equals("00001011-1212-efde-1523-785feabcd123")){
                    notifyCharacteristicObject = gattCharacteristic;

                }

                //read
                if(gattCharacteristic.getUuid().toString().equals("00001234-1212-efde-1523-785feabcd123")){
                    readCharacteristicObject = gattCharacteristic;
                }

                if(gattCharacteristic.getUuid().toString().equals("00001027-1212-efde-1523-785feabcd123") || gattCharacteristic.getUuid().toString().equals("00007777-1212-efde-1523-785feabcd123")){
                    writeCharacteristicObject = gattCharacteristic;
                    Log.i("적용됨?", "됏음");
                }
            }
        }
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
        return intentFilter;
    }

    private void btnView(Boolean choice){
        Button tempbtn = (Button)findViewById(R.id.tempbtn);
        Button ecgbtn = (Button)findViewById(R.id.ecgbtn);
        Button ppgbtn = (Button)findViewById(R.id.ppgbtn);

        if(choice == true){
            tempbtn.setVisibility(View.VISIBLE);
            ecgbtn.setVisibility(View.VISIBLE);
            ppgbtn.setVisibility(View.VISIBLE);
        }
        if(choice == false){
            tempbtn.setVisibility(View.INVISIBLE);
            ecgbtn.setVisibility(View.INVISIBLE);
            ppgbtn.setVisibility(View.INVISIBLE);
        }
    }
public interface onKeyBack{
        void onBackKey();
}

public FrameLayout getFrameLayout(){
        return (FrameLayout)findViewById(R.id.menu_fragment);
}

public void sendStrCmd(byte[] cmd){
    writeCharacteristicObject.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
    mBluetoothLeService.writeCharacteristic(writeCharacteristicObject, cmd);
    mBluetoothLeService.setCharacteristicNotification(notifyCharacteristicObject, true);
}


public void process_data(byte[] data){


        if(mode.equals("temp")){
            tempData.setPacket(data);
            valueForGraph = tempData.getValue();
            tempField.setText(String.valueOf(tempData.getValue()));
            tempFsField.setText(String.valueOf(((float) tempData.convertCelciustoF())));
            tempCsvFile.writeTempFile(tempData);

        }
        else if(mode.equals("ecg")){
            ecgData.setPacket(data);
            valueForGraph = ecgData.getTest_value_graph();
            ecgRtoRField.setText(String.valueOf(ecgData.getRtoR()));
            ecgRtoRBpmField.setText(String.valueOf(ecgData.getRtoRBpm()));
            ecgCSVFile.writeECGFile(ecgData);
        }
        else if(mode.equals("ppg")){
            ppgData.setPacket(data);
            valueForGraph = ppgData.getGrnCnt();
            ppgField.setText(String.valueOf(ppgData.getHeartRate()));
            ppgConfidenceField.setText(String.valueOf(ppgData.getHeartRateConfidence()) + "%");

            if(ppgData.getPpgActivity() == 0){
                ppgActivityField.setText("rest");
            }
            if(ppgData.getPpgActivity() == 1){
                ppgActivityField.setText("Non-rhythmic activity");
            }
            if(ppgData.getPpgActivity() == 2){
                ppgActivityField.setText("walking");
            }
            if(ppgData.getPpgActivity() == 3){
                ppgActivityField.setText("running");
            }
            if(ppgData.getPpgActivity() == 4){
                ppgActivityField.setText("biking");
            }
            if(ppgData.getPpgActivity() == 5){
                ppgActivityField.setText("rhythmic activity");
            }
            ppgCsvFile.writePPGFile(ppgData);

        }else if(mode.equals("stop")){
            return;
        }
}

public void setViewField(View view, String modeConfig){
    mode = modeConfig;

    if(mode.equals("temp")){
        tempField = (TextView)view.findViewById(R.id.temp_sensor_value);
        tempFsField = (TextView)view.findViewById(R.id.sensor_Fahrenheit_scale_count);
    }

    if(mode.equals("ppg")){
        ppgField = (TextView)view.findViewById(R.id.ppg_sensor_value);
        ppgConfidenceField = (TextView)view.findViewById(R.id.ppg_sensor_confidence_value);
        ppgActivityField = (TextView)view.findViewById(R.id.ppg_sensor_activity_value);
    }

    if(mode.equals("ecg")){
        ecgRtoRField = (TextView)view.findViewById(R.id.ecg_rtor_value);
        ecgRtoRBpmField = (TextView)view.findViewById(R.id.ecg_rtorbpm_value);
    }
}


}

