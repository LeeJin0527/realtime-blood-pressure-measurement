package com.example.ppgmeasure;

import android.os.Environment;
import android.util.Log;
import com.example.ppgmeasure.PPG.PPGData;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class CSVFile {
    String fileTitle;

    public CSVFile(String filename){
        this.fileTitle = filename + ".csv";
        File file = new File(Environment.getExternalStorageDirectory(), fileTitle);
        try{
            FileWriter writer = new FileWriter(file, false);
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
                writer.append(",");
                writer.append("hr");
                writer.append("\n");
            }
            if(filename.equals("ECG")){
                writer.append("Count");
                writer.append(",");
                writer.append("RAW ECG1");
                writer.append(",");
                writer.append("RAW ECG2");
                writer.append(",");
                writer.append("RAW ECG3");
                writer.append(",");
                writer.append("RAW ECG4");
                writer.append(",");
                writer.append("eTAg1");
                writer.append(",");
                writer.append("eTAG2");
                writer.append(",");
                writer.append("eTAG3");
                writer.append(",");
                writer.append("eTAG4");
                writer.append("\n");
            }

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
            writer.append(",");
            writer.append(Integer.toString(ppgData.getHeartRate()));
            writer.append("\n");
            writer.close();

        }catch(IOException e){
            e.printStackTrace();
        }
    }


}
