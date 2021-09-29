/*package com.example.ppgmeasure.PPG;

import android.graphics.Color;
import android.util.Log;
import android.view.View;

import com.example.ppgmeasure.R;
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
*/
    // 그래프 틀을 만들어주는 매서드
    // 현재 배경에 격자 무늬는 그리지 않도록 하고
    // 배경은 하얀색이며, x축은 아래쪽에 그리도록 하며
    // x축의 라벨은 표시하도록 했습니다
    // y축은 왼쪽에 표시하도록 하며 또한 라벨도 표시하도록 했습니다
    // 마지막으로 그래프에 대한 설명은 추가하지 않도록 해놨습니다
    /*
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

        chart.setVisibleXRangeMaximum(120);
        chart.getDescription().setEnabled(false);

        chart.invalidate();
    }

    // 그래프에 그려줄 데이터를 입력하는 매서드로, 한 개의 그래프를 그립니다
    public void addEntry(double num){
        LineData data = chart.getData();

        if(data == null){
            data = new LineData();
            chart.setData(data);
        }

        ILineDataSet set = data.getDataSetByIndex(0);

        if (set == null) {
            set = createSet();
            data.addDataSet(set);
        }

        data.addEntry(new Entry((float)set.getEntryCount(), (float)num), 0);

        data.notifyDataChanged();

        chart.notifyDataSetChanged();
        chart.setVisibleXRangeMaximum(120);
        chart.moveViewToX(data.getEntryCount());
    }

    // 그래프에 그려줄 데이터를 입력하는 매서드로, 두 개의 그래프를 그립니다
    public void addEntry(double num1, double num2) {
        LineData data = chart.getData();
        float min;
        float max;
        float diff;

        Log.i(TAG, "ch1 = " + String.valueOf(num1));
        Log.i(TAG, "ch2 = " + String.valueOf(num2));
        //어떤 값이 그래프에 그려지는 확인용

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


        if (data == null) {
            data = new LineData();
            chart.setData(data);
        }

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

}
*/