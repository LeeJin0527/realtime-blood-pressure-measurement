# MAXREFDES101

1. 기술 문서

[MAXIM공식 홈페이지 기술문서링크](https://www.maximintegrated.com/en/design/reference-design-center/system-board/6779.html)

pico adaptor board
micro board의 펌웨어 업데이트에 사용됨

display mode

	1. ECG(심전도) mode
		
		시계를 찬 상태에서, 피부가 electrodes 2와 3에 닿아야함.
		
	2. PPG(광용적맥파) mode
		
		ppg는 측정하려면 시계 뒷면 중앙에 위치한 LED/photodiode 피부가 접촉되어야 함
		
	3. temperature mode
	
	4. info mode

안드로이드 어플로 PPG, temperature, HRM 데이터를 시각적으로 볼 수 있음

측정된 데이터들은 csv파일로 저장 가능

2. 