package com.example.kotlin_ver.Temperature

class TempData(packet: ByteArray) {
    var count = 0
    var value = 0.0

    init {
        count = packet[1].toInt() and 0xff
        value = ((packet[3].toInt() and 0xff shl 8) + (packet[2].toInt() and 0xff)) / 100.0
    }

    fun convertCelciustoF(): Double {
        return value * (9 / 5.0) + 32.0
    }

}