package com.example.ppgmeasure;


import android.Manifest;
import android.app.Activity;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelUuid;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.PagerSnapHelper;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.SnapHelper;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    private BluetoothAdapter mBLEadapter;

    private ArrayList<BluetoothDevice> mBLElist = null;

    private BLEDataAdapter mBLEDeviceAdapter = null;

    private RecyclerView mBLERecyclerView = null;

    private static final int REQUEST_ENABLE_BT = 1;

    ScanFilter filter = null;
    ScanSettings scanSettings = null;


    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)){
            Toast.makeText(this, "BLE를 지원하지 않는 기기입니다!", Toast.LENGTH_SHORT).show();
            finish();
        }
        final BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBLEadapter = bluetoothManager.getAdapter();
        if(mBLEadapter == null){
            Toast.makeText(this, "BLE를 지원하지 않는 기기입니다.", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        setContentView(R.layout.mainactivity_recyclerview);
        mBLERecyclerView = findViewById(R.id.recyclerview);
    }


    @Override
    protected void onResume(){
        super.onResume();
               if (!mBLEadapter.isEnabled()) {
            if (!mBLEadapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
        }

        mBLElist = new ArrayList<BluetoothDevice>();

        // Initializes recycler view adapter and onClickListener.
        mBLEDeviceAdapter = new BLEDataAdapter(mBLElist);

        // 저전력블루투스로 스캔된 장비를 클릭 할 경우, PPGActivity로 넘어가는 매서드입니다.
        // 넘어갈때, 연결할 장비의 MAC 주소와 장비 이름을 넘겨줍니다.
        mBLEDeviceAdapter.setItemClickListener(new BLEDataAdapter.ItemClickListener() {
            @Override
            public void onItemClick(View view, int position) {
                Log.i("리스트 클릭 index 확인", String.valueOf(position));
                final String BLEDeviceName = mBLElist.get(position).getName();
                final String BLEDeviceMAC = mBLElist.get(position).getAddress();

                final Intent intent = new Intent(view.getContext(), PPGActivity.class);
                intent.putExtra("DEVICE_NAME", BLEDeviceName);
                intent.putExtra("DEVICE_ADDRESS", BLEDeviceMAC);
                scanLeDevice(false);

                startActivity(intent);

            }
        });
        mBLERecyclerView.setAdapter(mBLEDeviceAdapter);

        mBLERecyclerView.setLayoutManager(new LinearLayoutManager(this, RecyclerView.HORIZONTAL,false));

        SnapHelper snapHelper = new PagerSnapHelper();
        if(mBLERecyclerView.getOnFlingListener() == null)snapHelper.attachToRecyclerView(mBLERecyclerView);

        mBLERecyclerView.addOnScrollListener(new RecyclerView.OnScrollListener(){
            @Override
            public void onScrollStateChanged(@NonNull RecyclerView recyclerView, int newState) {
                super.onScrollStateChanged(mBLERecyclerView, newState);
            }

            @Override
            public void onScrolled(@NonNull RecyclerView recyclerView, int dx, int dy) {
                super.onScrolled(mBLERecyclerView, dx, dy);
                LinearLayoutManager layoutManager =LinearLayoutManager.class.cast(mBLERecyclerView.getLayoutManager());
            }
        });

        scanLeDevice(true);


    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // User chose not to enable Bluetooth.
        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }
        super.onActivityResult(requestCode, resultCode, data);
    }


    private void scanLeDevice(final boolean enable){
        List<ScanFilter> filters = new ArrayList<>();
        UUID myUUID = UUID.fromString("00000ffff-0000-1000-8000-00805f9b34fb");
        //MAXREFDES101 심박 측정 서비스만 스캔되도록 필터링 목록에 추가

        filter = new ScanFilter.Builder().setServiceUuid(new ParcelUuid(myUUID)).build();
        filters.add(filter);

        scanSettings = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_POWER).build();
        // 스캔 모드 설정
        // 현재 저전력 스캔 모드

        mBLEadapter.getBluetoothLeScanner().startScan(filters, scanSettings, mScanCallback);
        // 저전력 블루투스 스캔에 필요한
        // 필터링과 스캔모드, 스캔될때의 콜백매서드를 설정

        invalidateOptionsMenu();
    }

    // 저전력 블루투스 장치 스캔 콜백 매서드
    private ScanCallback mScanCallback = new ScanCallback() {
        @Override
        //정상적으로 스캔이 된 결과를 이용하여, 스캔 된 디바이스 목록들을 추가하고
        //추가 된 목록들을 리사이클러뷰(약간 list 형식과 비슷합니다)에 추가
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            mBLEDeviceAdapter.addDevice(result.getDevice());
            mBLEDeviceAdapter.notifyDataSetChanged();
            //Log.i("스캔 결과", "장치가 스캔되었습니다");
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            super.onBatchScanResults(results);
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
        }
    };

}
