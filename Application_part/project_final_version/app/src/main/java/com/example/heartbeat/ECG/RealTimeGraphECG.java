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
import com.github.mikephil.charting.components.XAxis;
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

        chart.getAxisLeft().setDrawLabels(true);
        chart.getAxisLeft().setEnabled(true);
        chart.getAxisRight().setDrawLabels(false);
        chart.getXAxis().setDrawLabels(true);
        chart.getXAxis().setDrawGridLines(false);
        chart.getXAxis().setEnabled(true);
        chart.getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM);
        chart.getAxisRight().setEnabled(false);
        chart.getDescription().setEnabled(false);


        chart.invalidate();


    }
    public void addEntry(double num) {
        LineData data = chart.getData();

        chart.getAxisLeft().setAxisMaximum(2000f);
        chart.getAxisLeft().setAxisMinimum(-2000f);


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

        chart.setVisibleXRangeMaximum(130);
        // this automatically refreshes the chart (calls invalidate())
        chart.moveViewTo(data.getEntryCount(), 100f, YAxis.AxisDependency.LEFT);
        //chart.moveViewToX(data.getEntryCount());
        //chart.moveViewTo(data.getEntryCount(), 50f, YAxis.AxisDependency.RIGHT);
        //chart.invalidate();

    }
    public void resetMaxandMin(){
        Log.i("호출 : ", "호출됨!");
        chart.getAxisLeft().resetAxisMaximum();
        chart.getAxisLeft().resetAxisMinimum();
    }

    private LineDataSet createSet() {
        LineDataSet set = new LineDataSet(null, "ECG");
        set.setDrawFilled(false);
        set.setFillColor(Color.RED);
        set.setLineWidth(1f);
        set.setDrawValues(false);
        set.setMode(LineDataSet.Mode.LINEAR);
        set.setDrawCircles(true);
        set.setHighLightColor(Color.RED);
        set.setColor(Color.RED);

        return set;
    }

}
