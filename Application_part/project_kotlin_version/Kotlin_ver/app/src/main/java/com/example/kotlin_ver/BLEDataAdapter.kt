package com.example.kotlin_ver

import android.bluetooth.BluetoothDevice
import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView

class BLEDataAdapter(mBLEDevices : ArrayList<BluetoothDevice>) : RecyclerView.Adapter<ViewHolder>() {
    val mBLEDevices = mBLEDevices
    lateinit var itemClickListener : ItemClickListener

    interface ItemClickListener {
        fun onItemClick(view: View?, position: Int)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val context = parent.context
        val inflater = context.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        val view = inflater.inflate(R.layout.bledeviceitem, parent, false)

        return ViewHolder(view, itemClickListener)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val BLEDeviceName = mBLEDevices[position].name
        val BLEDeviceMAC = mBLEDevices[position].address


        if (BLEDeviceName != null && BLEDeviceName.length > 0) {
            holder.deviceMACTextView.setText(BLEDeviceName)
        } else {
            holder.deviceTextView.setText("기기 이름을 식별할 수 없습니다.")
        }
        holder.deviceMACTextView.setText(BLEDeviceMAC)
        //holder.cardView.setOnClickListener(View.OnClickListener { println("터치터치함!") })
        holder.apply { bind() }
    }

    override fun getItemCount(): Int {
        if(mBLEDevices != null){
            return mBLEDevices.size
        }else{
            return 0
        }
    }

    override fun getItemId(position: Int): Long {
        return position.toLong()
    }


    fun addDevice(device : BluetoothDevice){
        if(!mBLEDevices.contains(device))mBLEDevices.add(device)
    }

    fun getDevice(position: Int) : BluetoothDevice{
        return mBLEDevices.get(position)
    }

    fun clearDevice(){
        mBLEDevices.clear()
    }



}