package com.example.kotlin_ver.ECG

import android.app.Activity
import android.content.Context
import android.os.Bundle
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
import com.google.android.material.internal.ContextUtils.getActivity

class FragmentECG : Fragment() {
    var start: Button? = null
    var pause: Button? = null

    var frameLayout: FrameLayout? = null
    var sensorField: TextView? = null
    lateinit var myGraph: RealTimeGraphECG
    var realTimeThread: Thread? = null
    var activity: Activity? = null
    var threadFlag: Boolean? = null

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
        val view : View = inflater.inflate(R.layout.fragmentecg, container, false)
        start = view!!.findViewById<View>(R.id.start) as Button
        pause = view!!.findViewById<View>(R.id.pause) as Button
        sensorField = view!!.findViewById<View>(R.id.ecg_rtor_value) as TextView
        myGraph = RealTimeGraphECG(view)
        threadFlag = false
        start!!.setOnClickListener {
            (activity as MenuActivity).setViewField(view, "ecg")
            (activity as MenuActivity).sendStrCmd(Command.str_readecg2)
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
        val layoutInflater = activity!!.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
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
        frameLayout!!.visibility = View.GONE
        if (realTimeThread != null) realTimeThread!!.interrupt()
        super.onDetach()
    }


    fun realTimeStart() {
        realTimeThread = Thread {
            while (threadFlag!!) {
                try {
                    Thread.sleep(90)
                } catch (e: InterruptedException) {
                    e.printStackTrace()
                }
                activity!!.runOnUiThread {
                    val ecgData: ECGData = (activity as MenuActivity).ecgData
                    myGraph.addEntry(ecgData.getEcg1().toDouble())
                    myGraph.addEntry(ecgData.getEcg2().toDouble())
                    myGraph.addEntry(ecgData.getEcg3().toDouble())
                    myGraph.addEntry(ecgData.getEcg4().toDouble())
                    //myGraph.addEntry(((MenuActivity) activity).valueForGraph);
                }
            }
        }
        realTimeThread!!.start()
    }
}