package com.example.heartbeat.data;

public class PPGData {
    private byte[] packet;
    private int count;
    private int x;
    private int y;
    private int z;
    private int heartRate;
    private int heartRateConfidence;

    public void setPacket(byte[] packet) {
        this.packet = packet;
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

    public int getHeartRate() {
        return this.heartRate;
    }

    public int getHeartRateConfidence() {
        return this.heartRateConfidence;
    }
}
