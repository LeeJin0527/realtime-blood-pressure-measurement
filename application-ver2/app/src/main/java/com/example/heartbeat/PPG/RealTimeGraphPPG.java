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
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;

public class RealTimeGraphPPG {
    private static final String TAG = RealTimeGraphPPG.class.getSimpleName();
    private LineChart chart;
    private float minX;
    private float maxX;
    private int count;

    public RealTimeGraphPPG(View view){
        creatGraph(view);
    }

    private void creatGraph(View view){
        chart = (LineChart)view.findViewById(R.id.LineChart);
        chart.setDrawGridBackground(false);
        chart.setBackgroundColor(Color.WHITE);

        chart.getXAxis().setEnabled(false);
        chart.getAxisLeft().setDrawLabels(true);
        chart.getAxisLeft().setEnabled(true);
        chart.getAxisRight().setDrawLabels(false);
        chart.getXAxis().setDrawLabels(true);
        chart.getXAxis().setDrawGridLines(false);
        chart.getXAxis().setEnabled(true);
        chart.getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM);
        chart.getAxisRight().setEnabled(false);

        chart.setVisibleXRangeMaximum(20);
        chart.getDescription().setEnabled(false);

        chart.invalidate();
    }
    public void addEntry(double num1, double num2) {
        LineData data = chart.getData();
        float min;
        float max;
        float diff;

        Log.i(TAG, "ch1 = " + String.valueOf(num1));
        Log.i(TAG, "ch2 = " + String.valueOf(num2));

        if(num1 > num2){
            max = (float) num1;
            min = (float) num2;
            diff = (max - min)*0.01f;
            num1 = num2 + diff;
            max = (float) num1;

        }
        else{
            max = (float) num2;
            min = (float) num1;
            diff = (max - min)*0.01f;
            num2 = num1 + diff;
            max = (float) num2;
        }



        //chart.getAxisLeft().setAxisMaximum(max + 1000);
        //chart.getAxisLeft().setAxisMinimum(min - 1000);



        if (data == null) {
            data = new LineData();
            chart.setData(data);


        }
        LineDataSet set3 = (LineDataSet) data.getDataSetByIndex(0);
        LineDataSet set4 = (LineDataSet) data.getDataSetByIndex(1);

        ILineDataSet set = data.getDataSetByIndex(0);
        // set.addEntry(...); // can be called as well
        ILineDataSet set2 = data.getDataSetByIndex(1);

        if (set == null) {
            set = createSet();
            data.addDataSet(set);

            set2 = createSet2();
            data.addDataSet(set2);
        }


        data.addEntry(new Entry((float)set.getEntryCount(), (float)num1), 0);
        data.addEntry(new Entry((float)set2.getEntryCount(), (float)num2), 1);

        data.notifyDataChanged();

        // let the chart know it's data has changed
        chart.notifyDataSetChanged();
        //count++;
        //if(count >= 75)yAxiSetting();

        chart.setVisibleXRangeMaximum(120);
        // this automatically refreshes the chart (calls invalidate())
        chart.moveViewTo(data.getEntryCount(), 50f, YAxis.AxisDependency.LEFT);

    }

    private LineDataSet createSet() {
        LineDataSet set = new LineDataSet(null, "ch1");
        set.setLineWidth(1f);
        set.setDrawValues(false);
        set.setMode(LineDataSet.Mode.LINEAR);
        set.setDrawCircles(false);
        set.setHighLightColor(Color.GREEN);
        set.setColor(Color.GREEN);

        return set;
    }

    private LineDataSet createSet2() {
        LineDataSet set = new LineDataSet(null, "ch2");
        set.setLineWidth(1f);
        set.setDrawValues(false);
        set.setMode(LineDataSet.Mode.LINEAR);
        set.setDrawCircles(false);
        set.setHighLightColor(Color.BLUE);
        set.setColor(Color.BLUE);

        return set;
    }

    private void yAxiSetting(){
        count = 0;
    }

}
