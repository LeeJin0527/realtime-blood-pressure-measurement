## RealTime Blood Pressure

MAXMIM 사의 손목형 헬스 디바이스 인 MAXREFDES 101과 안드로이드 스마트폰 어플리케이션을 이용하여
사용자의 체온, 심박수(ECG, PPG)를 측정하여 수집한 다음 수치와 실시간 그래프로 보여주고, 파일로 저장
하는 프로젝트입니다.


## Android Application Part
사용 언어 : Java, Kotlin

프로젝트 수행기간 동안은 java로 작업하였습니다. 구현된 버전의 저장소 위치는 다음과 같습니다.  
[링크]!

프로젝트가 끝나고 나서, kotlin으로 구현된 버전은 아래 저장소 위치에 있습니다.  
[링크]!

### Demo 

#### 1. 블루투스 기기 선택화면

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/device_select.jpg" width="40%" height="30%">

BLE를 스캔할 때, 심박수 측정 서비스 기기만 스캔되도록 BLE 서비스 <u>**UUID 필터**</u>를 블루투스 어댑테 객체에 추가한 다음에, 스캔하도록 하였음.

스캔된 디바이스들은 별도의 객체 array list에 저장해놨다가, **recycler view**를 이용해서 보여주도록 해놨음. 소수의 기기만 운용한다는
가정을 해서, 한 화면에 꽉 차는 cardView를 좌우로 스와이프 해서 recycler view를 탐색할 수 있도록 해놨음.

2. 메인 메뉴  

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/menu.jpg" width="40%" height="30%">

각각의 온도, PPG, ECG 버튼을 누르면 해당 fragment로 전환하도록 구현.

bluetooth 연결, 데이터 송수신(write, read, notification characteristic), 연결 종료 처리 및 관련 객체 핸들링은 
백그라운드에서 처리하도록 <u>**Service**</u>(BluetoothLeService)에 관련 동작이 정의되어 있음.

3. 체온 측정  

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/ongoing_temp.jpg" width="40%" height="30%">

체온 측정 fragment에서 실시간으로 그래프를 그려주는 부분은 <u>**mpAndroidChart api**</u>를 사용하였음.

5. 심박수(PPG) 측정  

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/ongoing_ppg.jpg" width="40%" height="30%">

심박수 측정 fragment에서 실시간으로 그래프를 그려주는 부분은 별도의 **<u>UI Thread**</u>에서 일정 인터벌 마다 sleep 한다음,
그래프 객체에 데이터를 추가하여 그리도록 해놨음.

```java

```

초기 일정 시간동안은 디바이스가 ppg를 측정하기 위해 세팅을 하는데, 이 때는 아무 의미 없는 nosie값이 전송됨. 따라서
일정 시간 기다렸다가, 측정 데이터를 보여주도록 해놨음.

표시해야 하는 데이터는 두 종류(grnCnt, grn2Cnt)임. 게다가 한쪽의 오프셋이 크거나, 두 데이터 사이의 차이가 너무 커서 정확한 
수치를 그래프에 표시하지는 않았음. 단순히 경향이나 추세만 확인하는 용도로 그래프를 추가함.

6. 심박수(ECG) 측정  

<img src="https://github.com/LeeJin0527/RealtimeBloodPressureMeasurement/blob/main/documentation/images/ongoing_ecg%202.jpg" width="40%" height="30%">

ECG와 PPG는 블루투스에서 한 번 수신될때 20bytes씩 데이터를 받음. 하지만, ECG는 PPG와 다르게 한 번 수신받을 때 순차적으로 측정된 
4개의 ECG 데이터를 받음. 4개의 ECG 데이터를 한번에 그릴때, 순차적으로 그려주어야 하므로, PPG와 다르게 처리.

```java
// ecg data handling code snippet
this.ecg1 = ((dataPacket[4] & 0xc0) >> 6) + ((dataPacket[5]& 0xff) << 2) + ((dataPacket[6] & 0xff) << 10) + ((dataPacket[7] &0x3f) << 18);
this.ecg2 = ((dataPacket[7] & 0xc0) >> 6) + ((dataPacket[8] & 0xff) << 2) + ((dataPacket[9] & 0xff) << 10) + ((dataPacket[10] & 0x3f) << 18);
this.ecg3 = ((dataPacket[10] & 0xc0) >> 6) + ((dataPacket[11] & 0xff) << 2) + ((dataPacket[12] & 0xff) << 10) + ((dataPacket[13] & 0x3f) << 18);
this.ecg4 = ((dataPacket[13] & 0xc0) >> 6) + ((dataPacket[14] & 0xff) << 2) + ((dataPacket[15] & 0xff) << 10) + ((dataPacket[16] & 0x3f) << 18);
```

7. BLE 연결, 처리 관련 Service

---

## Device Part
사용 언어 : C/C++

대부분의 기능은 제조사에서 이미 구현을 해놓았음. 따라서, 프로젝트를 수행하는 동안 저전력 블루투스 기술 구현 이해를 위해
여러 간단한 코드를 추가하거나 수정해보는 작업을 함.

이외에 간단하게 디바이스 화면을 바꿔본다던지, 동작에 따른 LED 추가/변경, 기능 수행에 필요한 코드 추적을 위한 코드 리뷰등을
수행하였음.



