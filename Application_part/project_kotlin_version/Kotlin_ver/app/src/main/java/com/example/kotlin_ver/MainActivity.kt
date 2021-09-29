package com.example.kotlin_ver

import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothAdapter.LeScanCallback
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.os.ParcelUuid
import android.util.Log
import android.view.Menu
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.PagerSnapHelper
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.SnapHelper
import java.util.*
import kotlin.collections.ArrayList
import kotlin.properties.Delegates

class MainActivity : AppCompatActivity(){
    private var mScanning = true
    private lateinit var mBLEadapter : BluetoothAdapter
    private lateinit var mBLERecyclerView : RecyclerView
    private lateinit var mBLEList : ArrayList<BluetoothDevice>
    private lateinit var mBLEDeviceAdapter : BLEDataAdapter
    private val REQUEST_ENABLE_BT = 1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        mBLEadapter = bluetoothManager.adapter

        //BLE 및 BL service를 지원하는 기기인지 확인
        if(!packageManager.hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE) || mBLEadapter == null){
            Toast.makeText(this, "BLE를 지원하지 않는 기기 입니다. 어플리케이션이 종료됩니다.", Toast.LENGTH_SHORT).show()
            finish()
        }

        setContentView(R.layout.mainactivity_recyclerview)
        mBLERecyclerView = findViewById(R.id.recyclerview)

    }

    override fun onResume() {
        super.onResume()

        // activity가 resume 될때마다, 사용자가 BL 기능을 off 했는지 확인해서, off 되어 있다면 on 시키도록 intent를 날려줌
        if(!mBLEadapter.isEnabled){
            val enableBluetoothIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            startActivityForResult(enableBluetoothIntent, REQUEST_ENABLE_BT)
        }

        mBLEList = ArrayList<BluetoothDevice>()

        // initialize recycler view adapter and onClickListener for ble device list
        mBLEDeviceAdapter = BLEDataAdapter(mBLEList)
        mBLEDeviceAdapter.itemClickListener = object : BLEDataAdapter.ItemClickListener{
            override fun onItemClick(view: View?, position: Int) {
                Log.i("확인용", position.toString())
                val BLEDeviceName: String = mBLEList.get(position).getName()
                val BLEDeviceMAC: String = mBLEList.get(position).getAddress()

                val intent = Intent(view!!.context, MenuActivity::class.java)
                intent.putExtra(MenuActivity.EXTRAS_DEVICE_NAME, BLEDeviceName)
                intent.putExtra(MenuActivity.EXTRAS_DEVICE_ADDRESS, BLEDeviceMAC)
                if (mScanning) {
                    mBLEadapter.bluetoothLeScanner.stopScan(mScanCallback)
                    mScanning = false
                }
                startActivity(intent)
            }
        }
        mBLERecyclerView.adapter = mBLEDeviceAdapter

        mBLERecyclerView.layoutManager = LinearLayoutManager(this, RecyclerView.HORIZONTAL, false)

        val snapHelper: SnapHelper = PagerSnapHelper()
        if (mBLERecyclerView.onFlingListener == null) snapHelper.attachToRecyclerView(
            mBLERecyclerView
        )

        mBLERecyclerView.addOnScrollListener(object : RecyclerView.OnScrollListener() {
            override fun onScrollStateChanged(recyclerView: RecyclerView, newState: Int) {
                super.onScrollStateChanged(mBLERecyclerView, newState)
            }

            override fun onScrolled(recyclerView: RecyclerView, dx: Int, dy: Int) {
                super.onScrolled(mBLERecyclerView, dx, dy)
                val layoutManager =
                    LinearLayoutManager::class.java.cast(mBLERecyclerView.layoutManager)
            }
        })

        scanLeDevice(true)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if(requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED){
            finish()
        }
        super.onActivityResult(requestCode, resultCode, data)
    }

    //todo : 이건 뭔지 모르겟음
    private fun scanLeDevice(enable: Boolean) {
        val filters: MutableList<ScanFilter> = java.util.ArrayList()
        val myUUID = UUID.fromString("00000ffff-0000-1000-8000-00805f9b34fb")
        val filter = ScanFilter.Builder().setServiceUuid(ParcelUuid(myUUID)).build()
        //filter = new ScanFilter.Builder().setDeviceAddress("80:6F:B0:90:02:3C").build();
        //filter = new ScanFilter.Builder().setDeviceName("HSP2_2.7.0.1").build();
        filters.add(filter)
        val scanSettings = ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_POWER).build()
        /*
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
            //mBLEadapter.startLeScan(mLeScanCallback);
            //mBLEadapter.getBluetoothLeScanner().startScan(filters, scanSettings, mScanCallback);

        }

 */

        mBLEadapter.bluetoothLeScanner.startScan(filters, scanSettings, mScanCallback)
        //mBLEadapter.getBluetoothLeScanner().startScan(mScanCallback);
        invalidateOptionsMenu()
    }
    val mScanCallback: ScanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            super.onScanResult(callbackType, result)
            mBLEDeviceAdapter.addDevice(result.device)
            mBLEDeviceAdapter.notifyDataSetChanged()
            Log.i("확인용", "밖에 호출!")
        }

        override fun onBatchScanResults(results: List<ScanResult>) {
            Log.i("확인용", "배치 호출!")
            super.onBatchScanResults(results)
        }

        override fun onScanFailed(errorCode: Int) {
            Log.i("확인용", "에러 호출!")
            Log.i("확인용", errorCode.toString())
            super.onScanFailed(errorCode)
        }
    }



}