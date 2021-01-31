/**
 * 온도, PPG, ECG 버튼을 클릭할 시 cmd_str을 블루투스 장치로 write하기
 *
 **/
package com.example.heartbeat;

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
import android.widget.FrameLayout;
import android.widget.SimpleExpandableListAdapter;
import android.widget.TextView;
import android.widget.Toast;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;


import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.charts.LineChart;

import org.w3c.dom.Text;

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
    private TextView ecgField;
    private TextView ppgField;

    private long lastTimeBackPressed;

    double valueForGraph = 0;



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
                Log.i(TAG, intent.getStringExtra(BluetoothLeService.EXTRA_DATA));

                process_data(notifyCharacteristicObject.getValue());
            }
        }
    };


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
/**
    @Override
    public void onBackPressed() {
        //프래그먼트 onBackPressedListener사용
        List<Fragment> fragmentList = getSupportFragmentManager().getFragments();
        for(Fragment fragment : fragmentList){
            if(fragment instanceof onBackPressedListener){
                ((onBackPressedListener)fragment).onBackPressed();
                return;
            }
        }

        //두 번 클릭시 어플 종료
        if(System.currentTimeMillis() - lastTimeBackPressed < 1500){
            finish();
            return;
        }
        lastTimeBackPressed = System.currentTimeMillis();
        Toast.makeText(this,"'뒤로' 버튼을 한 번 더 누르면 종료됩니다.",Toast.LENGTH_SHORT).show();
    }**/

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
            double value_tmp = (((data[3] & 0xff) << 8 ) + (data[2] & 0xff))/100.0;
            valueForGraph = value_tmp;
            tempField.setText(String.valueOf(value_tmp));
        }
        else if(mode.equals("ecg")){
            double value_tmp = (((data[3] & 0xff) << 8 ) + (data[2] & 0xff))/100.0;
            valueForGraph = value_tmp;

        }
        else if(mode.equals("ppg")){
            double value_tmp = (((data[3] & 0xff) << 8 ) + (data[2] & 0xff))/100.0;
            valueForGraph = value_tmp;
            ppgField.setText(String.valueOf(value_tmp));
        }else if(mode.equals("stop")){
            return;
        }
}

public void setTempvaluefield(View view, String modeConfig){
    mode = modeConfig;

    if(mode.equals("temp")){
        tempField = (TextView)view.findViewById(R.id.temp_sensor_value);
    }

    if(mode.equals("ppg")){
        ppgField = (TextView)view.findViewById(R.id.ppg_sensor_value);
    }
}



}

