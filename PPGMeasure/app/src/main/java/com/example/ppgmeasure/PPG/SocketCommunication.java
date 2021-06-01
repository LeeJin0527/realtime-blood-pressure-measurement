package com.example.ppgmeasure.PPG;

import android.os.Environment;
import android.util.Log;

import androidx.annotation.Nullable;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.Socket;
import java.net.SocketAddress;

public class SocketCommunication{
    private String ip = null;
    private int port = 0;
    private Socket socket = null;
    private DataInputStream dis = null;
    private DataOutputStream dos = null;
    private String deviceID = null;
    private String SBP = null;
    private String DBP = null;



    public SocketCommunication(String ip, int port, String deviceID, String SBP, String DBP){
        this.ip = ip;
        this.port = port;
        this.deviceID = deviceID;
        this.SBP = SBP;
        this.DBP = DBP;
        ProcessSocketCommunication PSC = new ProcessSocketCommunication();
        PSC.start();
    }

    public class ProcessSocketCommunication extends Thread{
        @Override
        public void run() {
            byte[] msg = new byte[30];
            int count = 2;
            try {
                socket = new Socket(ip, port);
                System.out.println(socket.isConnected());
                System.out.println("success to connected by socket!");
                Log.i("[Socket]", "Socket is created and connection is success");
            } catch (IOException I) {
                Log.i("[Socket]", "Socket creation is fail");
                I.printStackTrace();
            }

            try {
                dos = new DataOutputStream(socket.getOutputStream());
                dis = new DataInputStream(socket.getInputStream());
                System.out.println("dos and dis success to create" + dos.toString() +"\n" + dis.toString());
            } catch (IOException I) {
                Log.i("[DataIOStream]", "DATA IO stream fail");
                I.printStackTrace();
            } catch (NullPointerException N){
                N.printStackTrace();
                Log.i("[NULL]","소켓쪽에서 null pointer error!");
            }

            try{
                dos.writeUTF(deviceID);
                System.out.println("deviceID 전송 완료");
            }catch (IOException I){
                I.printStackTrace();
                Log.i("[IOException]", "device id sending error!");
            }

            try{
                dis.read(msg);
                System.out.println("sync 수신 완료");
            }catch (IOException I){
                I.printStackTrace();
                Log.i("[IOException]", "sync receive error!");
            }


            try{
                dos.writeUTF("SBP");
                Thread.sleep(1000);
                dos.writeUTF(SBP);
                System.out.println("SBP 전송 완료");
            }catch (IOException I){
                I.printStackTrace();
                Log.i("[IOException]", "SBP sending error!");
            }catch (InterruptedException In){
                In.printStackTrace();
            }

            try{
                dis.read(msg);
                System.out.println("sync 수신 완료");
            }catch (IOException I){
                I.printStackTrace();
                Log.i("[IOException]", "sync receive error!");
            }

            try{
                dos.writeUTF("DBP");
                Thread.sleep(1000);
                dos.writeUTF(DBP);
                System.out.println("DBP 전송 완료");
            }catch (IOException I){
                I.printStackTrace();
                Log.i("[IOException]", "DBP sending error!");
            }catch (InterruptedException In){
                In.printStackTrace();
            }

            try{
                dis.read(msg);
                System.out.println("sync 수신 완료");
            }catch (IOException I){
                I.printStackTrace();
                Log.i("[IOException]", "sync receive error!");
            }
            //while (count > 9){
                System.out.println("파일 전송 시작 " + String.valueOf(count) );
                String filename = "PPG" + String.valueOf(count) + ".csv";
                File myFile = new File(Environment.getExternalStorageDirectory(), filename);
                byte[] fileBytes = new byte[(int) myFile.length()];
                byte[] fileLength = (Integer.toString((int) myFile.length())).getBytes();
                try {
                    FileInputStream fis = new FileInputStream(myFile);
                    BufferedInputStream bis = new BufferedInputStream(fis);
                    bis.read(fileBytes);
                    System.out.printf("length : %d\n", fileBytes.length);
                    System.out.println(dos);
                    dos.writeUTF("CSV_Data");
                    dos.flush();

                    try{
                        dis.read(msg);
                        System.out.println("sync 수신 완료");
                    }catch (IOException I){
                        I.printStackTrace();
                        Log.i("[IOException]", "sync receive error!");
                    }

                    dos.writeUTF(Integer.toString((int) myFile.length()));
                    System.out.println("파일 길이 전송 완료");
                    try{
                        dis.read(msg);
                        System.out.println("sync 수신 완료");
                    }catch (IOException I){
                        I.printStackTrace();
                        Log.i("[IOException]", "sync receive error!");
                    }
                    dos.flush();
                    dos.write(fileBytes, 0, fileBytes.length);
                    System.out.println("파일 전송 완료");
                    dos.flush();
                    count++;

                    try{
                        dis.read(msg);
                    }catch (IOException I){
                        I.printStackTrace();
                        Log.i("[IOException]", "sync receive error!");
                    }


                }catch (FileNotFoundException F){
                    F.printStackTrace();
                }catch (IOException I){
                    I.printStackTrace();
                }


            //}
    }




/*
 *
 *
 * 전체적인 소켓 통신 수행해야함
 *           client               server
 *
 *       디바이스 id 전송
 *                               디바이스 id 전송 받음 sync 전송
 *       sync 받음
 *       SBP 전송
 *                               SBP 전송 받음, sync 전송
 *       sync 받음
 *       DBP 전송
 *                               DBP 전송 받음, sync 전송
 *       sync 받음
 *
 *       1. 전송할 파일 크기전송
 *                               2. 크기 받음, sync 전송
 *       3. sync 받음
 *       4. 파일 전송
 *                               5. 파일 다 전송받음, sync 전송
 *       6. sync 받음
 *               1-6번 과정을 9번 수행
 *       소켓 연결 종료 전송
 */
 }
}



/*
public class SocketCommunication {
    private String ip = null;
    private int port = 0;
    private Socket socket = null;
    private DataInputStream dis = null;
    private DataOutputStream dos = null;
    private int dataFromServer = -1;
    SocketPacketReceive SPR;
    Thread t;
    String msg;

    public SocketCommunication(String ip, int port) {
        this.ip = ip;
        this.port = port;
        SocketConnection SC = new SocketConnection();
        SC.start();
    }

    public void closeSocket() {
        msg = "close";
        SocketMsgSend SMS = new SocketMsgSend();
        SMS.start();

    }

    public int getDataFromServer() {
        return dataFromServer;
    }

    public int isConnect() {
        if (socket != null && socket.isConnected()) return 1;
        else return 0;
    }

    public void sendPacket() {
        SocketPacketSend SPS = new SocketPacketSend();
        SPS.start();
    }

    public void sendSBP(String SBPdata, String id){
        SocketMsgSendSBP SBP = new SocketMsgSendSBP(SBPdata, id);
        SBP.start();
    }



    class SocketConnection extends Thread {

        @Override
        public void run() {
            Log.i("Tag", "connection process start");
            try {
                socket = new Socket(ip, port);
                System.out.println(socket.isConnected());
                System.out.println("success to connected by socket!");
                Log.i("[Socket]", "Socket is created and connection is success");
            } catch (IOException I) {
                Log.i("[Socket]", "Socket creation is fail");
                I.printStackTrace();
            }

            try {
                dos = new DataOutputStream(socket.getOutputStream());
                dis = new DataInputStream(socket.getInputStream());
                System.out.println("dos and dis success to create" + dos.toString() +"\n" + dis.toString());
            } catch (IOException I) {
                Log.i("[DataIOStream]", "DATA IO stream fail");
                I.printStackTrace();
            } catch (NullPointerException N){
                N.printStackTrace();
                Log.i("[NULL]","소켓쪽에서 null pointer error!");
            }


            SPR = new SocketPacketReceive();
            SPR.start();
        }
    }

    class SocketMsgSend extends Thread {
        @Override
        public void run() {
            try {
                dos.writeUTF(msg);
                SPR.interrupt();
                socket.close();
            } catch (IOException I) {
                I.printStackTrace();
            }
        }
    }

    class SocketMsgSendSBP extends Thread{
        String SBP;
        String Id;

        SocketMsgSendSBP(String SBP, String id) {
            this.SBP = SBP;
            this.Id = id;
        }

        @Override
        public void run() {
            try {
                System.out.println("기기 고유 id" + this.Id);
                dos.writeUTF(this.Id);
                Thread.sleep(500);
                dos.writeUTF("SBP");
                Thread.sleep(1000);
                dos.writeUTF(this.SBP);
            } catch (IOException I) {
                I.printStackTrace();
            }catch (InterruptedException e){
                e.printStackTrace();
            }
        }
    }

    class SocketPacketSend extends Thread {
        //File myFile = new File("/data/data/com.example.ppgmeasure", "PPG.csv");
        File myFile = new File(Environment.getExternalStorageDirectory(), "PPG.csv");
        byte[] fileBytes = new byte[(int) myFile.length()];
        byte[] fileLength = (Integer.toString((int) myFile.length())).getBytes();

        @Override
        public void run() {
            try {
                FileInputStream fis = new FileInputStream(myFile);
                BufferedInputStream bis = new BufferedInputStream(fis);
                bis.read(fileBytes);
                System.out.printf("length : %d\n", fileBytes.length);
                System.out.println(dos);
                dos.writeUTF("CSV_Data");
                dos.flush();
                try {
                    Thread.sleep(1000);
                }catch (InterruptedException e){
                    e.printStackTrace();
                }
                dos.writeUTF(Integer.toString((int) myFile.length()));
                try {
                    Thread.sleep(2000);
                }catch (InterruptedException e){
                    e.printStackTrace();
                }
                dos.flush();
                dos.write(fileBytes, 0, fileBytes.length);
                dos.flush();

            } catch (IOException I) {
                I.printStackTrace();
                Log.e("Error", "Can't send msg!\n");
            }

        }
    }

    class SocketPacketReceive extends Thread {
        @Override
        public void run() {
            while (!Thread.currentThread().isInterrupted()) {
                try {
                    byte[] datatest = new byte[30];
                    dis.read(datatest);
                    dataFromServer = Character.getNumericValue(datatest[0]);
                } catch (IOException I) {
                    I.printStackTrace();
                    System.out.println("receive error!");
                }
            }
            System.out.println("Interrupted!");
        }
    }


}
*/