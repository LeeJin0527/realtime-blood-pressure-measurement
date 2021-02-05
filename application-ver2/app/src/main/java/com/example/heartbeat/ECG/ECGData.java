package com.example.heartbeat.ECG;

public class ECGData {
    private byte[] packet;
    private int count;
    private int RtoR;
    private int RtoRBpm;
    private int ecg1;
    private int ecg2;
    private int ecg3;
    private int ecg4;

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
        byte[] dataPacket = this.packet;
        this.count = dataPacket[1] & 0xff;

        tmp = (dataPacket[2] & 0xff) + ((dataPacket[3] & 0x3f) << 8);
        if(tmp != 0)this.RtoR = tmp;

        tmp = ((dataPacket[3] & 0xc0) >> 6) + ((dataPacket[4] & 0x3f) << 2);
        if(tmp != 0)this.RtoRBpm = tmp;

        //this.RtoR = (dataPacket[2] & 0xff) + ((dataPacket[3] & 0x3f) << 8);
        //this.RtoRBpm = ((dataPacket[3] & 0xc0) >> 6) + ((dataPacket[4] & 0x3f) << 2);

        this.ecg1 = ((dataPacket[4] & 0xc0) >> 6) + ((dataPacket[5]& 0xff) << 2) + ((dataPacket[6] & 0xff) << 10) + ((dataPacket[7] &0x3f) << 18);
        //tmp = ((dataPacket[4] & 0xc0) >> 6) + ((dataPacket[5]& 0xff) << 2) + ((dataPacket[6] & 0xff) << 10) + ((dataPacket[7] &0x1f) << 18);
        //if((dataPacket[7] & 0x20) != 0)this.ecg1 = - tmp;
        //else this.ecg1 = tmp;

        this.ecg2 = 1;
        this.ecg3 = 1;
        this.ecg4 = 1;
    }

}
