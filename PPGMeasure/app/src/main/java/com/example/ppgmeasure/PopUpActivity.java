package com.example.ppgmeasure;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.Nullable;

import org.w3c.dom.Text;

public class PopUpActivity extends Activity {
    Button exitButton;
    TextView textView;
    LinearLayout signBanner;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.pop_up_activity_layout);

        exitButton = (Button)findViewById(R.id.exit_pop_up_activity);
        textView = (TextView) findViewById(R.id.pop_up_context);
        signBanner = (LinearLayout)findViewById(R.id.sign_banner_pop_up_activity);

        exitButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        Intent intent = getIntent();
        String data = intent.getStringExtra("data");
        setSignBanner(Integer.parseInt(data));
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if(event.getAction() == MotionEvent.ACTION_OUTSIDE){
            return false;
        }
        else{
            return true;
        }
    }

    @Override
    public void onBackPressed() {
        return;
    }

    public void setSignBanner(int data){
        switch (data){
            case 0:
                textView.setText("안전");
                signBanner.setBackgroundColor(getResources().getColor(R.color.green));
                break;
            case 1:
                textView.setText("주의");
                signBanner.setBackgroundColor(getResources().getColor(R.color.yellow));
                break;
            case 2:
                textView.setText("위험");
                signBanner.setBackgroundColor(getResources().getColor(R.color.red));
                break;
            default:
                break;
        }
    }


}
