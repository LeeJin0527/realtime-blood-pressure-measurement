package com.example.ppgmeasure.PPG;

import android.util.Log;

import uk.me.berndporr.iirj.Butterworth;

public class PPGData {
    private byte[] packet;
    private int count;
    private int x;
    private int y;
    private int z;
    private int heartRate;
    private int heartRateConfidence;
    private double grnCnt;
    private double grn2Cnt;
    private int ppgActivity;


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

    public void setGrnCnt(int grnCnt){
        this.grnCnt = grnCnt;
    }

    public void setGrn2Cnt(int grn2Cnt){
        this.grn2Cnt = grn2Cnt;
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

    public double getGrnCnt(){return this.grnCnt;}

    public double getGrn2Cnt(){return this.grn2Cnt;}

    public int getPpgActivity(){return this.ppgActivity;}

    private void applyFilter(){
        int dt = 25;
        int order = 5;
        double fs = 1/(double)25.0;
        double cutoff = 1/(double)(dt*24);
        int fb = 3;

        Butterworth butterworth = new Butterworth();

        butterworth.highPass(order, fs, cutoff);
        this.grnCnt = butterworth.filter((double) (this.grnCnt));
    }

    private void processData(){
        byte[] dataPacket = this.packet;
        this.count = (dataPacket[1] & 0xff) + ((dataPacket[2] & 0xff)<<8);
        this.heartRate = (((dataPacket[13] & 0xfc) >> 2 )+ ((dataPacket[14] & 0x3f) << 6));
        this.heartRateConfidence = ((dataPacket[14] & 0xc0) >> 6) + ((dataPacket[15] & 0x3f) << 2);
        this.grnCnt = (dataPacket[3] & 0xff ) + ((dataPacket[4] & 0xff )<<8) + ((dataPacket[5] & 0x0f)<<16);
        this.grn2Cnt = (dataPacket[5] & 0xf0) + ((dataPacket[6] & 0xff)<<4) + ((dataPacket[7] & 0xff)<<12);
        this.ppgActivity = ((dataPacket[17] & 0xfe) >> 1) + ((dataPacket[18] & 0x01) << 7);
        Log.i("activity 값 : ", Integer.toString(this.ppgActivity));
        Log.i("calc값 ", Double.toString(this.grnCnt));
        applyFilter();
    }
}
