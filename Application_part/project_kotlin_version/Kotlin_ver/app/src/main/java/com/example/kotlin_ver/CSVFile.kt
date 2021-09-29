package com.example.kotlin_ver

import android.os.Environment
import com.example.kotlin_ver.ECG.ECGData
import com.example.kotlin_ver.PPG.PPGData
import com.example.kotlin_ver.Temperature.TempData
import java.io.File
import java.io.FileWriter
import java.io.IOException

class CSVFile {
    var fileTitle: String? = null

    constructor(filename: String) {
        fileTitle = "$filename.csv"
        val file = File(Environment.getExternalStorageDirectory(), fileTitle)
        try {
            val writer = FileWriter(file, false)
            if (!file.exists()) {
                file.createNewFile()
            }
            if (filename == "Temp") {
                writer.append("Count")
                writer.append(",")
                writer.append("Temperature")
                writer.append("\n")
            }
            if (filename == "PPG") {
                writer.append("Count")
                writer.append(",")
                writer.append("grnCnt")
                writer.append(",")
                writer.append("grn2Cnt")
                writer.append(",")
                writer.append("hr")
                writer.append("\n")
            }
            if (filename == "ECG") {
                writer.append("Count")
                writer.append(",")
                writer.append("RAW ECG1")
                writer.append(",")
                writer.append("RAW ECG2")
                writer.append(",")
                writer.append("RAW ECG3")
                writer.append(",")
                writer.append("RAW ECG4")
                writer.append(",")
                writer.append("eTAg1")
                writer.append(",")
                writer.append("eTAG2")
                writer.append(",")
                writer.append("eTAG3")
                writer.append(",")
                writer.append("eTAG4")
                writer.append("\n")
            }
            writer.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    fun writeTempFile(tempData: TempData) {
        val file = File(Environment.getExternalStorageDirectory(), fileTitle)
        try {
            val writer = FileWriter(file, true)
            if (!file.exists()) {
                file.createNewFile()
            }
            writer.append(Integer.toString(tempData.getCount()))
            writer.append(",")
            writer.append(java.lang.Double.toString(tempData.getValue()))
            writer.append("\n")
            writer.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    fun writePPGFile(ppgData: PPGData) {
        val file = File(Environment.getExternalStorageDirectory(), fileTitle)
        try {
            val writer = FileWriter(file, true)
            if (!file.exists()) {
                file.createNewFile()
            }
            writer.append(Integer.toString(ppgData.getCount()))
            writer.append(",")
            writer.append(ppgData.getGrnCnt().toString())
            writer.append(",")
            writer.append(ppgData.getGrn2Cnt().toString())
            writer.append(",")
            writer.append(ppgData.getHeartRate().toString())
            writer.append("\n")
            writer.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    fun writeECGFile(ecgData: ECGData) {
        val file = File(Environment.getExternalStorageDirectory(), fileTitle)
        try {
            val writer = FileWriter(file, true)
            if (!file.exists()) {
                file.createNewFile()
            }
            writer.append(Integer.toString(ecgData.getCount()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg1()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg2()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg3()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg4()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg1eTag()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg2eTag()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg3eTag()))
            writer.append(",")
            writer.append(Integer.toString(ecgData.getEcg4eTag()))
            writer.append("\n")
            writer.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }
}