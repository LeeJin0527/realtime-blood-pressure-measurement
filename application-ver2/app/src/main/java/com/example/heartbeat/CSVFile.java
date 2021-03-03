package com.example.heartbeat;

import android.os.Environment;
import android.util.Log;

import com.example.heartbeat.ECG.ECGData;
import com.example.heartbeat.PPG.PPGData;
import com.example.heartbeat.Temperature.TempData;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class CSVFile {
    String fileTitle;

    public CSVFile(String filename){
        this.fileTitle = filename + ".csv";
        File file = new File(Environment.getExternalStorageDirectory(), fileTitle);
        try{
            FileWriter writer = new FileWriter(file, true);
            if(!file.exists()) {
                file.createNewFile();
            }

            if(filename.equals("Temp")) {
                writer.append("Count");
                writer.append(",");
                writer.append("Temperature");
                writer.append("\n");
            }
            if(filename.equals("PPG")){
                writer.append("Count");
                writer.append(",");
                writer.append("grnCnt");
                writer.append(",");
                writer.append("grn2Cnt");
                writer.append("\n");
            }
            if(filename.equals("ECG")){
                writer.append("Count");
                writer.append(",");
                writer.append("RAW ECG");
                writer.append(",");
                writer.append("ETAG");
                writer.append(",");
                writer.append("PTAG");
                writer.append("\n");
            }

            writer.close();

        }catch(IOException e){
            e.printStackTrace();
        }
    }

    void writeTempFile(TempData tempData){
        File file = new File(Environment.getExternalStorageDirectory(), this.fileTitle);

        try{
            FileWriter writer = new FileWriter(file, true);
            if(!file.exists()) {
                file.createNewFile();
            }
            writer.append(Integer.toString(tempData.getCount()));
            writer.append(",");
            writer.append(Double.toString(tempData.getValue()));
            writer.append("\n");
            writer.close();

        }catch(IOException e){
            e.printStackTrace();
        }
    }

    void writePPGFile(PPGData ppgData){
        File file = new File(Environment.getExternalStorageDirectory(), this.fileTitle);

        try{
            FileWriter writer = new FileWriter(file, true);
            if(!file.exists()) {
                file.createNewFile();
            }
            writer.append(Integer.toString(ppgData.getCount()));
            writer.append(",");
            writer.append(Double.toString(ppgData.getGrnCnt()));
            writer.append(",");
            writer.append(Double.toString(ppgData.getGrn2Cnt()));
            writer.append("\n");
            writer.close();

        }catch(IOException e){
            e.printStackTrace();
        }
    }

    void writeECGFile(ECGData ecgData){
        File file = new File(Environment.getExternalStorageDirectory(), this.fileTitle);

        try{
            FileWriter writer = new FileWriter(file, true);
            if(!file.exists()) {
                file.createNewFile();
            }
            writer.append(Integer.toString(ecgData.getCount()));
            writer.append(",");
            writer.append(Integer.toString(ecgData.getEcg1()));
            writer.append("\n");
            writer.close();

        }catch(IOException e){
            e.printStackTrace();
        }
    }

}
