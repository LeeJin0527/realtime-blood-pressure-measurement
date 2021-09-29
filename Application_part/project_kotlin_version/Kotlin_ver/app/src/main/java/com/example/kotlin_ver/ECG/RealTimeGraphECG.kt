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
    private val minX = 0f
    private val maxX = 0f

    constructor(view : View) : this() {
        creatGraph(view)
    }

    private fun creatGraph(view: View) {
        chart = view.findViewById<View>(R.id.LineChart) as LineChart
        chart.setDrawGridBackground(false)
        chart.setBackgroundColor(Color.WHITE)
        chart.getAxisLeft().setDrawLabels(true)
        chart.getAxisLeft().setEnabled(true)
        chart.getAxisRight().setDrawLabels(false)
        chart.getXAxis().setDrawLabels(true)
        chart.getXAxis().setDrawGridLines(false)
        chart.getXAxis().setEnabled(true)
        chart.getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM)
        chart.getAxisRight().setEnabled(false)
        chart.getDescription().setEnabled(false)
        chart.invalidate()
    }

    fun addEntry(num: Double) {
        var data: LineData? = chart.getData()
        chart.getAxisLeft().setAxisMaximum(2000f)
        chart.getAxisLeft().setAxisMinimum(-2000f)
        if (data == null) {
            data = LineData()
            chart.setData(data)
        }
        var set: ILineDataSet = data.getDataSetByIndex(0)
        // set.addEntry(...); // can be called as well
        if (set == null) {
            set = createSet()
            data.addDataSet(set)
        }
        data.addEntry(Entry(set.getEntryCount() as Float, num.toFloat()), 0)
        data.notifyDataChanged()

        // let the chart know it's data has changed
        chart.notifyDataSetChanged()
        chart.setVisibleXRangeMaximum(130F)
        // this automatically refreshes the chart (calls invalidate())
        chart.moveViewTo(data.getEntryCount().toFloat(), 100f, YAxis.AxisDependency.LEFT)
        //chart.moveViewToX(data.getEntryCount());
        //chart.moveViewTo(data.getEntryCount(), 50f, YAxis.AxisDependency.RIGHT);
        //chart.invalidate();
    }

    fun resetMaxandMin() {
        Log.i("호출 : ", "호출됨!")
        chart.getAxisLeft().resetAxisMaximum()
        chart.getAxisLeft().resetAxisMinimum()
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