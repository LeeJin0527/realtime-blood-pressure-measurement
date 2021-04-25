package com.example.ppgmeasure;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.example.ppgmeasure.PPG.SocketCommunication;

public class TestActivity extends AppCompatActivity {
    Button sendBtn;
    String ip = "220.81.117.44";
    int port = 1234;//enter server process port number
    SocketCommunication SC;
    int currentData = -1;
    LinearLayout signBanner;
    Button connectionClose = null;

    @Override
    protected void onDestroy() {
        super.onDestroy();
        SC.closeSocket();
    }

    @Override
    public void onBackPressed() {
        SC.closeSocket();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        View view = getLayoutInflater().from(this).inflate(R.layout.test_layout, null);
        setContentView(view);

        signBanner = (LinearLayout)findViewById(R.id.banner);

        sendBtn = (Button)findViewById(R.id.send_btn);
        connectionClose = (Button)findViewById(R.id.close_btn);
        SC = new SocketCommunication(ip, port);

        setServerData SD = new setServerData();
        Thread t = new Thread(SD);
        t.start();

        connectionClose.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SC.closeSocket();
                t.interrupt();
            }
        });

        sendBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SC.sendPacket();
            }
        });

    }

    public void changeSignBanner(int data){
        System.out.printf("data in chageSignBanner = %d\n", data);
        switch (data){
            case 1:
                signBanner.setBackgroundColor(getResources().getColor(R.color.black));
                System.out.println("Black!");
                break;
            case 2:
                signBanner.setBackgroundColor(getResources().getColor(R.color.white));
                System.out.println("White!");
                break;
            case 3:
                signBanner.setBackgroundColor(getResources().getColor(R.color.purple_500));
                System.out.println("Purple!");
                break;
            default:
                break;
        }
    }

    // 이 부분은 나중에 여건되면, 고치는게 좋을듯함
    // 스레드로 1초마다 확인하지말고, 인터럽트나 콜백이나 다른 방법으로
    // 좀 더 우아하게 서버 데이터를 비동기적으로 확인하는 방법이 필요
    class setServerData implements Runnable{
        int tmp;
        @Override
        public void run() {
            while(!Thread.currentThread().isInterrupted()){

                    tmp = SC.getDataFromServer();
                    if (tmp != currentData) {
                        currentData = tmp;
                        changeSignBanner(currentData);
                    }
                    if (tmp == -2) break;
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException I) {
                        I.printStackTrace();
                    }

            }
            System.out.println("thread terminated");
        }
    }

}
