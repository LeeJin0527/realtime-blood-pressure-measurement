package com.example.heartbeat.data;

public class PPGData {
    private byte[] packet;
    private int count;
    private int x;
    private int y;
    private int z;
    private double heartRate;
    private int heartRateConfidence;

    public void setPacket(byte[] packet) {
        this.packet = packet;
        processData();
    }

    public void setCount(int count){
        this.count = count;
    }

    public void setX(int x) {
        this.x = x;
    }

    public void setY(int y) {
        this.y = y;
    }

    public int setY() {
        return y;
    }

    public void setZ(int z) {
        this.z = z;
    }

    public void setHeartRate(int heartRate) {
        this.heartRate = heartRate;
    }

    public void setHeartRateConfidence(int heartRateConfidence) {
        this.heartRateConfidence = heartRateConfidence;
    }

    public byte[] getPacket() {
        return this.packet;
    }

    public int getCount() {
        return this.count;
    }

    public int getX() {
        return this.x;
    }

    public int getY() {
        return this.y;
    }

    public int getZ() {
        return this.z;
    }

    public double getHeartRate() {
        return this.heartRate;
    }

    public int getHeartRateConfidence() {
        return this.heartRateConfidence;
    }

    private void processData(){
        byte[] dataPacket = this.packet;

        this.heartRate = (double) (((dataPacket[13] & 0xfc) >> 2 )+ ((dataPacket[14] & 0x3f) << 6))/10.0;
        this.heartRateConfidence = ((dataPacket[14] & 0xc0) >> 6) + ((dataPacket[15] & 0x3f) << 2);
    }
}
