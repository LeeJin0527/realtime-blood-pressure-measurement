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
            writer.append("${tempData.count}")
            writer.append(",")
            writer.append("${tempData.value}")
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
            writer.append("${ppgData.count}")
            writer.append(",")
            writer.append("${ppgData.grnCnt}")
            writer.append(",")
            writer.append("${ppgData.grn2Cnt}")
            writer.append(",")
            writer.append("${ppgData.heartRate}")
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
            writer.append("${ecgData.count}")
            writer.append(",")
            writer.append("${ecgData.ecg1}")
            writer.append(",")
            writer.append("${ecgData.ecg2}")
            writer.append(",")
            writer.append("${ecgData.ecg3}")
            writer.append(",")
            writer.append("${ecgData.ecg4}")
            writer.append(",")
            writer.append("${ecgData.ecg1eTag}")
            writer.append(",")
            writer.append("${ecgData.ecg2eTag}")
            writer.append(",")
            writer.append("${ecgData.ecg3eTag}")
            writer.append(",")
            writer.append("${ecgData.ecg4eTag}")
            writer.append("\n")
            writer.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }
}