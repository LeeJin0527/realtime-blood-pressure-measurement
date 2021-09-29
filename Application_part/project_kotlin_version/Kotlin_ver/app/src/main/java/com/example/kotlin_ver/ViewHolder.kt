package com.example.kotlin_ver

import android.view.View
import android.widget.TextView
import androidx.cardview.widget.CardView
import androidx.recyclerview.widget.RecyclerView


class ViewHolder(itemView : View, listener : BLEDataAdapter.ItemClickListener) : RecyclerView.ViewHolder(itemView) {
    val deviceTextView : TextView = itemView.findViewById(R.id.device_item_name)
    val deviceMACTextView : TextView = itemView.findViewById(R.id.device_item_mac)
    val cardView : CardView = itemView.findViewById(R.id.card_view)
    val listener = listener

    fun bind() {
        cardView.setOnClickListener(
            object : View.OnClickListener {
                override fun onClick(v: View) {
                    val position: Int = getAdapterPosition()
                    if (position != RecyclerView.NO_POSITION) {
                        listener.onItemClick(v, position)
                    }
                }
            })
    }

}