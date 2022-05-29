package com.example.kotlin_ver.ECG

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

class RealTimeGraphECG() {
    lateinit var chart: LineChart

    constructor(view : View) : this() {
        creatGraph(view)
    }

    private fun creatGraph(view: View) {
        chart = view.findViewById<View>(R.id.LineChart) as LineChart
        chart.apply {
            setDrawGridBackground(false)
            setBackgroundColor(Color.WHITE)
            getAxisLeft().setDrawLabels(true)
            getAxisLeft().setEnabled(true)
            getAxisRight().setDrawLabels(false)
            getXAxis().setDrawLabels(true)
            getXAxis().setDrawGridLines(false)
            getXAxis().setEnabled(true)
            getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM)
            getAxisRight().setEnabled(false)
            getDescription().setEnabled(false)
            xAxis.mAxisMinimum = 0f
            xAxis.mAxisMaximum = 200f
            isAutoScaleMinMaxEnabled = true
            invalidate()
        }

    }

    fun addEntry(ecg: ECGData){
        var data: LineData? = chart.data

        if(data == null){
            data = LineData()
            chart.data = data
        }

        var set: ILineDataSet? = data.getDataSetByIndex(0)
        if(set == null){
            set = createSet()
            data.addDataSet(set)
        }

        data.addEntry(Entry(set.entryCount.toFloat(), ecg.ecg1.toFloat()), 0)
        data.addEntry(Entry(set.entryCount.toFloat(), ecg.ecg2.toFloat()), 0)
        data.addEntry(Entry(set.entryCount.toFloat(), ecg.ecg3.toFloat()), 0)
        data.addEntry(Entry(set.entryCount.toFloat(), ecg.ecg4.toFloat()), 0)
        data.notifyDataChanged()

        if(data.entryCount >= 200f){

            chart.xAxis.mAxisMinimum = data.entryCount - 200f
            chart.xAxis.mAxisMaximum = data.entryCount.toFloat()
        }
        data.calcMinMaxY(data.xMin, data.xMax)

        chart.notifyDataSetChanged()
        chart.setVisibleXRangeMaximum(200f)
        chart.moveViewToX(data.entryCount.toFloat())
    }


    private fun createSet(): LineDataSet {
        val set = LineDataSet(null, "ECG")
        set.setDrawFilled(false)
        set.setFillColor(Color.RED)
        set.setLineWidth(1f)
        set.setDrawValues(false)
        set.setMode(LineDataSet.Mode.LINEAR)
        set.setDrawCircles(true)
        set.setHighLightColor(Color.RED)
        set.setColor(Color.RED)
        return set
    }
}