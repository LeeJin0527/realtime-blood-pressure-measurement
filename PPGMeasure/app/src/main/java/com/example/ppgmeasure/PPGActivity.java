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
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;


import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.example.ppgmeasure.PPG.PPGData;
import com.example.ppgmeasure.PPG.SocketCommunication;
import com.github.lzyzsd.circleprogress.ArcProgress;

import org.apache.commons.math3.geometry.euclidean.twod.Line;
import org.apache.commons.math3.geometry.spherical.oned.Arc;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.List;

public class PPGActivity extends AppCompatActivity {
    private final static String TAG = PPGActivity.class.getSimpleName();
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";
    private String mDeviceAddress;
    private BluetoothLeService mBluetoothLeService;

    private BluetoothGattCharacteristic writeCharacteristicObject;
    private BluetoothGattCharacteristic notifyCharacteristicObject;


    private TextView heartRateField;
    private TextView heartRateConfidenceField;
    private TextView ppgActivityField;
    private TextView serverData;

    private LinearLayout showLayout;
    private LinearLayout progressLayout;
    private LinearLayout progressDataLayout;
    private LinearLayout inputInitialLayout;

    private EditText SBPEditText;

    Activity activity;

    Command MAXREFDES101Command;

    public PPGData ppgData;

    private CSVFile ppgCsvFile;

    private boolean mConnected = false;
    int count = 0;
    int numberOfData = 0;

    SocketCommunication SC;
    int currentData = -1;
    LinearLayout signBanner;
    Thread t;
    upDateLoading U;
    String SBP;

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

    // 데이터 입력이 두번째로 일어나는 매서드입니다.
    // 첫번재는 BluetoothLeService.java의 onCharacteristicChanged 매서드
    // 세번째는 process_data 매서드
    // BluetoothLeService의 onCharacteristicChanged -> mGattUpdateReceiver의 onReceive -> PPGActivity의
    // process_data 매서드
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
                // MAXREFDES101로 부터 읽어들인 데이터를 문자열 형식으로 받아옵니다
                
                byte[] hexaData = hexaStringToByteArray(tmp_data);
                //문자열(String)을 다시 16진수 byte 배열로 변환

                process_data(hexaData);
            }
        }
    };

    // 문자열(String)을 다시 16진수 byte 배열로 변환해주는 매서드
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
        Button sendSocketMsg = (Button)findViewById(R.id.send_by_socket);
        Button sendSBP = (Button)findViewById(R.id.send_by_socket_initial_value);

        heartRateField = (TextView)findViewById(R.id.ppg_hr_value);
        heartRateConfidenceField = (TextView)findViewById(R.id.ppg_hr_confidence_value);
        ppgActivityField = (TextView)findViewById(R.id.ppg_sensor_activity_value);
        serverData = (TextView)findViewById(R.id.servercalcdata);

        MAXREFDES101Command = new Command();

        activity = PPGActivity.this;

        showLayout = (LinearLayout)findViewById(R.id.show_layout);
        progressLayout = (LinearLayout)findViewById(R.id.loading_layout);
        progressDataLayout = (LinearLayout)findViewById(R.id.data_loading_layout);
        inputInitialLayout = (LinearLayout)findViewById(R.id.input_initial);

        SBPEditText = (EditText)findViewById(R.id.SBP);

        //showLayout.setVisibility(View.VISIBLE);
        //progressLayout.setVisibility(View.GONE);
        showLayout.setVisibility(View.GONE);
        inputInitialLayout.setVisibility(View.VISIBLE);


        // CSV 파일 생성
        ppgCsvFile= new CSVFile("PPG");

        // 데이터 핸들링 객체 initialize
        ppgData = new PPGData();

        // 서버로 부터 받은 데이터에 대해 banner 색깔별로 표시
        signBanner = (LinearLayout)findViewById(R.id.serverDataColor);

        // connected to server by socket
        String ip = plzenterIP;
        int port = 1221;//enter server process port number
        SC = new SocketCommunication(ip, port);
        setServerData SD = new setServerData();
        t = new Thread(SD);
        t.start();

        //초기 혈압 값 SBP 버튼에 대한 동작 정의
        sendSBP.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                showLayout.setVisibility(View.VISIBLE);
                inputInitialLayout.setVisibility(View.GONE);
                SBP = SBPEditText.getText().toString();
                SC.sendSBP(SBP);
            }
        });

        // 측정하기 버튼에 대한 동작 정의
        startbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                numberOfData = 0;
                ppgCsvFile= new CSVFile("PPG");
                startbtn.setVisibility(View.GONE);
                stopbtn.setVisibility(View.VISIBLE);
                progressDataLayout.setVisibility(View.VISIBLE);

                 U = new upDateLoading(findViewById(R.id.arc_progress));
                 U.start();

                sendStrCmd(MAXREFDES101Command.str_readppg0);
                // PPG 측정을 시작하기 위해, MAXREFDES101에 read ppg 0라고 
                // 명령어를 전송합니다
            }
        });

        // 일시정지 버튼에 대한 동작 정의
        stopbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                U.interrupt();
                progressDataLayout.setVisibility(View.GONE);
                count++;
                if(count > 0 && numberOfData > 660){sendSocketMsg.setVisibility(View.VISIBLE);}
                startbtn.setVisibility(View.VISIBLE);
                stopbtn.setVisibility(View.GONE);

                sendStrCmd(MAXREFDES101Command.str_stop);
                // 일시정지 버튼을 누르면, 현재 MAXREFDES101이 수행하는 동작을 멈추기 위해
                // "stop" 이라는 문자열을 MAXREFDES101에 전송합니다

            }
        });

        // 서버로 파일 보내기 버튼 동작 정의
        sendSocketMsg.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                SC.sendPacket();
            }
        });

    }

    // 뒤로가기 버튼 누르면 연결 종료
    @Override
    public void onBackPressed() {
        SC.closeSocket();
        t.interrupt();
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
        SC.closeSocket();
        t.interrupt();
    }


    // 심박 측정 서비스의 읽기/쓰기, 알림(notification)에 대한 속성을 설정하는 매서드
    private void setupGattCharacteristic(List<BluetoothGattService> gattServices){
        if (gattServices == null) return;

        for (BluetoothGattService gattService : gattServices) {
            List<BluetoothGattCharacteristic> gattCharacteristics =
                    gattService.getCharacteristics();

            for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                if(gattCharacteristic.getUuid().toString().equals("00001011-1212-efde-1523-785feabcd123")){
                    notifyCharacteristicObject = gattCharacteristic;
                    mBluetoothLeService.setCharacteristicNotification(notifyCharacteristicObject, true);
                }

                if(gattCharacteristic.getUuid().toString().equals("00001027-1212-efde-1523-785feabcd123") || gattCharacteristic.getUuid().toString().equals("00007777-1212-efde-1523-785feabcd123")){
                    writeCharacteristicObject = gattCharacteristic;
                    writeCharacteristicObject.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                }

            }

            // 읽기/쓰기, 알림 속성 설정이 완료되어 로딩을 끝내는 부분
            if((notifyCharacteristicObject != null) && (writeCharacteristicObject != null)){
                progressDataLayout.setVisibility(View.VISIBLE);
                progressLayout.setVisibility(View.GONE);
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

    // 수행할 동작에 관한 명령어를 휴대폰에서 MAXREFDES101로 보내는 부분
    // 예)read ppg 0, read ecg 2, read temp 0, stop등등
    public void sendStrCmd(byte[] cmd){

        try {
            Thread.sleep(200);
        }catch (InterruptedException e){
            e.printStackTrace();
        }
            mBluetoothLeService.writeCharacteristic(writeCharacteristicObject, cmd);
    }


    // 16진수 배열형식의 데이터를 입력하는 부분과 csv파일을 작성하는 부분
    public void process_data(byte[] data){
            ppgData.setPacket(data);
            heartRateField.setText(String.valueOf(ppgData.getHeartRate()));
            heartRateConfidenceField.setText(String.valueOf(ppgData.getHeartRateConfidence()) + "%");
            numberOfData++;

            // 액티비티(활동)에 대한 데이터 정보는 PPGData를 참조
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

            if(numberOfData > 100) {
                ppgCsvFile.writePPGFile(ppgData);
                // CSV를 작성하는 매서드를 호출합니다.
                // flag를 둔 이유는 처음 10초 동안은 그래프에 데이터가
                // 표시되지 않을때, CSV파일 작성도 하지 않아야 하기 때문
            }
    }


    public void changeSignBanner(int data){
        System.out.printf("data in chageSignBanner = %d\n", data);
        switch (data){
            case 0:
                signBanner.setBackgroundColor(getResources().getColor(R.color.green));
                System.out.println("Black!");
                break;
            case 1:
                signBanner.setBackgroundColor(getResources().getColor(R.color.yellow));
                System.out.println("White!");
                break;
            case 2:
                signBanner.setBackgroundColor(getResources().getColor(R.color.red));
                System.out.println("Purple!");
                break;
            default:
                break;
        }
    }

    // 이 부분은 나중에 여건되면, 고치는게 좋을듯함
    // 스레드로 1초마다 확인하지말고, 인터럽트나 콜백이나 다른 방법으로
    // 좀 더 우아하게 서버 데이터를 비동기적으로 확인하는 방법이 필요
    class setServerData implements Runnable{
        int tmp;
        @Override
        public void run() {
            while(!Thread.currentThread().isInterrupted()){

                tmp = SC.getDataFromServer();
                if (tmp != currentData) {
                    currentData = tmp;
                    changeSignBanner(currentData);
                }
                if (tmp == -2) break;
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException I) {
                    I.printStackTrace();
                }

            }
            System.out.println("thread terminated");
        }
    }

    class upDateLoading extends Thread{
        ArcProgress AP;

        upDateLoading(ArcProgress AP){
            this.AP = AP;
        }

        @Override
        public void run() {
            while (numberOfData < 660 && !Thread.currentThread().isInterrupted()) {
                try{
                    AP.setProgress((int)(numberOfData/660.0 * 100));
                    Thread.sleep(1000);
                }catch (InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
    }

}
