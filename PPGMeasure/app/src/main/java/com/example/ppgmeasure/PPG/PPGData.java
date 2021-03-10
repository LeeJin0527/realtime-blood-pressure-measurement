package com.example.ppgmeasure.PPG;

import android.util.Log;

import uk.me.berndporr.iirj.Butterworth;

/**
 * 펌웨어 단에서 데이터 구조는 아래와 같습니다
 * 
 * typedef struct __attribute__((packed)) {
 * 		uint32_t start_byte	:8;
 * 		uint32_t sample_cnt	:16;
 * 		uint32_t grnCnt		:20;
 * 		uint32_t grn2Cnt	:20;
 * 		uint32_t x	:14;
 * 		uint32_t y	:14;
 * 		uint32_t z	:14;
 * 		uint32_t hr	:12;
 * 		uint32_t hr_confidence  :8;
 * 		uint32_t rr	:11;
 * 		uint32_t status	:8;
 * 		uint8_t	:0;
 * 		uint8_t crc8:8;
 *        } ds_pkt_data_mode2_compact;
 *
 * status가 activity를 의미하며, 0, 1, 2, 3, 4, 5에 대한 의미는 데이터
 * 시트지를 참조하면 됩니다.
 *
 * 참조한 데이터 시트지를 바탕으로 0, 1, 2, 3, 4, 5는 각각 rest, non, walking,
 * running, biking, rythmic activity로 표시하도록 하였습니다.
 *
 * 위 데이터 구조에 다라, 측정 카운트를 얻고 싶다면 byte(8비트씩) 참조를 해야하므로
 * byte[] packet에서 packet[1] & 0xff + ((packet[2] & 0xff) << 8)을 하면
 * 10진수의 값을 얻을 수 있습니다.
 *
 */
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
/*
    private void applyFilter(){
        double dt = 1.0/25.0;

        int order = 5;
        double fs = 1.0/dt;
        double cutoff = 1/(dt*1000);
        //int fb = 3;

        System.out.println("%l %");
        Butterworth butterworth = new Butterworth();

        butterworth.highPass(order, fs, cutoff);
        this.grnCnt = butterworth.filter((double) (this.grnCnt));
    }
*/
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
        //applyFilter();
    }

}
