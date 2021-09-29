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
import android.widget.FrameLayout
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.example.kotlin_ver.Command
import com.example.kotlin_ver.MenuActivity
import com.example.kotlin_ver.R
import java.io.File
import java.io.FileWriter
import java.io.IOException

class FragmentTemp : Fragment(){
    var start: Button? = null
    var pause: Button? = null

    var frameLayout: FrameLayout? = null
    var sensorField: TextView? = null
    lateinit var myGraph: RealTimeGraphTemp
    var realTimeThread: Thread? = null
    var activity: Activity? = null
    var threadFlag: Boolean? = null

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragmenttemp, container, false)
        start = view!!.findViewById<View>(R.id.start) as Button
        pause = view!!.findViewById<View>(R.id.pause) as Button
        sensorField = view!!.findViewById<View>(R.id.temp_sensor_value) as TextView
        myGraph = RealTimeGraphTemp(view)

        //String CSVPath = ((MenuActivity)getActivity()).AppAbsolutePath;
        val CSVPath = "/Android/data/"
        val myFile = CSVPath + "test.csv"

/*
        try {
            FileWriter fw = new FileWriter(myFile);

            fw.append("count");
            fw.append(",");
            fw.append("Temperature");
            fw.append("\n");
            fw.flush();
            fw.close();
            Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("text/plain");
            intent.putExtra(intent.EXTRA_TITLE, myFile);
            startActivityForResult(intent, 1);
        }catch(IOException e){
            e.printStackTrace();
        }
*/writeFile()
        //Log.i("경로", CSVPath);
        threadFlag = false
        start!!.setOnClickListener {
            (activity as MenuActivity).setViewField(view, "temp")
            (activity as MenuActivity).sendStrCmd(Command.str_readtemp0)
            start!!.visibility = View.GONE
            pause!!.visibility = View.VISIBLE
            threadFlag = true
            realTimeStart()
        }
        pause!!.setOnClickListener {
            (activity as MenuActivity).setViewField(view, "stop")
            (activity as MenuActivity).sendStrCmd(Command.str_stop)
            start!!.visibility = View.VISIBLE
            pause!!.visibility = View.GONE
            threadFlag = false
            realTimeThread!!.interrupt()
        }
        val layoutInflater =
            activity!!.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        frameLayout = (activity as MenuActivity).getFrameLayout()
        return view
    }




    override fun onAttach(context: Context) {
        super.onAttach(context)
        if (context is Activity) activity = context
    }

    override fun onDetach() {
        (getActivity() as MenuActivity).sendStrCmd(Command.str_stop)
        (getActivity() as MenuActivity).mode = "stop"
        //stop추가 안하면, null point 에러남
        //stop 커맨드도 값이 바뀌는 경우라서
        //process_data 메서드가 호출되기 때문
        frameLayout!!.visibility = View.GONE
        if (realTimeThread != null) realTimeThread!!.interrupt()
        super.onDetach()
    }


    fun realTimeStart() {
        realTimeThread = Thread {
            while (threadFlag!!) {
                try {
                    Thread.sleep(1000)
                } catch (e: InterruptedException) {
                    e.printStackTrace()
                }
                activity!!.runOnUiThread { myGraph.addEntry((activity as MenuActivity).valueForGraph) }
            }
        }
        realTimeThread!!.start()
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