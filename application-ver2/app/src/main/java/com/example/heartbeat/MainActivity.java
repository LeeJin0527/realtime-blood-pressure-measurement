/**
 * application이 처음으로 구동 될 때 시작될 activity입니다.
 * scanning된 device목록을 리스트 형식으로 보여줍니다.
 *
 * todo : 특정 device만 scanning되도록 하는게 좋을듯(service uuid로 filter?)
 *
 *
 */
package com.example.heartbeat;

import android.app.Activity;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.ScanFilter;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelUuid;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import androidx.annotation.Nullable;

import java.util.ArrayList;

public class MainActivity extends ListActivity{
    private BluetoothAdapter mBLEadapter;
    private boolean mScanning;
    private Handler mHandler;
    private BLEDeviceListAdapter mBLEDeviceAdapter;

    private static final int REQUEST_ENABLE_BT = 1;

    private static final long SCAN_PERIOD = 15000;
    //15초 뒤에 스캔 중지

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mHandler = new Handler();

        //해당 기기에서 BLE를 지원하는지 확인
        //지원하지 않는 다면 해당 activity 종료
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


    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu){
        getMenuInflater().inflate(R.menu.main, menu);
        if (!mScanning) {
            menu.findItem(R.id.menu_stop).setVisible(false);
            menu.findItem(R.id.menu_scan).setVisible(true);
            menu.findItem(R.id.menu_refresh).setActionView(null);
        } else {
            menu.findItem(R.id.menu_stop).setVisible(true);
            menu.findItem(R.id.menu_scan).setVisible(false);
            menu.findItem(R.id.menu_refresh).setActionView(
                    R.layout.actionbar_indeterminate_progress);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item){
        switch(item.getItemId()){
            case R.id.menu_scan:
                System.out.println("press menu scan!");
                mBLEDeviceAdapter.clear();
                scanLeDevice(true);
                break;
            case R.id.menu_stop:
                System.out.println("press menu stop!");
                scanLeDevice(false);
                break;
        }
        return true;
    }



    @Override
    protected void onResume(){
        super.onResume();
        // Ensures Bluetooth is enabled on the device.  If Bluetooth is not currently enabled,
        // fire an intent to display a dialog asking the user to grant permission to enable it.
        if (!mBLEadapter.isEnabled()) {
            if (!mBLEadapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
        }

        // Initializes list view adapter.

        mBLEDeviceAdapter = new BLEDeviceListAdapter();
        setListAdapter(mBLEDeviceAdapter);
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
    //todo : 이건 뭔지 모르겟음

    @Override
    // 해당 list의 BLE device를 클릭하면 온도, ppg, ecg를 처리할 activity로 이동
    protected void onListItemClick(ListView l, View v, int position, long id) {
        final BluetoothDevice device = mBLEDeviceAdapter.getDevice(position);
        if (device == null) return;
        //final Intent intent = new Intent(this, BLEControlActivity.class);
        final Intent intent = new Intent(this, MenuActivity.class);
        intent.putExtra(BLEControlActivity.EXTRAS_DEVICE_NAME, device.getName());
        intent.putExtra(BLEControlActivity.EXTRAS_DEVICE_ADDRESS, device.getAddress());
        if (mScanning) {
            mBLEadapter.stopLeScan(mLeScanCallback);
            mScanning = false;
        }
        startActivity(intent);
    }


    private void scanLeDevice(final boolean enable){
        if(enable){
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mScanning = false;
                    mBLEadapter.stopLeScan(mLeScanCallback);
                    invalidateOptionsMenu();
                }
            }, SCAN_PERIOD);

            mScanning = true;
            mBLEadapter.startLeScan(mLeScanCallback);
        }
        invalidateOptionsMenu();
    }



    private class BLEDeviceListAdapter extends  BaseAdapter{
        private ArrayList<BluetoothDevice> BLEDevices;
        private LayoutInflater Inflator;

        public BLEDeviceListAdapter(){
            super();
            BLEDevices = new ArrayList<BluetoothDevice>();
            Inflator = MainActivity.this.getLayoutInflater();
        }

        public void addDevice(BluetoothDevice device){
            if(!BLEDevices.contains(device)){
                BLEDevices.add(device);
            }
        }

        public BluetoothDevice getDevice(int position){
            return BLEDevices.get(position);
        }

        public void clear(){
            BLEDevices.clear();
        }

        @Override
        public int getCount() {
            return BLEDevices.size();
        }

        @Override
        public Object getItem(int position) {
            return BLEDevices.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder viewHolder;

            if(convertView == null){
                convertView = Inflator.inflate(R.layout.listitem_device, null);
                viewHolder = new ViewHolder();
                viewHolder.deviceMacAddr = (TextView) convertView.findViewById(R.id.device_address);
                viewHolder.deviceName = (TextView) convertView.findViewById(R.id.device_name);
                convertView.setTag(viewHolder);
            }
            else{
                viewHolder = (ViewHolder) convertView.getTag();
            }

            BluetoothDevice device = BLEDevices.get(position);
            final String deviceName = device.getName();
            if(deviceName != null && deviceName.length() > 0)
                viewHolder.deviceName.setText(deviceName);
            else
                viewHolder.deviceName.setText("식별할 수 없는 기기");
            viewHolder.deviceMacAddr.setText(device.getAddress());

            return convertView;
        }
    }

    static class ViewHolder{
        TextView deviceName;
        TextView deviceMacAddr;
    }

    // Device scan callback.
    private BluetoothAdapter.LeScanCallback mLeScanCallback =
            new BluetoothAdapter.LeScanCallback() {

                @Override
                public void onLeScan(final BluetoothDevice device, int rssi, byte[] scanRecord) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mBLEDeviceAdapter.addDevice(device);
                            mBLEDeviceAdapter.notifyDataSetChanged();
                        }
                    });
                }
            };
}
