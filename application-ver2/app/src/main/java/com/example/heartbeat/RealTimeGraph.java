package com.example.heartbeat;

import android.graphics.Color;
import android.util.Log;
import android.view.View;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;

public class RealTimeGraph {
    private LineChart chart;

    public RealTimeGraph(View view){
        creatGraph(view);
    }

    private void creatGraph(View view){
        chart = (LineChart)view.findViewById(R.id.LineChart);

        chart.setDrawGridBackground(true);
        chart.setBackgroundColor(Color.BLACK);
        chart.setGridBackgroundColor(Color.BLACK);

        chart.getXAxis().setDrawGridLines(true);
        chart.getXAxis().setDrawAxisLine(false);

        chart.getXAxis().setEnabled(true);
        chart.getXAxis().setDrawGridLines(false);

        YAxis leftAxis = chart.getAxisLeft();
        leftAxis.setEnabled(true);
        //leftAxis.setTextColor(getResources().getColor(R.color.colorGrid));
        leftAxis.setDrawGridLines(true);
        //leftAxis.setGridColor(getResources().getColor(R.color.colorGrid));

        YAxis rightAxis = chart.getAxisRight();
        rightAxis.setEnabled(false);

        chart.invalidate();


    }
    public void addEntry(double num) {
        LineData data = chart.getData();

        if (data == null) {
            data = new LineData();
            chart.setData(data);
        }

        ILineDataSet set = data.getDataSetByIndex(0);
        // set.addEntry(...); // can be called as well

        if (set == null) {
            set = createSet();
            data.addDataSet(set);
        }



        data.addEntry(new Entry((float)set.getEntryCount(), (float)num), 0);
        data.notifyDataChanged();

        // let the chart know it's data has changed
        chart.notifyDataSetChanged();

        chart.setVisibleXRangeMaximum(150);
        // this automatically refreshes the chart (calls invalidate())
        chart.moveViewTo(data.getEntryCount(), 50f, YAxis.AxisDependency.LEFT);

    }

    private LineDataSet createSet() {
        LineDataSet set = new LineDataSet(null, "Real-time Line Data");
        set.setLineWidth(1f);
        set.setDrawValues(false);
        //set.setValueTextColor(getResources().getColor(Color.WHITE));
        //set.setColor(getResources().getColor(Color.WHITE));
        set.setMode(LineDataSet.Mode.LINEAR);
        set.setDrawCircles(false);
        set.setHighLightColor(Color.rgb(190, 190, 190));

        return set;
    }

}
