package com.example.kotlin_ver.PPG

import android.util.Log

class PPGData {
    private lateinit var packet: ByteArray
    private var count = 0
    private var x = 0
    private var y = 0
    private var z = 0
    private var heartRate = 0
    private var heartRateConfidence = 0
    private var grnCnt = 0
    private var grn2Cnt = 0
    private var ppgActivity = 0


    fun setPacket(packet: ByteArray) {
        this.packet = packet
        processData()
    }

    fun setCount(count: Int) {
        this.count = count
    }

    fun setX(x: Int) {
        this.x = x
    }

    fun setY(y: Int) {
        this.y = y
    }

    fun setY(): Int {
        return y
    }

    fun setZ(z: Int) {
        this.z = z
    }

    fun setHeartRate(heartRate: Int) {
        this.heartRate = heartRate
    }

    fun setHeartRateConfidence(heartRateConfidence: Int) {
        this.heartRateConfidence = heartRateConfidence
    }

    fun setGrnCnt(grnCnt: Int) {
        this.grnCnt = grnCnt
    }

    fun setGrn2Cnt(grn2Cnt: Int) {
        this.grn2Cnt = grn2Cnt
    }

    fun getPacket(): ByteArray? {
        return packet
    }

    fun getCount(): Int {
        return count
    }

    fun getX(): Int {
        return x
    }

    fun getY(): Int {
        return y
    }

    fun getZ(): Int {
        return z
    }

    fun getHeartRate(): Int {
        return heartRate
    }

    fun getHeartRateConfidence(): Int {
        return heartRateConfidence
    }

    fun getGrnCnt(): Int {
        return grnCnt
    }

    fun getGrn2Cnt(): Int {
        return grn2Cnt
    }

    fun getPpgActivity(): Int {
        return ppgActivity
    }

    private fun processData() {
        val dataPacket = packet
        count = (dataPacket[1].toInt() and 0xff) + (dataPacket[2].toInt() and 0xff shl 8)
        heartRate = (dataPacket[13].toInt() and 0xfc shr 2) + (dataPacket[14].toInt() and 0x3f shl 6)
        heartRateConfidence = (dataPacket[14].toInt() and 0xc0 shr 6) + (dataPacket[15].toInt() and 0x3f shl 2)
        grnCnt =
            (dataPacket[3].toInt() and 0xff) + (dataPacket[4].toInt() and 0xff shl 8) + (dataPacket[5].toInt() and 0x0f shl 16)
        grn2Cnt =
            (dataPacket[5].toInt() and 0xf0) + (dataPacket[6].toInt() and 0xff shl 4) + (dataPacket[7].toInt() and 0xff shl 12)
        ppgActivity = (dataPacket[17].toInt() and 0xfe shr 1) + (dataPacket[18].toInt() and 0x01 shl 7)
        Log.i("activity 값 : ", Integer.toString(ppgActivity))
    }
}