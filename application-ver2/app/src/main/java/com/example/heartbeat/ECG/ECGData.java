package com.example.heartbeat.ECG;

import android.util.Log;

public class ECGData {
    private byte[] packet;
    private int count;
    private int RtoR;
    private int RtoRBpm;
    private int ecg1;
    private int ecg2;
    private int ecg3;
    private int ecg4;
    private int eTag;
    private int pTag;

    public ECGData(){
        this.RtoR = 0;
        this.RtoRBpm = 0;
    }

    public void setPacket(byte[] packet) {
        this.packet = packet;

        processData();
    }

    public void setCount(int count) {
        this.count = count;
    }

    public void setRtor(int RtoR) {
        this.RtoR = RtoR;
    }

    public void setARtoR(int RtoRBpm){
        this.RtoRBpm = RtoRBpm;
    }

    public int getCount() {
        return this.count;
    }

    public int getRtoR(){
        return this.RtoR;
    }

    public int getRtoRBpm() {
        return this.RtoRBpm;
    }

    public int getEcg1() {
        return this.ecg1;
    }

    public void processData(){
        int tmp = 0;
        int total_ecg = 0;
        byte[] dataPacket = this.packet;
        this.count = dataPacket[1] & 0xff;

        this.ecg1 = ((dataPacket[4] & 0xc0) >> 6) + ((dataPacket[5]& 0xff) << 2) + ((dataPacket[6] & 0xff) << 10) + ((dataPacket[7] &0x3f) << 18);
        this.ecg2 = ((dataPacket[7] & 0xc0) >> 6) + ((dataPacket[8] & 0xff) << 2) + ((dataPacket[9] & 0xff) << 10) + ((dataPacket[10] & 0x3f) << 18);
        this.ecg3 = ((dataPacket[10] & 0xc0) >> 6) + ((dataPacket[11] & 0xff) << 2) + ((dataPacket[12] & 0xff) << 10) + ((dataPacket[13] & 0x3f) << 18);
        this.ecg4 = ((dataPacket[13] & 0xc0) >> 6) + ((dataPacket[14] & 0xff) << 2) + ((dataPacket[15] & 0xff) << 10) + ((dataPacket[16] & 0x3f) << 18);

        total_ecg = this.ecg1 | this.ecg2 | this.ecg3 | this.ecg4;
        this.eTag = (this.ecg1 & 0x38) >> 3 ;
        this.pTag = this.ecg1 & 0x07;
        //Log.i("eTag : ", Integer.toString(this.eTag));
        //Log.i("pTag : ", Integer.toString(this.pTag));
        int tmp_data;

        tmp = (dataPacket[2] & 0xff) + ((dataPacket[3] & 0x3f) << 8);
        if(tmp != 0)this.RtoR = tmp;

        tmp = ((dataPacket[3] & 0xc0) >> 6) + ((dataPacket[4] & 0x3f) << 2);
        if(tmp != 0)this.RtoRBpm = tmp;


        tmp_data = ((dataPacket[4] & 0xc0) >> 6) + ((dataPacket[5]& 0xff) << 2) + ((dataPacket[6] & 0xff) << 10) + ((dataPacket[7] &0x3f) << 18);
        //tmp_data = this.ecg1;
        tmp_data = total_ecg;


        tmp_data = tmp_data >> 6;
        //Log.i("전체 데이터 : ", Integer.toBinaryString(tmp_data));

        if((tmp_data & 0x20000) != 0x0){
            tmp_data = tmp_data ^ 0x3ffff;
            tmp_data = tmp_data + 0x01;
            tmp_data = -tmp_data;
        }
        this.ecg1 = tmp_data;
        Log.i("계산 : ", Integer.toString(this.ecg1));



    }

}
