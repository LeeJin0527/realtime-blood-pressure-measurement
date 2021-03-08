package com.example.ppgmeasure;

import android.Manifest;
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
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.example.ppgmeasure.PPG.PPGData;
import com.example.ppgmeasure.PPG.RealTimeGraphPPG;

import java.util.List;

public class PPGActivity extends AppCompatActivity {
    private final static String TAG = BLEControlActivity.class.getSimpleName();
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";
    private String mDeviceAddress;
    private BluetoothLeService mBluetoothLeService;

    private BluetoothGattCharacteristic writeCharacteristicObject;
    private BluetoothGattCharacteristic notifyCharacteristicObject;

    RealTimeGraphPPG ppgGraph;
    Thread realTimeThread;
    Boolean threadFlag;
    Boolean initialStartFlag;

    private TextView heartRateField;
    private TextView heartRateConfidenceField;
    private TextView grnCntField;
    private TextView grn2CntField;
    private TextView ppgActivityField;

    private LinearLayout showLayout;
    private LinearLayout progressLayout;

    Activity activity;

    Command MAXREFDES101Command;

    public PPGData ppgData;

    public String AppAbsolutePath;
    public double valueForGraph = 0;
    private CSVFile ppgCsvFile;

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
        View view = getLayoutInflater().from(this).inflate(R.layout.ppg_layout, null);
        setContentView(view);

        final Intent intent = getIntent();
        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);

        Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);

        // 버튼 initialize
        Button startbtn = (Button)findViewById(R.id.start);
        Button stopbtn = (Button)findViewById(R.id.pause);

        heartRateField = (TextView)findViewById(R.id.ppg_hr_value);
        heartRateConfidenceField = (TextView)findViewById(R.id.ppg_hr_confidence_value);
        ppgActivityField = (TextView)findViewById(R.id.ppg_sensor_activity_value);

        MAXREFDES101Command = new Command();

        realTimeThread = null;
        threadFlag = false;

        activity = PPGActivity.this;

        showLayout = (LinearLayout)findViewById(R.id.show_layout);
        progressLayout = (LinearLayout)findViewById(R.id.loading_layout);

        showLayout.setVisibility(View.GONE);
        progressLayout.setVisibility(View.VISIBLE);

        Log.i("path 확인용", getFilesDir().toString());
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, MODE_PRIVATE);//사용자에게 권한 요구
        AppAbsolutePath = getFilesDir().toString();


        ppgCsvFile= new CSVFile("PPG");

        // 데이터 핸들링 객체 initialize
        ppgData = new PPGData();


        startbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                startbtn.setVisibility(View.GONE);
                stopbtn.setVisibility(View.VISIBLE);
                ppgGraph = new RealTimeGraphPPG(view);
                threadFlag = true;
                initialStartFlag = true;
                //sendStrCmd(MAXREFDES101Command.set_cfg_accel_sh_0_a);
                //sendStrCmd(MAXREFDES101Command.set_cfg_accel_sh_0_b);
                //sendStrCmd(MAXREFDES101Command.set_cfg_accel_sh_1_a);
                //sendStrCmd(MAXREFDES101Command.set_cfg_accel_sh_1_b);
                sendStrCmd(MAXREFDES101Command.str_readppg0);
                realTimeStart();
            }
        });

        stopbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                startbtn.setVisibility(View.VISIBLE);
                stopbtn.setVisibility(View.GONE);
                ppgGraph = null;
                sendStrCmd(MAXREFDES101Command.str_stop);
                threadFlag = false;
                initialStartFlag = false;
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

                if(gattCharacteristic.getUuid().toString().equals("00001027-1212-efde-1523-785feabcd123") || gattCharacteristic.getUuid().toString().equals("00007777-1212-efde-1523-785feabcd123")){
                    writeCharacteristicObject = gattCharacteristic;
                    Log.i("적용됨?", "됏음");
                }

            }

            if((notifyCharacteristicObject != null) && (writeCharacteristicObject != null)){
                showLayout.setVisibility(View.VISIBLE);
                progressLayout.setVisibility(View.GONE);
                //로딩 종료료
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

    public void sendStrCmd(byte[] cmd){

        try {
            Thread.sleep(200);
        }catch (InterruptedException e){
            e.printStackTrace();
        }
            writeCharacteristicObject.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
            mBluetoothLeService.writeCharacteristic(writeCharacteristicObject, cmd);
            mBluetoothLeService.setCharacteristicNotification(notifyCharacteristicObject, true);


    }


    public void process_data(byte[] data){
            ppgData.setPacket(data);
            valueForGraph = ppgData.getGrnCnt();
            heartRateField.setText(String.valueOf(ppgData.getHeartRate()));
            heartRateConfidenceField.setText(String.valueOf(ppgData.getHeartRateConfidence()) + "%");

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
    }

    public void realTimeStart(){
        realTimeThread = new Thread(new Runnable() {

            @Override
            public void run() {
                while(threadFlag){
                    try {
                        if(initialStartFlag){
                            Thread.sleep(10000);
                            initialStartFlag = false;
                        }
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            ppgGraph.addEntry( ppgData.getGrnCnt(), ppgData.getGrn2Cnt());
                        }
                    });
                }
            }
        });

        realTimeThread.start();
    }

}
