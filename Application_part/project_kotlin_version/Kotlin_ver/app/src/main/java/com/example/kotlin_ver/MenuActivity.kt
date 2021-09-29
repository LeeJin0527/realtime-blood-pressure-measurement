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

    //public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
    val EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS"

    private var mDeviceAddress: String? = null

    private var mBluetoothLeService: BluetoothLeService? = null

    private var writeCharacteristicObject: BluetoothGattCharacteristic? = null
    private var readCharacteristicObject: BluetoothGattCharacteristic? = null
    private var notifyCharacteristicObject: BluetoothGattCharacteristic? = null
    //read, write, notify를 위한 GATT characteristic

    //read, write, notify를 위한 GATT characteristic
    var mode: String? = null
    //현재 temp, ppg, or ecg인지 알 수 있는 flag

    //현재 temp, ppg, or ecg인지 알 수 있는 flag
    private var tempField: TextView? = null
    private var tempFsField: TextView? = null

    private val ecgCountField: TextView? = null
    private var ecgRtoRField: TextView? = null
    private var ecgRtoRBpmField: TextView? = null
    private val ecg1Field: TextView? = null
    private val ecg2Field: TextView? = null
    private val ecg3Field: TextView? = null

    private var ppgField: TextView? = null
    private var ppgConfidenceField: TextView? = null
    private var ppgActivityField: TextView? = null

    lateinit var tempData: TempData
    lateinit var ecgData: ECGData
    lateinit var ppgData: PPGData

    var AppAbsolutePath: String? = null

    var valueForGraph = 0.0

    private var tempCsvFile: CSVFile? = null
    private var ppgCsvFile: CSVFile? = null
    var ecgCSVFile: CSVFile? = null


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
                //Log.i(TAG, intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
                val tmp_data = intent.getStringExtra(BluetoothLeService.EXTRA_DATA)
                val hexaData = hexaStringToByteArray(tmp_data)
                val sb = StringBuilder()
                for (tmp in hexaData) {
                    sb.append(String.format("%02x", tmp.toInt() and 0xff))
                }
                //Log.i("원본", tmp_data);
                Log.i("변환", sb.toString())
                process_data(hexaData)
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
        val intent = intent
        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS)
        val gattServiceIntent = Intent(this, BluetoothLeService::class.java)
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE)
        //bindService 메서드가 호출되면 onServiceConnected 호출되는거 같음

        // 버튼 initialize
        val tempbtn = findViewById<View>(R.id.tempbtn) as Button
        val ecgbtn = findViewById<View>(R.id.ecgbtn) as Button
        val ppgbtn = findViewById<View>(R.id.ppgbtn) as Button
        Log.i("path 확인용", filesDir.toString())
        ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), MODE_PRIVATE) //사용자에게 권한 요구
        AppAbsolutePath = filesDir.toString()
        tempCsvFile = CSVFile("Temp")
        ppgCsvFile = CSVFile("PPG")
        ecgCSVFile = CSVFile("ECG")


        // 데이터 핸들링 객체 initialize
        tempData = TempData()
        ppgData = PPGData()
        ecgData = ECGData()
        tempbtn.setOnClickListener {
            val transaction = supportFragmentManager.beginTransaction()
            val fragmentTemp = FragmentTemp()
            transaction.replace(R.id.menu_fragment, fragmentTemp)
            val frameLayout = findViewById<View>(R.id.menu_fragment) as FrameLayout
            frameLayout.visibility = View.VISIBLE
            transaction.addToBackStack(null)
            transaction.commit()
        }
        ecgbtn.setOnClickListener {
            val transaction = supportFragmentManager.beginTransaction()
            val fragmentECG = FragmentECG()
            transaction.replace(R.id.menu_fragment, fragmentECG)
            val frameLayout = findViewById<View>(R.id.menu_fragment) as FrameLayout
            frameLayout.visibility = View.VISIBLE
            transaction.addToBackStack(null)
            transaction.commit()
        }
        ppgbtn.setOnClickListener {
            val transaction = supportFragmentManager.beginTransaction()
            val fragmentPPG = FragmentPPG()
            transaction.replace(R.id.menu_fragment, fragmentPPG)
            val frameLayout = findViewById<View>(R.id.menu_fragment) as FrameLayout
            frameLayout.visibility = View.VISIBLE
            transaction.addToBackStack(null)
            //뒤로가기
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
                    Log.i("적용됨?", "됏음")
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

    private fun btnView(choice: Boolean) {
        val tempbtn = findViewById<View>(R.id.tempbtn) as Button
        val ecgbtn = findViewById<View>(R.id.ecgbtn) as Button
        val ppgbtn = findViewById<View>(R.id.ppgbtn) as Button
        if (choice == true) {
            tempbtn.visibility = View.VISIBLE
            ecgbtn.visibility = View.VISIBLE
            ppgbtn.visibility = View.VISIBLE
        }
        if (choice == false) {
            tempbtn.visibility = View.INVISIBLE
            ecgbtn.visibility = View.INVISIBLE
            ppgbtn.visibility = View.INVISIBLE
        }
    }

    interface onKeyBack {
        fun onBackKey()
    }

    fun getFrameLayout(): FrameLayout? {
        return findViewById<View>(R.id.menu_fragment) as FrameLayout
    }

    fun sendStrCmd(cmd: ByteArray?) {
        writeCharacteristicObject!!.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
        mBluetoothLeService!!.writeCharacteristic(writeCharacteristicObject!!, cmd)
        mBluetoothLeService!!.setCharacteristicNotification(notifyCharacteristicObject!!, true)
    }


    fun process_data(data: ByteArray) {
        if (mode == "temp") {
            tempData.setPacket(data)
            valueForGraph = tempData.getValue()
            tempField!!.setText(java.lang.String.valueOf(tempData.getValue()))
            tempFsField!!.text = tempData.convertCelciustoF().toString()
            tempCsvFile!!.writeTempFile(tempData)
        } else if (mode == "ecg") {
            ecgData.setPacket(data)
            valueForGraph = ecgData.getTest_value_graph().toDouble()
            ecgRtoRField!!.setText(java.lang.String.valueOf(ecgData.getRtoR()))
            ecgRtoRBpmField!!.setText(java.lang.String.valueOf(ecgData.getRtoRBpm()))
            ecgCSVFile!!.writeECGFile(ecgData)
        } else if (mode == "ppg") {
            ppgData.setPacket(data)
            valueForGraph = ppgData.getGrnCnt().toDouble()
            ppgField!!.setText(java.lang.String.valueOf(ppgData.getHeartRate()))
            ppgConfidenceField!!.setText(java.lang.String.valueOf(ppgData.getHeartRateConfidence()).toString() + "%")
            if (ppgData.getPpgActivity() === 0) {
                ppgActivityField!!.text = "rest"
            }
            if (ppgData.getPpgActivity() === 1) {
                ppgActivityField!!.text = "Non-rhythmic activity"
            }
            if (ppgData.getPpgActivity() === 2) {
                ppgActivityField!!.text = "walking"
            }
            if (ppgData.getPpgActivity() === 3) {
                ppgActivityField!!.text = "running"
            }
            if (ppgData.getPpgActivity() === 4) {
                ppgActivityField!!.text = "biking"
            }
            if (ppgData.getPpgActivity() === 5) {
                ppgActivityField!!.text = "rhythmic activity"
            }
            ppgCsvFile!!.writePPGFile(ppgData)
        } else if (mode == "stop") {
            return
        }
    }

    fun setViewField(view: View, modeConfig: String?) {
        mode = modeConfig
        if (mode == "temp") {
            tempField = view.findViewById<View>(R.id.temp_sensor_value) as TextView
            tempFsField = view.findViewById<View>(R.id.sensor_Fahrenheit_scale_count) as TextView
        }
        if (mode == "ppg") {
            ppgField = view.findViewById<View>(R.id.ppg_sensor_value) as TextView
            ppgConfidenceField = view.findViewById<View>(R.id.ppg_sensor_confidence_value) as TextView
            ppgActivityField = view.findViewById<View>(R.id.ppg_sensor_activity_value) as TextView
        }
        if (mode == "ecg") {
            ecgRtoRField = view.findViewById<View>(R.id.ecg_rtor_value) as TextView
            ecgRtoRBpmField = view.findViewById<View>(R.id.ecg_rtorbpm_value) as TextView
        }
    }
}