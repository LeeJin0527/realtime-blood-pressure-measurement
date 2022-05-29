package com.example.kotlin_ver

import android.Manifest
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattService
import android.content.*
import android.os.Bundle
import android.os.IBinder
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.FrameLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.kotlin_ver.ECG.ECGData
import com.example.kotlin_ver.ECG.FragmentECG
import com.example.kotlin_ver.PPG.FragmentPPG
import com.example.kotlin_ver.PPG.PPGData
import com.example.kotlin_ver.Temperature.FragmentTemp
import com.example.kotlin_ver.Temperature.TempData
import java.lang.StringBuilder

class MenuActivity : AppCompatActivity(){
    companion object{
        const val EXTRAS_DEVICE_NAME = "DEVICE_NAME"
        const val EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS"
    }

    private var mDeviceAddress: String? = null
    private var mBluetoothLeService: BluetoothLeService? = null

    private var writeCharacteristicObject: BluetoothGattCharacteristic? = null
    private var readCharacteristicObject: BluetoothGattCharacteristic? = null
    private var notifyCharacteristicObject: BluetoothGattCharacteristic? = null
    //read, write, notification을 위한 GATT characteristic

    var mode: String? = null
    //현재 temp, ppg, or ecg인지 알 수 있는 flag

    lateinit var fragmentTemp: FragmentTemp
    lateinit var fragmentPPG: FragmentPPG
    lateinit var fragmentECG: FragmentECG

    lateinit var fragmentContainerLayout: FrameLayout

    var AppAbsolutePath: String? = null

    private var tempCsvFile: CSVFile? = null
    private var ppgCsvFile: CSVFile? = null
    private var ecgCSVFile: CSVFile? = null

    private var mConnected = false

    private val mServiceConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(componentName: ComponentName, service: IBinder) {
            mBluetoothLeService = (service as BluetoothLeService.LocalBinder).getService()
            if (!mBluetoothLeService!!.initialize()) {
                Log.e("에러확인", "Unable to initialize Bluetooth")
                finish()
            }
            mBluetoothLeService!!.connect(mDeviceAddress)
        }

        override fun onServiceDisconnected(componentName: ComponentName) {
            mBluetoothLeService = null
        }
    }

    private val mGattUpdateReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
                mConnected = true
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                mConnected = false
            } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                setupGattCharacteristic(mBluetoothLeService!!.getSupportedGattServices())
            } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {

                val stringData = intent.getStringExtra(BluetoothLeService.EXTRA_DATA)
                val byteData = hexaStringToByteArray(stringData)

                when(mode){
                    "TEMP" -> {
                       fragmentTemp.addData(byteData)
                    }

                    "ECG" -> {
                        fragmentECG.addData(byteData)
                    }

                    "PPG" -> {
                        fragmentPPG.addData(byteData)
                    }
                }


            }
        }
    }

    fun hexaStringToByteArray(hexaString: String?): ByteArray {
        val length = hexaString!!.length
        val data = ByteArray(length / 2)
        var i = 0
        while (i < length) {
            data[i / 2] = ((Character.digit(hexaString[i], 16) shl 4)
                    + Character.digit(hexaString[i + 1], 16)).toByte()
            i += 2
        }
        return data
    }


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.menu)
        fragmentContainerLayout = findViewById(R.id.menu_fragment)

        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS)
        val gattServiceIntent = Intent(this, BluetoothLeService::class.java)
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE)
        //연결되는 기기에서 요구되는 GATT 특성이 포함된 BLE service binding 하기

        initializeViewComponent()
    }

    private fun initializeViewComponent(){
        // 버튼 initialize
        val tempbtn = findViewById<View>(R.id.tempbtn) as Button
        val ecgbtn = findViewById<View>(R.id.ecgbtn) as Button
        val ppgbtn = findViewById<View>(R.id.ppgbtn) as Button

        ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), MODE_PRIVATE) //사용자에게 권한 요구

        AppAbsolutePath = filesDir.toString()
        tempCsvFile = CSVFile("Temp")
        ppgCsvFile = CSVFile("PPG")
        ecgCSVFile = CSVFile("ECG")

        tempbtn.setOnClickListener {
            mode = "TEMP"
            val transaction = supportFragmentManager.beginTransaction()
            fragmentTemp = FragmentTemp()
            transaction.replace(R.id.menu_fragment, fragmentTemp)
            val frameLayout = findViewById<View>(R.id.menu_fragment) as FrameLayout
            frameLayout.visibility = View.VISIBLE
            transaction.addToBackStack(null)
            transaction.commit()
        }
        ecgbtn.setOnClickListener {
            mode = "ECG"
            val transaction = supportFragmentManager.beginTransaction()
            fragmentECG = FragmentECG()
            transaction.replace(R.id.menu_fragment, fragmentECG)
            val frameLayout = findViewById<View>(R.id.menu_fragment) as FrameLayout
            frameLayout.visibility = View.VISIBLE
            transaction.addToBackStack(null)
            transaction.commit()
        }
        ppgbtn.setOnClickListener {
            mode = "PPG"
            val transaction = supportFragmentManager.beginTransaction()
            fragmentPPG = FragmentPPG()
            transaction.replace(R.id.menu_fragment, fragmentPPG)
            val frameLayout = findViewById<View>(R.id.menu_fragment) as FrameLayout
            frameLayout.visibility = View.VISIBLE
            transaction.addToBackStack(null)
            transaction.commit()
        }
    }

    override fun onResume() {
        super.onResume()
        registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter())
        if (mBluetoothLeService != null) {
            val result = mBluetoothLeService!!.connect(mDeviceAddress)
            Log.d("에러 확인", "Connect request result=$result")
        }
    }

    override fun onPause() {
        super.onPause()
        unregisterReceiver(mGattUpdateReceiver)
    }

    override fun onDestroy() {
        super.onDestroy()
        unbindService(mServiceConnection)
        mBluetoothLeService = null
    }


    //특정 service들을 구성하는 characteristics를 구해주는 매소드
    private fun setupGattCharacteristic(gattServices: List<BluetoothGattService?>?) {
        if (gattServices == null) return
        for (gattService in gattServices) {
            val gattCharacteristics = gattService!!.characteristics
            for (gattCharacteristic in gattCharacteristics) {
                Log.i("GATT 확인", gattCharacteristic.uuid.toString())
                //notify
                if (gattCharacteristic.uuid.toString() == "00001011-1212-efde-1523-785feabcd123") {
                    notifyCharacteristicObject = gattCharacteristic
                }

                //read
                if (gattCharacteristic.uuid.toString() == "00001234-1212-efde-1523-785feabcd123") {
                    readCharacteristicObject = gattCharacteristic
                }
                if (gattCharacteristic.uuid.toString() == "00001027-1212-efde-1523-785feabcd123" || gattCharacteristic.uuid.toString() == "00007777-1212-efde-1523-785feabcd123") {
                    writeCharacteristicObject = gattCharacteristic
                }
            }
        }
    }

    private fun makeGattUpdateIntentFilter(): IntentFilter? {
        val intentFilter = IntentFilter()
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED)
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED)
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED)
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE)
        return intentFilter
    }


    fun sendStrCmd(cmd: ByteArray?) {
        writeCharacteristicObject!!.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
        mBluetoothLeService!!.writeCharacteristic(writeCharacteristicObject!!, cmd)
        mBluetoothLeService!!.setCharacteristicNotification(notifyCharacteristicObject!!, true)
    }

}