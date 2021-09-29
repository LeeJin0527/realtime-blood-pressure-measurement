package com.example.kotlin_ver.Temperature

class TempData {
    private var packet : ByteArray? = null
    private var count = 0
    private var value = 0.0

    fun setPacket(packet : ByteArray){
        this.packet = packet
        processData()
    }

    fun setCount(count: Int) {
        this.count = count
    }

    fun setValue(value: Float) {
        this.value = value.toDouble()
    }

    fun getCount(): Int {
        return count
    }

    fun getValue(): Double {
        return value
    }

    fun convertCelciustoF(): Double {
        return value * (9 / 5.0) + 32.0
    }

    private fun processData() {
        count = packet!![1].toInt() and 0xff
        value = ((packet!![3].toInt() and 0xff shl 8) + (packet!![2].toInt() and 0xff)) / 100.0
    }
}