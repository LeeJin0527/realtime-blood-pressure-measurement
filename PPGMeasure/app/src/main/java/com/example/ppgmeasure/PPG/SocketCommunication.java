package com.example.ppgmeasure.PPG;

import android.os.Environment;
import android.util.Log;

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

    public SocketCommunication(String ip, int port){
        this.ip = ip;
        this.port = port;
        SocketConnection SC = new SocketConnection();
        SC.start();
    }

    public void sendPacket(){
        SocketPacketSend SPS = new SocketPacketSend();
        SPS.start();
    }

    class SocketConnection extends Thread{

        @Override
        public void run(){
            Log.i("Tag", "connection process start");
            try{
                socket = new Socket(ip, port);
                System.out.println(socket.isConnected());

                Log.i("[Socket]", "Socket is created and connection is success");
            }catch (IOException I){
                Log.i("[Socket]", "Socket creation is fail");
                I.printStackTrace();
            }

            try{
                dos = new DataOutputStream(socket.getOutputStream());
                dis = new DataInputStream(socket.getInputStream());
            }catch (IOException I){
                Log.i("[DataIOStream]", "DATA IO stream fail");
                I.printStackTrace();
            }

            SocketPacketSend SPS = new SocketPacketSend();
            SPS.start();
        }
    }

    class SocketPacketSend extends Thread{
        //File myFile = new File("/data/data/com.example.ppgmeasure", "dummy.csv");
        File myFile = new File(Environment.getExternalStorageDirectory(), "PPG.csv");
        byte []fileBytes = new byte[(int)myFile.length()];
        byte[] fileLength = (Integer.toString((int)myFile.length())).getBytes();

        @Override
        public void run() {
            try {
                FileInputStream fis = new FileInputStream(myFile);
                BufferedInputStream bis = new BufferedInputStream(fis);
                bis.read(fileBytes);
                System.out.printf("length : %d\n", fileBytes.length);
                System.out.println(dos);
                dos.writeUTF(Integer.toString((int)myFile.length()));
                dos.write(fileBytes, 0, fileBytes.length);

            }catch (IOException I){
                I.printStackTrace();
                Log.e("Error", "Can't send msg!\n");
            }
        }
    }

    class SocketPacketReceive extends Thread{

    }



}
