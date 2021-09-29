package com.example.kotlin_ver.ECG

import kotlin.experimental.and

class ECGData {
    private lateinit var packet: ByteArray
    private var count = 0
    private var RtoR = 0
    private var RtoRBpm = 0
    private var ecg1 = 0
    private var ecg2 = 0
    private var ecg3 = 0
    private var ecg4 = 0
    private val eTag = 0
    private val pTag = 0
    private var ecg1eTag = 0
    private var ecg1pTag = 0
    private var ecg2eTag = 0
    private var ecg2pTag = 0
    private var ecg3eTag = 0
    private var ecg3pTag = 0
    private var ecg4eTag = 0
    private var ecg4pTag = 0
    private var test_value_graph = 0

    fun ECGData() {
        RtoR = 0
        RtoRBpm = 0
    }

    fun setPacket(packet: ByteArray) {
        this.packet = packet
        processData()
    }

    fun setCount(count: Int) {
        this.count = count
    }

    fun setRtor(RtoR: Int) {
        this.RtoR = RtoR
    }

    fun setARtoR(RtoRBpm: Int) {
        this.RtoRBpm = RtoRBpm
    }

    fun getCount(): Int {
        return count
    }

    fun getRtoR(): Int {
        return RtoR
    }

    fun getRtoRBpm(): Int {
        return RtoRBpm
    }

    fun getEcg1(): Int {
        return ecg1
    }

    fun getEcg2(): Int {
        return ecg2
    }

    fun getEcg3(): Int {
        return ecg3
    }

    fun getEcg4(): Int {
        return ecg4
    }

    fun geteTag(): Int {
        return eTag
    }

    fun getpTag(): Int {
        return pTag
    }

    fun getEcg1eTag(): Int {
        return ecg1eTag
    }

    fun getEcg2eTag(): Int {
        return ecg2eTag
    }

    fun getEcg3eTag(): Int {
        return ecg3eTag
    }

    fun getEcg4eTag(): Int {
        return ecg4eTag
    }

    fun getTest_value_graph(): Int {
        return test_value_graph
    }

    fun processData() {
        var tmp = 0
        val total_ecg = 0
        val dataPacket = packet
        count = dataPacket[1].toInt() and 0xff
        ecg1 = (dataPacket[4].toInt() and 0xc0 shr 6) + (dataPacket[5].toInt() and 0xff shl 2) + (dataPacket[6].toInt() and 0xff shl 10) + (dataPacket[7].toInt() and 0x3f shl 18)
        ecg2 = (dataPacket[7].toInt() and 0xc0 shr 6) + (dataPacket[8].toInt() and 0xff shl 2) + (dataPacket[9].toInt() and 0xff shl 10) + (dataPacket[10].toInt() and 0x3f shl 18)
        ecg3 = (dataPacket[10].toInt() and 0xc0 shr 6) + (dataPacket[11].toInt() and 0xff shl 2) + (dataPacket[12].toInt() and 0xff shl 10) + (dataPacket[13].toInt() and 0x3f shl 18)
        ecg4 = (dataPacket[13].toInt() and 0xc0 shr 6) + (dataPacket[14].toInt() and 0xff shl 2) + (dataPacket[15].toInt() and 0xff shl 10) + (dataPacket[16].toInt() and 0x3f shl 18)
        ecg1eTag = ecg1 and 0x38 shr 3
        ecg1pTag = ecg1 and 0x07
        ecg1 = ecg1 shr 6
        ecg2eTag = ecg2 and 0x38 shr 3
        ecg2pTag = ecg2 and 0x07
        ecg2 = ecg2 shr 6
        ecg3eTag = ecg3 and 0x38 shr 3
        ecg3pTag = ecg3 and 0x07
        ecg3 = ecg3 shr 6
        ecg4eTag = ecg4 and 0x38 shr 3
        ecg4pTag = ecg4 and 0x07
        ecg4 = ecg4 shr 6

        //Log.i("eTag : ", Integer.toString(this.eTag));
        //Log.i("pTag : ", Integer.toString(this.pTag));
        tmp = (dataPacket[2].toInt() and 0xff) + (dataPacket[3].toInt() and 0x3f shl 8)
        if (tmp != 0) RtoR = tmp
        tmp = (dataPacket[3].toInt() and 0xc0 shr 6) + (dataPacket[4].toInt() and 0x3f shl 2)
        if (tmp != 0) RtoRBpm = tmp


        //tmp_data = ((dataPacket[4] & 0xc0) >> 6) + ((dataPacket[5]& 0xff) << 2) + ((dataPacket[6] & 0xff) << 10) + ((dataPacket[7] &0x3f) << 18);
        //tmp_data = this.ecg1;
        //tmp_data = total_ecg;


        //tmp_data = tmp_data >> 6;
        //Log.i("전체 데이터 : ", Integer.toBinaryString(tmp_data));
        if (ecg1 and 0x20000 != 0x0) {
            ecg1 = ecg1 xor 0x3ffff
            ecg1 = ecg1 + 0x01
            ecg1 = -ecg1
        }
        if (ecg2 and 0x20000 != 0x0) {
            ecg2 = ecg2 xor 0x3ffff
            ecg2 = ecg2 + 0x01
            ecg2 = -ecg2
        }
        if (ecg3 and 0x20000 != 0x0) {
            ecg3 = ecg3 xor 0x3ffff
            ecg3 = ecg3 + 0x01
            ecg3 = -ecg3
        }
        if (ecg4 and 0x20000 != 0x0) {
            ecg4 = ecg4 xor 0x3ffff
            ecg4 = ecg4 + 0x01
            ecg4 = -ecg4
        }
        if (ecg1eTag == 0 || ecg1eTag == 2) test_value_graph = ecg1
        if (ecg2eTag == 0 || ecg2eTag == 2) test_value_graph = ecg2
        if (ecg3eTag == 0 || ecg3eTag == 2) test_value_graph = ecg3
        if (ecg4eTag == 0 || ecg4eTag == 2) test_value_graph = ecg4
        //모든 eTag값을 검사해봐야함, 뭔가 두번이상 실행되는 분기문도 있을듯
    }
}


