package com.example.heartbeat.PPG;

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

public class RealTimeGraphPPG {
    private LineChart chart;
    private float minX;
    private float maxX;
    private int count;

    public RealTimeGraphPPG(View view){
        creatGraph(view);
    }

    private void creatGraph(View view){
        chart = (LineChart)view.findViewById(R.id.LineChart);
        chart.setDrawGridBackground(true);
        chart.setBackgroundColor(Color.WHITE);
        chart.setGridBackgroundColor(Color.WHITE);


        chart.getXAxis().setEnabled(false);

        YAxis leftAxis = chart.getAxisLeft();
        leftAxis.setEnabled(true);
        //leftAxis.setTextColor(getResources().getColor(R.color.colorGrid));
        leftAxis.setDrawGridLines(true);
        //leftAxis.setGridColor(getResources().getColor(R.color.colorGrid));


        YAxis rightAxis = chart.getAxisRight();
        rightAxis.setEnabled(false);



        chart.getAxisLeft().setDrawLabels(true);
        chart.getAxisRight().setDrawLabels(false);
        chart.getXAxis().setDrawLabels(false);
        chart.getXAxis().setDrawGridLines(false);


        chart.setVisibleXRangeMaximum(75);
        chart.getDescription().setEnabled(false);

        Paint pain = chart.getRenderer().getPaintRender();
        count = 0;

        chart.invalidate();


    }
    public void addEntry(double num) {
        LineData data = chart.getData();
        chart.getAxisLeft().setAxisMaximum((float)num + 5000);
        chart.getAxisLeft().setAxisMinimum((float)num - 5000);

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
        //count++;
        //if(count >= 75)yAxiSetting();

        chart.setVisibleXRangeMaximum(75);
        // this automatically refreshes the chart (calls invalidate())
        chart.moveViewTo(data.getEntryCount(), 50f, YAxis.AxisDependency.LEFT);

    }

    private LineDataSet createSet() {
        LineDataSet set = new LineDataSet(null, "ch1");
        set.setLineWidth(1f);
        set.setDrawValues(false);
        set.setMode(LineDataSet.Mode.LINEAR);
        set.setDrawCircles(false);
        set.setHighLightColor(Color.RED);
        set.setColor(Color.RED);

        return set;
    }

    private void yAxiSetting(){
        count = 0;
    }

}
