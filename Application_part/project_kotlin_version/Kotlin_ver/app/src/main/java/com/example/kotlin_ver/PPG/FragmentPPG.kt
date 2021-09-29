package com.example.kotlin_ver.PPG

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

class FragmentPPG : Fragment() {
    var start: Button? = null
    var pause: Button? = null

    var frameLayout: FrameLayout? = null
    var sensorField: TextView? = null
    lateinit var myGraph: RealTimeGraphPPG
    var realTimeThread: Thread? = null
    var activity: Activity? = null
    var threadFlag: Boolean? = null
    var initialStartFlag: Boolean? = null
    //ppg의 경우, 초기에 일정 인터벌 쉬었다가 그래프에 그려줘야 하는데, runOnUiThread를 우아하게 멈출 방법이 아직 생각이 안나서 임시방편으로 flag와 if문으로 인터벌 쉬도록 구현함

    //ppg의 경우, 초기에 일정 인터벌 쉬었다가 그래프에 그려줘야 하는데, runOnUiThread를 우아하게 멈출 방법이 아직 생각이 안나서 임시방편으로 flag와 if문으로 인터벌 쉬도록 구현함
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragmentppg, container, false)
        start = view!!.findViewById<View>(R.id.start) as Button
        pause = view!!.findViewById<View>(R.id.pause) as Button
        sensorField = view!!.findViewById<TextView>(R.id.temp_sensor_value)
        myGraph = RealTimeGraphPPG(view)
        realTimeThread = null
        threadFlag = false
        start!!.setOnClickListener {
            (activity as MenuActivity).setViewField(view, "ppg")
            (activity as MenuActivity).sendStrCmd(Command.str_readppg0)
            start!!.visibility = View.GONE
            pause!!.visibility = View.VISIBLE
            threadFlag = true
            initialStartFlag = true
            realTimeStart()
        }
        pause!!.setOnClickListener {
            (activity as MenuActivity).setViewField(view, "stop")
            (activity as MenuActivity).sendStrCmd(Command.str_stop)
            start!!.visibility = View.VISIBLE
            pause!!.visibility = View.GONE
            threadFlag = false
        }
        val layoutInflater =
            getActivity()!!.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        frameLayout = (activity as MenuActivity).getFrameLayout()
        return view
    }

    override fun onAttach(context: Context) {
        super.onAttach(context)
        if (context is Activity) activity = context
    }

    override fun onDetach() {
        (activity as MenuActivity).sendStrCmd(Command.str_stop)
        (activity as MenuActivity).mode = "stop"
        frameLayout!!.visibility = View.GONE
        super.onDetach()
    }


    fun realTimeStart() {
        realTimeThread = Thread {
            while (threadFlag!!) {
                try {
                    if (initialStartFlag!!) {
                        Thread.sleep(10000)
                        initialStartFlag = false
                    }
                    Thread.sleep(100)
                } catch (e: InterruptedException) {
                    e.printStackTrace()
                }
                activity!!.runOnUiThread {
                    myGraph.addEntry(
                        (activity as MenuActivity).ppgData.getGrnCnt().toDouble(),
                        (activity as MenuActivity).ppgData.getGrn2Cnt().toDouble()
                    )
                }
            }
        }
        realTimeThread!!.start()
    }
}