package com.example.kotlin_ver.ECG

class ECGData(packet: ByteArray) {
    var count = 0
    var RtoR = 0
    var RtoRBpm = 0

    var ecg1 = 0
    var ecg2 = 0
    var ecg3 = 0
    var ecg4 = 0
    //ecg1, 2, 3, 4 must changed to array type

    val eTag = 0
    val pTag = 0

    var ecg1eTag = 0
    var ecg1pTag = 0
    var ecg2eTag = 0
    var ecg2pTag = 0
    var ecg3eTag = 0
    var ecg3pTag = 0
    var ecg4eTag = 0
    var ecg4pTag = 0
    //p and e tag must be changed to array type
    var test_value_graph = 0

    init {
        var tmp = 0
        val total_ecg = 0

        count = packet[1].toInt() and 0xff
        ecg1 = (packet[4].toInt() and 0xc0 shr 6) + (packet[5].toInt() and 0xff shl 2) + (packet[6].toInt() and 0xff shl 10) + (packet[7].toInt() and 0x3f shl 18)
        ecg2 = (packet[7].toInt() and 0xc0 shr 6) + (packet[8].toInt() and 0xff shl 2) + (packet[9].toInt() and 0xff shl 10) + (packet[10].toInt() and 0x3f shl 18)
        ecg3 = (packet[10].toInt() and 0xc0 shr 6) + (packet[11].toInt() and 0xff shl 2) + (packet[12].toInt() and 0xff shl 10) + (packet[13].toInt() and 0x3f shl 18)
        ecg4 = (packet[13].toInt() and 0xc0 shr 6) + (packet[14].toInt() and 0xff shl 2) + (packet[15].toInt() and 0xff shl 10) + (packet[16].toInt() and 0x3f shl 18)

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

        tmp = (packet[2].toInt() and 0xff) + (packet[3].toInt() and 0x3f shl 8)
        if (tmp != 0) RtoR = tmp
        tmp = (packet[3].toInt() and 0xc0 shr 6) + (packet[4].toInt() and 0x3f shl 2)
        if (tmp != 0) RtoRBpm = tmp

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


