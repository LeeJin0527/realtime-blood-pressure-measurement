package com.example.kotlin_ver.PPG

import android.util.Log

class PPGData(packet: ByteArray) {
    var count = 0
    var x = 0
    var y = 0
    var z = 0
    var heartRate = 0
    var heartRateConfidence = 0
    var grnCnt = 0
    var grn2Cnt = 0
    var ppgActivity = 0

    init {
        count = (packet[1].toInt() and 0xff) + (packet[2].toInt() and 0xff shl 8)
        heartRate = (packet[13].toInt() and 0xfc shr 2) + (packet[14].toInt() and 0x3f shl 6)
        heartRateConfidence = (packet[14].toInt() and 0xc0 shr 6) + (packet[15].toInt() and 0x3f shl 2)
        grnCnt = (packet[3].toInt() and 0xff) + (packet[4].toInt() and 0xff shl 8) + (packet[5].toInt() and 0x0f shl 16)
        grn2Cnt = (packet[5].toInt() and 0xf0) + (packet[6].toInt() and 0xff shl 4) + (packet[7].toInt() and 0xff shl 12)
        ppgActivity = (packet[17].toInt() and 0xfe shr 1) + (packet[18].toInt() and 0x01 shl 7)

        Log.i("activity 값 : ", Integer.toString(ppgActivity))
    }

}