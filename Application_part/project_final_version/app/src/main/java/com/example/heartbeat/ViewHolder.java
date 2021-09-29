package com.example.heartbeat;

import android.view.View;
import android.widget.TextView;

import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;

public class ViewHolder extends RecyclerView.ViewHolder{
    //LinearLayout linearLayout;
    CardView cardView;
    TextView DeviceTextView;
    TextView DeviceMACTextView;

    ViewHolder(View itemView, BLEDataAdapter.ItemClickListener listener){
        super(itemView);

        DeviceTextView = itemView.findViewById(R.id.device_item_name);
        DeviceMACTextView = itemView.findViewById(R.id.device_item_mac);
        cardView = itemView.findViewById(R.id.card_view);

        cardView.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                int position = getAdapterPosition();

                if(position != RecyclerView.NO_POSITION){
                    listener.onItemClick(v, position);
                }
            }
        });
        /*
        itemView.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                int position = getAdapterPosition();

                if(position != RecyclerView.NO_POSITION){
                    listener.onItemClick(v, position);
                }
            }
        });
*/

    }
}
