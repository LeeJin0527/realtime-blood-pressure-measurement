package com.example.ppgmeasure.PPG;

import android.os.Environment;
import android.util.Log;

import androidx.annotation.Nullable;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.SocketAddress;

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


    class SocketConnection extends Thread {

        @Override
        public void run() {
            Log.i("Tag", "connection process start");
            try {
                socket = new Socket(ip, port);
                System.out.println(socket.isConnected());

                Log.i("[Socket]", "Socket is created and connection is success");
            } catch (IOException I) {
                Log.i("[Socket]", "Socket creation is fail");
                I.printStackTrace();
            }

            try {
                dos = new DataOutputStream(socket.getOutputStream());
                dis = new DataInputStream(socket.getInputStream());
            } catch (IOException I) {
                Log.i("[DataIOStream]", "DATA IO stream fail");
                I.printStackTrace();
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

    class SocketPacketSend extends Thread {
        File myFile = new File("/data/data/com.example.ppgmeasure", "dummy.csv");
        //File myFile = new File(Environment.getExternalStorageDirectory(), "PPG.csv");
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
