package com.example.ppgmeasure;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

import com.example.ppgmeasure.PPG.SocketCommunication;

public class TestActivity extends AppCompatActivity {
    Button sendBtn;
    String ip = "plz enter server ip or dns";
    int port = 0;//enter server process port number

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        View view = getLayoutInflater().from(this).inflate(R.layout.test_layout, null);
        setContentView(view);

        sendBtn = (Button)findViewById(R.id.send_btn);

        sendBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SocketCommunication SC = new SocketCommunication(ip, port);
            }
        });

    }
}
