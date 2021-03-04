package com.example.heartbeat.PPG;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.example.heartbeat.Command;
import com.example.heartbeat.MenuActivity;
import com.example.heartbeat.R;

public class FragmentPPG extends Fragment{
    Button start;
    Button pause;
    Command MyCmd;
    FrameLayout frameLayout;
    TextView sensorField;
    View view;
    RealTimeGraphPPG myGraph;
    Thread realTimeThread;
    Activity activity;
    Boolean threadFlag;
    Boolean initialStartFlag;
    //ppg의 경우, 초기에 일정 인터벌 쉬었다가 그래프에 그려줘야 하는데, runOnUiThread를 우아하게 멈출 방법이 아직 생각이 안나서 임시방편으로 flag와 if문으로 인터벌 쉬도록 구현함

    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragmentppg, container, false);

        start = (Button)view.findViewById(R.id.start);
        pause = (Button)view.findViewById(R.id.pause);

        sensorField = (TextView)view.findViewById(R.id.temp_sensor_value);
        myGraph = new RealTimeGraphPPG(view);
        realTimeThread = null;
        threadFlag = false;

        start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                ((MenuActivity)getActivity()).setViewField(view, "ppg");
                ((MenuActivity)getActivity()).sendStrCmd(MyCmd.str_readppg0);

                start.setVisibility(View.GONE);
                pause.setVisibility(View.VISIBLE);
                threadFlag = true;
                initialStartFlag = true;
                realTimeStart();


            }
        });

        pause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                ((MenuActivity)getActivity()).setViewField(view, "stop");
                ((MenuActivity)getActivity()).sendStrCmd(MyCmd.str_stop);
                start.setVisibility(View.VISIBLE);
                pause.setVisibility(View.GONE);
                threadFlag = false;
            }
        });

        LayoutInflater layoutInflater = (LayoutInflater)getActivity().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        frameLayout = ((MenuActivity)getActivity()).getFrameLayout();

        MyCmd = new Command();

        return view;
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);

        if (context instanceof Activity)
            activity = (Activity) context;
    }
    @Override
    public void onDetach() {
        ((MenuActivity)getActivity()).sendStrCmd(MyCmd.str_stop);
        ((MenuActivity)getActivity()).mode="stop";
        frameLayout.setVisibility(View.GONE);
        super.onDetach();

    }


    public void realTimeStart(){
        realTimeThread = new Thread(new Runnable() {
            
            @Override
            public void run() {
                while(threadFlag){
                    try {
                        if(initialStartFlag){
                            Thread.sleep(10000);
                            initialStartFlag = false;
                        }
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                                myGraph.addEntry( ((MenuActivity) activity).ppgData.getGrnCnt(), ((MenuActivity) activity).ppgData.getGrn2Cnt());
                        }
                    });
                }
            }
        });

        realTimeThread.start();
    }


}
