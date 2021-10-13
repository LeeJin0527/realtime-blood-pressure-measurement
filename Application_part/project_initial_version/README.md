## BLE 디바이스 식별/ 연결/ 데이터 패킷 송수신 디버깅등 프로젝트 초기에 대한 내용입니다.

***

![](../documentation/images/1.JPG)
블루투스 장치 Scanning 구현

scanning시 MAXREFDES 101 장비만 스캔되도록 filtering이 필요함.

***

![](../documentation/images/2.JPG)

BLE가 연결된 상태에서, 온도에 대한 문자열 command("read temp 0")를 write를 구현한 뒤 확인.

중간에 spinner 옆에 해당 대응하는 write characteristic의 UUID가 표시되도록 함. 

***

![](../documentation/images/3.JPG)

온도 버튼을 클릭하면 정상적으로 write되어 디바이스(시계)에서 데이터를 보낼 수 있음을 확인(좌측 시리얼 통신 프로그램)

마찬가지로 stop 커맨드도 정상적으로 write되어 센싱 중단 수행을 확인.

***

## 영상

[![notify시연](http://img.youtube.com/vi/4-1Gxlas5qk/0.jpg)](https://www.youtube.com/watch?v=4-1Gxlas5qk) 

센싱 측정 command를 보내면, 디바이스에서는 계속해서 알아서 데이터를 보내는 notification characteristic이 필요함. notify 기능을 어플리케이션에 
추가한 뒤 notify callBack이 일어날때 마다 Data TextView가 변경되도록 표시함.

***

## References

1. https://stackoverflow.com/questions/35436891/ble-android-write-characteristic-data

2. https://github.com/Fakher-Hakim/android-BluetoothLeGatt

3. https://developer.android.com/guide/topics/connectivity/bluetooth-le?hl=ko

4. https://jung-max.github.io/2019/09/06/Android-BLE-overview/

5. https://stackoverflow.com/questions/27068673/subscribe-to-a-ble-gatt-notification-android