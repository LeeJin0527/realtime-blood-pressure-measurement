** void WatchInterface::updateDisplay(int batteryLevelToBeScaled)

display에 관한 전반적인 동작담당.
작은 font는 snprintf()로 표시할 수 있음.
호출되기전에 display.locate()를 통해 출력할 위치를 정할 수 있음.



1. 블루투스 connection 시도 

2. watchInterface.BLE_Interface_Exists가 true로 바뀜 (-> 어디서? 누가? )

3. dsInterface.stopcommand() 호출해서 모든 sensor들을 중단
	
	stopcommand()함수 가보면, 모든 센서에 대해서 센서개수만큼 stop함수 호출함
	
4. watchInterface.BLE_Interface_Exists = BLE::Instance().gap().getState().connected;

5. if (watchInterface.BLE_Interface_Exists || watchInterface.USB_Interface_Exists) 

6. 이 사이에 더 있는거 같음

7. process_ble();

여기서부터 연결을 기다리는거 같음

1. BLE::Instance().waitForEvent();
	
	mbed-os.features.FEATURE_BLE.source.(BLE.cpp)

2. 


일단 only 블루투스 connection과 disconnection만 됨

## Connection 성공

![connected](./images/conn.jpg)

## Disconnection 성공(또는 평상시 연결이 안된 상태)

![disconnected](./images/disconn.jpg)




Drivers.Interfaces.DSInterface.cpp

Utilities.WatchInterface.cpp