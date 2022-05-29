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
    lateinit var chart: LineChart

    constructor(view: View) {
        creatGraph(view)
    }

    private fun creatGraph(view: View) {
        chart = view.findViewById<View>(R.id.LineChart) as LineChart
        chart.apply {
            setDrawGridBackground(false)
            setBackgroundColor(Color.WHITE)
            xAxis.isEnabled = false
            axisLeft.setDrawLabels(true)
            axisLeft.isEnabled = true
            axisRight.setDrawLabels(false)
            xAxis.setDrawLabels(true)
            xAxis.setDrawGridLines(false)
            xAxis.isEnabled = true
            xAxis.position = XAxis.XAxisPosition.BOTTOM
            axisRight.isEnabled = false
            setVisibleXRangeMaximum(20f)
            description.isEnabled = false
            isAutoScaleMinMaxEnabled = true
            invalidate()
        }

    }

    fun addEntry(ppg: PPGData){

        var num1 = ppg.grnCnt.toFloat()
        var num2 = ppg.grn2Cnt.toFloat()

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

        var set: ILineDataSet? = data.getDataSetByIndex(0)
        // set.addEntry(...); // can be called as well
        var set2: ILineDataSet? = data.getDataSetByIndex(1)
        if (set == null || set2 == null) {
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
        data.calcMinMaxY(data.xMin, data.xMax)


        chart.notifyDataSetChanged()
        chart.setVisibleXRangeMaximum(200f)
        chart.moveViewToX(data.entryCount.toFloat())

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

}