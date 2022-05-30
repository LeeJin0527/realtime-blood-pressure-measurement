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
    lateinit var start: Button
    lateinit var pause: Button
    lateinit var ppgValueField: TextView
    lateinit var confidenceField: TextView
    lateinit var activityStateField: TextView
    lateinit var myGraph: RealTimeGraphPPG

    var activity: Activity? = null

    val ppgDataList: ArrayList<PPGData> = ArrayList()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragmentppg, container, false)
        start = view.findViewById(R.id.start)
        pause = view.findViewById(R.id.pause)
        ppgValueField = view.findViewById(R.id.ppg_sensor_value)
        confidenceField = view.findViewById(R.id.ppg_sensor_confidence_value)
        activityStateField = view.findViewById(R.id.ppg_sensor_activity_value)

        myGraph = RealTimeGraphPPG(view)

        start.setOnClickListener {
            (activity as MenuActivity).sendStrCmd(Command.str_readppg0)
            (activity as MenuActivity).mode = "PPG"
            start.visibility = View.GONE
            pause.visibility = View.VISIBLE
        }
        pause.setOnClickListener {
            (activity as MenuActivity).sendStrCmd(Command.str_stop)
            (activity as MenuActivity).mode = "stop"
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
        (activity as MenuActivity).sendStrCmd(Command.str_stop)
        (getActivity() as MenuActivity).mode = "stop"
        (getActivity() as MenuActivity).fragmentContainerLayout.visibility = View.GONE
        super.onDetach()
    }

    fun addData(hexaData: ByteArray){
        val ppg = PPGData(hexaData)
        ppgDataList.add(ppg)

        ppgValueField.text = "${ppg.heartRate}"
        confidenceField.text = "${ppg.heartRateConfidence}"
        activityStateField.text = "${ppg.ppgActivityString}"

        activity!!.runOnUiThread {
            myGraph.addEntry(ppg)
        }
    }


}