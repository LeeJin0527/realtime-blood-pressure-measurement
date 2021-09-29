package com.example.kotlin_ver.PPG

import android.graphics.Color
import android.util.Log
import android.view.View
import com.example.kotlin_ver.R
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.components.XAxis
import com.github.mikephil.charting.components.YAxis
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet

class RealTimeGraphPPG {
    private val TAG = RealTimeGraphPPG::class.java.simpleName
    private var chart: LineChart? = null
    private val minX = 0f
    private val maxX = 0f
    private var count = 0

    constructor(view: View) {
        creatGraph(view)
    }

    private fun creatGraph(view: View) {
        chart = view.findViewById<View>(R.id.LineChart) as LineChart
        chart!!.setDrawGridBackground(false)
        chart!!.setBackgroundColor(Color.WHITE)
        chart!!.xAxis.isEnabled = false
        chart!!.axisLeft.setDrawLabels(true)
        chart!!.axisLeft.isEnabled = true
        chart!!.axisRight.setDrawLabels(false)
        chart!!.xAxis.setDrawLabels(true)
        chart!!.xAxis.setDrawGridLines(false)
        chart!!.xAxis.isEnabled = true
        chart!!.xAxis.position = XAxis.XAxisPosition.BOTTOM
        chart!!.axisRight.isEnabled = false
        chart!!.setVisibleXRangeMaximum(20f)
        chart!!.description.isEnabled = false
        chart!!.invalidate()
    }

    fun addEntry(num1: Double, num2: Double) {
        var num1 = num1
        var num2 = num2
        var data = chart!!.data
        val min: Float
        var max: Float
        val diff: Float
        Log.i(TAG, "ch1 = $num1")
        Log.i(TAG, "ch2 = $num2")
        if (num1 > num2) {
            max = num1.toFloat()
            min = num2.toFloat()
            diff = (max - min) * 0.01f
            num1 = num2 + diff
            max = num1.toFloat()
        } else {
            max = num2.toFloat()
            min = num1.toFloat()
            diff = (max - min) * 0.01f
            num2 = num1 + diff
            max = num2.toFloat()
        }


        //chart.getAxisLeft().setAxisMaximum(max + 1000);
        //chart.getAxisLeft().setAxisMinimum(min - 1000);
        if (data == null) {
            data = LineData()
            chart!!.data = data
        }

        var set = data.getDataSetByIndex(0)
        // set.addEntry(...); // can be called as well
        var set2 = data.getDataSetByIndex(1)
        if (set == null) {
            set = createSet()
            data.addDataSet(set)
            set2 = createSet2()
            data.addDataSet(set2)
        }
        data.addEntry(
            Entry(
                set.entryCount.toFloat(),
                num1.toFloat()
            ), 0
        )
        data.addEntry(
            Entry(
                set2.entryCount.toFloat(),
                num2.toFloat()
            ), 1
        )
        data.notifyDataChanged()

        // let the chart know it's data has changed
        chart!!.notifyDataSetChanged()
        //count++;
        //if(count >= 75)yAxiSetting();
        chart!!.setVisibleXRangeMaximum(120f)
        // this automatically refreshes the chart (calls invalidate())
        chart!!.moveViewTo(data.entryCount.toFloat(), 50f, YAxis.AxisDependency.LEFT)
    }

    private fun createSet(): LineDataSet {
        val set = LineDataSet(null, "ch1")
        set.lineWidth = 1f
        set.setDrawValues(false)
        set.mode = LineDataSet.Mode.LINEAR
        set.setDrawCircles(false)
        set.highLightColor = Color.GREEN
        set.color = Color.GREEN
        return set
    }

    private fun createSet2(): LineDataSet {
        val set = LineDataSet(null, "ch2")
        set.lineWidth = 1f
        set.setDrawValues(false)
        set.mode = LineDataSet.Mode.LINEAR
        set.setDrawCircles(false)
        set.highLightColor = Color.BLUE
        set.color = Color.BLUE
        return set
    }

    private fun yAxiSetting() {
        count = 0
    }
}