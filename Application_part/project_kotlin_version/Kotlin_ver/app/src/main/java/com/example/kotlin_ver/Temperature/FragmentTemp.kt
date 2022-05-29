package com.example.kotlin_ver.Temperature

import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.example.kotlin_ver.Command
import com.example.kotlin_ver.MenuActivity
import com.example.kotlin_ver.R
import java.io.File
import java.io.FileWriter
import java.io.IOException

class FragmentTemp : Fragment(){
    lateinit var start: Button
    lateinit var pause: Button
    lateinit var sensorField: TextView
    lateinit var sensor2Field: TextView
    lateinit var myGraph: RealTimeGraphTemp

    var activity: Activity? = null

    val tempDataList: ArrayList<TempData> = ArrayList()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragmenttemp, container, false)
        start = view.findViewById(R.id.start)
        pause = view.findViewById(R.id.pause)
        sensorField = view.findViewById(R.id.temp_sensor_value)
        sensor2Field = view.findViewById(R.id.sensor_Fahrenheit_scale_count)
        myGraph = RealTimeGraphTemp(view)

        //String CSVPath = ((MenuActivity)getActivity()).AppAbsolutePath;
        val CSVPath = "/Android/data/"
        val myFile = CSVPath + "test.csv"

        writeFile()
        //Log.i("경로", CSVPath);

        start.setOnClickListener {
            (activity as MenuActivity).sendStrCmd(Command.str_readtemp0)
            start.visibility = View.GONE
            pause.visibility = View.VISIBLE
        }
        pause!!.setOnClickListener {
            (activity as MenuActivity).sendStrCmd(Command.str_stop)
            start.visibility = View.VISIBLE
            pause.visibility = View.GONE
        }

        return view
    }




    override fun onAttach(context: Context) {
        super.onAttach(context)
        if (context is Activity) activity = context
    }

    override fun onDetach() {
        (getActivity() as MenuActivity).sendStrCmd(Command.str_stop)
        (getActivity() as MenuActivity).mode = "stop"
        (getActivity() as MenuActivity).fragmentContainerLayout.visibility = View.GONE
        super.onDetach()
    }

    fun addData(hexaData: ByteArray){
        val temp = TempData(hexaData)
        tempDataList.add(temp)

        sensorField.text = "${temp.value}"
        sensor2Field.text = "${temp.convertCelciustoF()}"

        getActivity()!!.runOnUiThread {
            myGraph.addEntry(temp)
        }

    }


    /**
     * Test method
     */
    fun writeFile() {
        val fileTitle = "testcsvfile.csv"
        val file = File(Environment.getExternalStorageDirectory(), fileTitle)
        Log.i("write 디렉토리 확인", Environment.getExternalStorageDirectory().absolutePath.toString())
        try {
            if (!file.exists()) {
                file.createNewFile()
            }
            val writer = FileWriter(file, false)
            writer.append("Temperature")
            writer.append(",")
            writer.append("count")
            writer.append("\n")
            writer.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

}