package com.example.heartbeat.Temperature;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.text.BoringLayout;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.os.EnvironmentCompat;
import androidx.fragment.app.Fragment;

import com.example.heartbeat.Command;
import com.example.heartbeat.MenuActivity;
import com.example.heartbeat.R;
import com.example.heartbeat.Temperature.RealTimeGraphTemp;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import com.opencsv.CSVWriter;
import com.opencsv.CSVReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class FragmentTemp extends Fragment{
    Button start;
    Button pause;

    Command MyCmd;
    FrameLayout frameLayout;
    TextView sensorField;
    View view;
    RealTimeGraphTemp myGraph;
    Thread realTimeThread;
    Activity activity;
    Boolean threadFlag;

    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragmenttemp, container, false);

        start = (Button)view.findViewById(R.id.start);
        pause = (Button)view.findViewById(R.id.pause);
        sensorField = (TextView)view.findViewById(R.id.temp_sensor_value);
        myGraph = new RealTimeGraphTemp(view);

        //String CSVPath = ((MenuActivity)getActivity()).AppAbsolutePath;
        String CSVPath = "/Android/data/";
        String myFile = CSVPath + "test.csv";

/*
        try {
            FileWriter fw = new FileWriter(myFile);

            fw.append("count");
            fw.append(",");
            fw.append("Temperature");
            fw.append("\n");
            fw.flush();
            fw.close();
            Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("text/plain");
            intent.putExtra(intent.EXTRA_TITLE, myFile);
            startActivityForResult(intent, 1);
        }catch(IOException e){
            e.printStackTrace();
        }
*/

        writeFile();
        //Log.i("경로", CSVPath);

        threadFlag = false;

        start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                ((MenuActivity)getActivity()).setViewField(view, "temp");
                ((MenuActivity)getActivity()).sendStrCmd(MyCmd.str_readtemp0);
                start.setVisibility(View.GONE);
                pause.setVisibility(View.VISIBLE);
                threadFlag = true;
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
                realTimeThread.interrupt();
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
        //stop추가 안하면, null point 에러남
        //stop 커맨드도 값이 바뀌는 경우라서
        //process_data 메서드가 호출되기 때문
        frameLayout.setVisibility(View.GONE);
        if(realTimeThread != null)realTimeThread.interrupt();

        super.onDetach();

    }



    public void realTimeStart(){
        realTimeThread = new Thread(new Runnable() {
            @Override
            public void run() {
                while(threadFlag){
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            myGraph.addEntry(((MenuActivity)activity).valueForGraph);
                        }
                    });
                }
            }
        });

        realTimeThread.start();
    }

    public void writeFile() {
        String fileTitle = "testcsvfile.csv";
        File file = new File(Environment.getExternalStorageDirectory(), fileTitle);
        Log.i("write 디렉토리 확인", Environment.getExternalStorageDirectory().getAbsolutePath().toString());

        try {
            if (!file.exists()) {
                file.createNewFile();
            }
            FileWriter writer = new FileWriter(file, false);
            writer.append("Temperature");
            writer.append(",");
            writer.append("count");
            writer.append("\n");
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


}
