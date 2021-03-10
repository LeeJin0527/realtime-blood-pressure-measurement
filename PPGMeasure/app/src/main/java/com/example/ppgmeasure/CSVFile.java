package com.example.ppgmeasure;

import android.os.Environment;
import android.util.Log;
import com.example.ppgmeasure.PPG.PPGData;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

// CSV파일을 생성하고, 읽기 추가하는 매서드가 정의
public class CSVFile {
    String fileTitle;

    public CSVFile(String filename){
        this.fileTitle = filename + ".csv";
        File file = new File(Environment.getExternalStorageDirectory(), fileTitle);
        // 파일을 생성합니다
        // 파일을 생성경로는 Environment.getExternalStorageDirectory()로서
        // 보통 최상위 폴더 경로입니다(파일 탐색기 앱에서 -> 내부 저장소)

        try{
            FileWriter writer = new FileWriter(file, false);
            if(!file.exists()) {
                file.createNewFile();
            }

            // CSV파일을 생성하면서,
            // 각 열에 붙일 태그입니다.
            // 열을 건너뛰고자 할경우 ,(콤마)를 써주고
            // 행을 건너뛰고자 할경우 /n(줄끝맺음)을 써주면 됩니다.
            if(filename.equals("PPG")){
                writer.append("Count");// 1열
                writer.append(",");// 열 건너 뛰기
                writer.append("grnCnt");// 2열
                writer.append(",");
                writer.append("grn2Cnt");// 3열
                writer.append(",");
                writer.append("hr");// 4열
                writer.append("\n");// 행 건너 뛰기
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
            writer.append(Integer.toString(ppgData.getCount()));// 1열
            writer.append(",");// 건너뛰기
            writer.append(Double.toString(ppgData.getGrnCnt()));// 2열
            writer.append(",");
            writer.append(Double.toString(ppgData.getGrn2Cnt()));// 3열
            writer.append(",");
            writer.append(Integer.toString(ppgData.getHeartRate()));// 4열
            writer.append("\n");
            writer.close();

        }catch(IOException e){
            e.printStackTrace();
        }
    }


}
