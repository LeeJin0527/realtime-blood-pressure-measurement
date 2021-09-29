package com.example.heartbeat;

import android.bluetooth.BluetoothDevice;
import android.companion.BluetoothLeDeviceFilter;
import android.content.Context;
import android.content.Intent;
import android.text.Layout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;


public class BLEDataAdapter extends RecyclerView.Adapter<ViewHolder>{

    public interface ItemClickListener{
        void onItemClick(View view, int position);
    }

    private ArrayList<BluetoothDevice> mBLEDevices = null;
    private ItemClickListener itemClickListener = null;



    BLEDataAdapter(ArrayList<BluetoothDevice> mBLEDevices){
        this.mBLEDevices = mBLEDevices;
    }
    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        View view = inflater.inflate(R.layout.bledeviceitem, parent, false);
        ViewHolder viewHolder = new ViewHolder(view, itemClickListener);

        return viewHolder;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int position) {

        final String BLEDeviceName = mBLEDevices.get(position).getName();
        final String BLEDeviceMAC = mBLEDevices.get(position).getAddress();

        if( BLEDeviceName != null && BLEDeviceName.length() > 0){
            viewHolder.DeviceTextView.setText(BLEDeviceName);
        }
        else{
            viewHolder.DeviceTextView.setText("기기 이름을 식별할 수 없습니다.");
        }
        viewHolder.DeviceMACTextView.setText(BLEDeviceMAC);


    }

    @Override
    public int getItemCount() {
        if(mBLEDevices != null) {
            return mBLEDevices.size();
        }
        else{
            return 0;
        }
    }

    public void addDevice(BluetoothDevice device){
        if(!mBLEDevices.contains(device)){
            mBLEDevices.add(device);
        }
    }

    public BluetoothDevice getDevice(int position){
        return mBLEDevices.get(position);
    }

    public void clear(){
        mBLEDevices.clear();
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    public void setItemClickListener(ItemClickListener listener){
        this.itemClickListener = listener;
    }


}

