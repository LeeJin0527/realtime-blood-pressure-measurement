package com.example.kotlin_ver.ECG

import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.Menu
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
import kotlin.collections.ArrayList

class FragmentECG : Fragment() {
    lateinit var start: Button
    lateinit var pause: Button
    lateinit var sensorField: TextView
    lateinit var rtorField: TextView
    lateinit var bpmField: TextView
    lateinit var myGraph: RealTimeGraphECG

    var activity: Activity? = null

    val ecgDataList: ArrayList<ECGData> = ArrayList()

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
        val view : View = inflater.inflate(R.layout.fragmentecg, container, false)
        start = view.findViewById(R.id.start)
        pause = view.findViewById(R.id.pause)
        sensorField = view.findViewById(R.id.ecg_rtor_value) as TextView
        rtorField = view.findViewById(R.id.ecg_rtor_value)
        bpmField = view.findViewById(R.id.ecg_rtorbpm_value)

        myGraph = RealTimeGraphECG(view)

        start.setOnClickListener {
            (activity as MenuActivity).sendStrCmd(Command.str_readecg2)
            start.visibility = View.GONE
            pause.visibility = View.VISIBLE
        }

        pause.setOnClickListener {
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
        val ecg = ECGData(hexaData)
        ecgDataList.add(ecg)

        rtorField.text = "${ecg.RtoR}"
        bpmField.text = "${ecg.RtoRBpm}"

        activity!!.runOnUiThread {
            myGraph.addEntry(ecg)
        }

    }

}