# RealTime Blood Pressure

MAXMIM 사의 손목형 헬스 디바이스 인 MAXREFDES 101과 안드로이드 스마트폰 어플리케이션을 이용하여
사용자의 체온, 심박수(ECG, PPG)를 측정하여 수집한 다음 수치와 실시간 그래프로 보여주고, 파일로 저장
하는 프로젝트입니다.


# Android Application Part
사용 언어 : Java, Kotlin

[프로젝트 소스 저장소 위치](https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/tree/main/Application_part/project_kotlin_version/Kotlin_ver)

# 어플리케이션 구조
<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/project_description.jpg" width="100%" height="100%">



***

### 1. 블루투스 기기 선택화면

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/device_select.jpg" width="40%" height="30%">

BLE를 스캔할 때, 심박수 측정 서비스 기기만 스캔되도록 BLE 서비스 <u>**UUID 필터**</u>를 블루투스 어댑테 객체에 추가한 다음에, 스캔하도록 하였음.

스캔된 디바이스들은 별도의 객체 array list에 저장해놨다가, **recycler view**를 이용해서 보여주도록 해놨음. 소수의 기기만 운용한다는
가정을 해서, 한 화면에 꽉 차는 cardView를 좌우로 스와이프 해서 recycler view를 탐색할 수 있도록 함.

***

### 2. 메인 메뉴  

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/menu.jpg" width="40%" height="30%">

각각의 온도, PPG, ECG 버튼을 누르면 해당 fragment로 전환하도록 구현.

bluetooth 연결, 종료 처리, 데이터 송수신(write, read, notification characteristic) 및 BLE 처리 이벤트는
백그라운드에서 처리하도록 <u>**Service**</u>(BluetoothLeService)에 관련 동작을 구현함.
<br><br>


***

### 3. 체온 측정  

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/ongoing_temp.jpg" width="40%" height="30%">

체온 측정 fragment에서 실시간으로 그래프를 그려주는 부분은 <u>**mpAndroidChart api**</u>를 사용하였음.

***

### 4. 심박수(PPG) 측정  

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/ongoing_ppg.jpg" width="40%" height="30%">

심박수 측정 fragment에서 실시간으로 그래프를 그려주는 부분은 블루투스 브로드캐스트 리시버에서 데이터를 수신받을 때 마다 
별도의 **<u>UI Thread**</u>에서
그래프 객체에 데이터를 그리도록 해놨음. 심박수 데이터는 한번만 보내는 것이 아니라, 일정 간격으로 알림(notification)처럼 
데이터가 수신됨. 따라서, 콜백처럼 구현하기 위해, 각각의 fragment에서 그래프를 그리는 메서드를 호출하는게 아니라 
브로드캐스트 리시버에서 처리하도록 해놨음.


```kotlin
private val mGattUpdateReceiver: BroadcastReceiver = object : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        val action = intent.action
        if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
            mConnected = true
        } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
            mConnected = false
        } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
            setupGattCharacteristic(mBluetoothLeService!!.getSupportedGattServices())
        } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {

            val stringData = intent.getStringExtra(BluetoothLeService.EXTRA_DATA)
            val byteData = hexaStringToByteArray(stringData)

            when(mode){
                "TEMP" -> {
                    fragmentTemp.addData(byteData)
                }

                "ECG" -> {
                    fragmentECG.addData(byteData)
                }

                "PPG" -> {
                    fragmentPPG.addData(byteData)
                }
            }
        }
    }
}
```

<br><br>



표시해야 하는 데이터는 두 종류(grnCnt, grn2Cnt)임. 게다가 한쪽의 데이터 크기의 오프셋이 크거나, 
두 데이터 사이의 차이가 너무 커서 제한된 휴대폰 해상도에서는 그래프가 직선처럼 보이는 문제가 발생했었음. 
PPG 그래프는 데이터의 추세 및 패턴을 확인하기 위한 목적을 가지고 있으므로, 아래와 같이 한쪽 값은 고정하고
다른 쪽 값은 상대적인 차이값을 이용해 두 그래프를 한번에 나타내도록 하였음.

```kotlin
if(num1 > num2){
    max = (float) num1
    min = (float) num2
    diff = (max - min)*0.01f
    num1 = num2 + diff
    max = (float) num1

}
else{
    max = (float) num2
    min = (float) num1
    diff = (max - min)*0.01f
    num2 = num1 + diff
    max = (float) num2
}
```

***

### 5. 심박수(ECG) 측정  

<p float="left">
<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/ongoing_ecg%202.jpg" width="45%" height="30%"><img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/ongoing_ecg2_play.gif" width="45%" height="30%">
</p>

ECG와 PPG는 블루투스에서 한 번 수신될때 20bytes씩 데이터를 받음. 하지만, ECG는 PPG와 다르게 한 번 수신받을 때 순차적으로 측정된 
4개의 ECG 데이터를 받음. 4개의 ECG 데이터를 한번에 그릴때, 순차적으로 그려주어야 하므로, PPG와 다르게 데이터를 처리 해야함.

```kotlin
// ecg data handling code snippet
this.ecg1 = ((dataPacket[4] & 0xc0) >> 6) + ((dataPacket[5]& 0xff) << 2) + ((dataPacket[6] & 0xff) << 10) + ((dataPacket[7] &0x3f) << 18);
this.ecg2 = ((dataPacket[7] & 0xc0) >> 6) + ((dataPacket[8] & 0xff) << 2) + ((dataPacket[9] & 0xff) << 10) + ((dataPacket[10] & 0x3f) << 18);
this.ecg3 = ((dataPacket[10] & 0xc0) >> 6) + ((dataPacket[11] & 0xff) << 2) + ((dataPacket[12] & 0xff) << 10) + ((dataPacket[13] & 0x3f) << 18);
this.ecg4 = ((dataPacket[13] & 0xc0) >> 6) + ((dataPacket[14] & 0xff) << 2) + ((dataPacket[15] & 0xff) << 10) + ((dataPacket[16] & 0x3f) << 18);
```

***

### 6. BLE 연결, 처리 관련 Service  

구글 예제를 참조하여, 기능 구현을 해봄.  
(https://android.googlesource.com/platform/development/+/f8a92396babb6592bb8780866def23795f3dab70/samples/BluetoothLeGatt/src/com/example/bluetooth/le/BluetoothLeService.java)

  
블루투스를 이용해 데이터를 송수신할 때는 관련 프로토콜에 따라 GATT에 정의되어 있는 동작(characteristic)인 read, write, notification
에 대한 정의가 필요함. 해당 동작에 대한 객체, 매서드 정의, notification을 위한 callback method, 그리고 descriptor 정의등이 있음.


---

# Device Part
사용 언어 : C/C++

대부분의 기능은 제조사에서 이미 구현을 해놓았음. 따라서, 프로젝트를 수행하는 동안 저전력 블루투스 기술 구현 이해를 위해
여러 간단한 코드를 추가하거나 수정해보는 작업을 함.

이외에 간단하게 디바이스 화면을 바꿔본다던지, 동작에 따른 LED 추가/변경, 기능 수행에 필요한 코드 추적을 위한 코드 리뷰등의 리버스 엔지니어링을
수행하였음.



