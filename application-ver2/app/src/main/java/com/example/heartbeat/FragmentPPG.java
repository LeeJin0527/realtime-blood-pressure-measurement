package com.example.heartbeat;

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

public class FragmentPPG extends Fragment{
    Button start;
    Button pause;
    Command MyCmd;
    FrameLayout frameLayout;
    TextView sensorField;
    View view;
    RealTimeGraph myGraph;
    Thread realTimeThread;
    Activity activity;

    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragmentppg, container, false);

        start = (Button)view.findViewById(R.id.start);
        pause = (Button)view.findViewById(R.id.pause);

        sensorField = (TextView)view.findViewById(R.id.temp_sensor_value);
        myGraph = new RealTimeGraph(view);
        realTimeThread = null;

        start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                ((MenuActivity)getActivity()).setTempvaluefield(view, "ppg");
                ((MenuActivity)getActivity()).sendStrCmd(MyCmd.str_readppg0);
                start.setVisibility(View.GONE);
                pause.setVisibility(View.VISIBLE);

                realTimeStart();


            }
        });

        pause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                ((MenuActivity)getActivity()).setTempvaluefield(view, "stop");
                ((MenuActivity)getActivity()).sendStrCmd(MyCmd.str_stop);
                start.setVisibility(View.VISIBLE);
                pause.setVisibility(View.GONE);

                realTimeThread.interrupt();
                realTimeThread = null;
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
                while(true){
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                                myGraph.addEntry(((MenuActivity) activity).valueForGraph);
                        }
                    });
                }
            }
        });

        realTimeThread.start();
    }


}
