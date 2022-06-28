# LoRa32_example

# summry

## 사용 모듈

**TTGO-LORA32-V2.0**

## 프로젝트 개요

### 목적

인터넷이 되지 않는 환경에서 수집한 센서값을 데이터베이스에 업데이트하기 위함

### 구성

![그림1](/assets/1.png)

수집한 센서값을 LoRa통신으로 인터넷에 연결된 Slave LoRa에 전송하고 Slave LoRa는 Firebase와 상호작용함.

## 동작 순서 

1. **Master LoRa → 센서값 수집**
2. **Master LoRa → Slave LoRa에 센서값 전송 (30초 delay)**
3. **Slave LoRa → firebase에 센서값 update (수행 2초, delay 5초)**
4. **Slave LoRa → firebase에서 Max,Min값 Read (수행 1.5초 delay 3초)**
5. **Slave LoRa → Master LoRa에 Max,Min값 전송 (수행 0.5초 delay 없음)**
6. **Master LoRa → Max,Min값과 센서값으로 스마트팜 관리**

> **업로드된 코드에는 센서값을 수집하지 않고 랜덤값으로 사용**

# Setting Guide

## 1. TTGO-LORA32-V1.0.zip 압축해제

## 2. Receiver.ino 열기

## 3. 스케치 → 라이브러리 포함하기 → ZIP 라이브러리 추가

### 3.1 lib_zip 폴더에 있는 ZIP파일 모두 추가

## 4. 파일 → 환경설정 → 보드매니저 URLs

* ```https://dl.espressif.com/dl/package_esp32_index.json``` 추가

![그림2](/assets/2.png)

## 5. 툴 → 보드 → 보드매니저

### 5.1 Arduino AVR Boards 설치

### 5.2 ESP32 Arduino 설치

## 6. 보드, Upload Speed, Flash Frequency 설정

![그림3](/assets/3.png)

### 6.1 포트는 알맞게 설정


# Reference
- lora example code  
[TTGO-LORA32/OLED_LoRa_Sender.ino at master · LilyGO/TTGO-LORA32](https://github.com/LilyGO/TTGO-LORA32/blob/master/OLED_LoRa_Sender/OLED_LoRa_Sender.ino)

- LoRa.h  
[GitHub - sandeepmistry/arduino-LoRa: An Arduino library for sending and receiving data using LoRa radios.](https://github.com/sandeepmistry/arduino-LoRa)

- ssd1306.h  
[GitHub - ThingPulse/esp8266-oled-ssd1306: Driver for the SSD1306 and SH1106 based 128x64, 128x32, 64x48 pixel OLED display running on ESP8266/ESP32](https://github.com/ThingPulse/esp8266-oled-ssd1306)

- LoRa API desc  
[arduino-LoRa/API.md at master · sandeepmistry/arduino-LoRa](https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md)

- FirebaseArduino.h  
[GitHub - FirebaseExtended/firebase-arduino: Arduino samples for Firebase.](https://github.com/FirebaseExtended/firebase-arduino)

- ESP8266.h  
[Arduino IDE can't find ESP8266WiFi.h file](https://stackoverflow.com/questions/50080260/arduino-ide-cant-find-esp8266wifi-h-file)

- 아두이노 파이어베이스 연동  
[아두이노 LoRa Wi-Fi Bluethooth 모듈 라이브러리 설치](https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=wjdrudtn0225&logNo=221374350274)

- firebase-esp32  
[GitHub - mobizt/Firebase-ESP32: 🔥 Firebase RTDB Arduino Library for ESP32. The complete, fast, secured and reliable Firebase Arduino client library that supports CRUD (create, read, update, delete) and Stream operations.](https://github.com/mobizt/Firebase-ESP32)