package com.example.heartbeat.data;

/**
 * setPacket()메서드를 호출하면, processData 메서드를 이용해서
 * field 에 대응되는 값들을 처리해서 저장
 */

public class TempData {
    private byte[] packet;
    private int count;
    private double value;

    public void setPacket(byte[] packet) {
        this.packet = packet;
        processData();
    }

    public void setCount(int count) {
        this.count = count;
    }

    public void setValue(float value) {
        this.value = value;
    }

    public byte[] getPacket() {
        return packet;
    }

    public int getCount() {
        return count;
    }

    public double getValue() {
        return value;
    }

    private void processData(){
        this.count = packet[1] & 0xff;
        this.value = (((packet[3] & 0xff) << 8 ) + (packet[2] & 0xff))/100.0;
    }
}
