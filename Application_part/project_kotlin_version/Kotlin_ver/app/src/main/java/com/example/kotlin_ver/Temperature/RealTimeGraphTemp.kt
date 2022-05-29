package com.example.kotlin_ver.Temperature

import android.graphics.Color
import android.view.View
import com.example.kotlin_ver.R
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.components.YAxis
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet

class RealTimeGraphTemp {
    private var chart: LineChart? = null
    private var minX = 0f
    private var maxX = 0f

    constructor(view: View) {
        creatGraph(view)
    }

    private fun creatGraph(view: View) {
        chart = view.findViewById<View>(R.id.LineChart) as LineChart
        chart!!.setDrawGridBackground(true)
        chart!!.setBackgroundColor(Color.WHITE)
        chart!!.setGridBackgroundColor(Color.WHITE)
        chart!!.xAxis.isEnabled = false
        val leftAxis = chart!!.axisLeft
        leftAxis.isEnabled = true
        leftAxis.setDrawGridLines(true)
        val rightAxis = chart!!.axisRight
        rightAxis.isEnabled = false
        chart!!.axisLeft.setDrawLabels(true)
        chart!!.axisRight.setDrawLabels(false)
        chart!!.xAxis.setDrawLabels(false)
        chart!!.xAxis.setDrawGridLines(false)
        chart!!.description.isEnabled = false
        val pain = chart!!.renderer.paintRender
        chart!!.invalidate()
    }

    fun addEntry(temp: TempData){
        var num = temp.value
        var data = chart!!.data
        if (Math.abs(num.toFloat() - minX) > 2) {
            minX = num.toFloat() - 1.8.toFloat()
            chart!!.axisLeft.axisMinimum = minX
        }
        if (Math.abs(maxX - num.toFloat()) > 2) {
            maxX = num.toFloat() + 1.8.toFloat()
            chart!!.axisLeft.axisMaximum = maxX
        }
        if (data == null) {
            data = LineData()
            chart!!.data = data
        }
        var set = data.getDataSetByIndex(0)
        // set.addEntry(...); // can be called as well
        if (set == null) {
            set = createSet()
            data.addDataSet(set)
        }
        data.addEntry(
            Entry(
                set.entryCount.toFloat(),
                num.toFloat()
            ), 0
        )
        data.notifyDataChanged()

        chart!!.notifyDataSetChanged()
        chart!!.setVisibleXRangeMaximum(15f)

        chart!!.moveViewTo(data.entryCount.toFloat(), 50f, YAxis.AxisDependency.LEFT)

    }


    private fun createSet(): LineDataSet {
        val set = LineDataSet(null, "체온")
        set.setDrawFilled(true)
        set.fillColor = Color.RED
        set.lineWidth = 1f
        set.setDrawValues(false)
        set.mode = LineDataSet.Mode.LINEAR
        set.setDrawCircles(false)
        set.highLightColor = Color.RED
        set.color = Color.RED
        return set
    }

}