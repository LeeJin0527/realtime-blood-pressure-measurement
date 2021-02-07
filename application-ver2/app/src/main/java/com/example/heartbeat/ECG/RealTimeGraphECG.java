package com.example.heartbeat.ECG;

import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.util.Log;
import android.view.View;

import androidx.core.content.ContextCompat;

import java.math.*;

import com.example.heartbeat.R;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;

public class RealTimeGraphECG {
    private LineChart chart;
    private float minX;
    private float maxX;

    public RealTimeGraphECG(View view){
        creatGraph(view);
    }

    private void creatGraph(View view){
        chart = (LineChart)view.findViewById(R.id.LineChart);
        chart.setDrawGridBackground(false);
        chart.setBackgroundColor(Color.WHITE);

        chart.getXAxis().setEnabled(false);
        chart.getAxisLeft().setDrawLabels(false);
        chart.getAxisLeft().setEnabled(false);
        chart.getAxisRight().setDrawLabels(false);
        chart.getXAxis().setDrawLabels(false);
        chart.getXAxis().setDrawGridLines(false);
        chart.getXAxis().setEnabled(false);
        chart.getAxisRight().setEnabled(false);
        chart.getDescription().setEnabled(false);


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

        chart.setVisibleXRangeMaximum(35);
        // this automatically refreshes the chart (calls invalidate())
        chart.moveViewTo(data.getEntryCount(), 50f, YAxis.AxisDependency.LEFT);

    }

    private LineDataSet createSet() {
        LineDataSet set = new LineDataSet(null, "ECG");
        set.setDrawFilled(false);
        set.setFillColor(Color.RED);
        set.setLineWidth(1f);
        set.setDrawValues(false);
        set.setMode(LineDataSet.Mode.LINEAR);
        set.setDrawCircles(false);
        set.setHighLightColor(Color.RED);
        set.setColor(Color.RED);

        return set;
    }

}
