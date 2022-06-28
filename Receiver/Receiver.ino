#include <SPI.h>
#include <Wire.h>
#include <SSD1306.h>
#include <LoRa.h>
#include "images.h"

# include  <WiFi.h>
# include  <FirebaseESP32.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//#define LORA_BAND    433
//#define LORA_BAND    868
#define LORA_BAND    915

#define OLED_SDA    4
#define OLED_SCL    15
#define OLED_RST    16

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)


// WIFI
//#define WIFI_SSID "와이파이 이름"
//#define WIFI_PASSWORD "와이파이 비밀번호"

// FIREBASE
//#define FIREBASE_HOST "파이어베이스 url"
//#define FIREBASE_AUTH "파이어베이스 비밀번호"

FirebaseData fbdo;  // firebase data object
FirebaseAuth mAuth;
FirebaseJson json1;
FirebaseJson json2;
FirebaseJson json3;
FirebaseJson json4;
FirebaseJson json5;
FirebaseJson json6;
FirebaseConfig mConfig;

SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

// Forward declarations
void displayLoraData(int packetSize, String packet, String rssi);
void showLogo();

void setup() {
  Serial.begin(115200);

  // WIFI connecting
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connection to Wi-fi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("conneted with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Firebase connecting
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
    // Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(fbdo, 1000 * 60);
    // tiny, small, medium, large and unlimited.
    // Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(fbdo, "tiny");

  Serial.println("-------------------------------------");
  Serial.println("Connected...");

  
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Receiver");

  // Configure OLED by setting the OLED Reset HIGH, LOW, and then back HIGH
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, HIGH);
  delay(100);
  digitalWrite(OLED_RST, LOW);
  delay(100);
  digitalWrite(OLED_RST, HIGH);

  display.init();
  display.flipScreenVertically();
  
  showLogo();
  delay(2000);

  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, "LoRa Receiver");
  display.display();
  
  delay(2000);
  
  // Configure the LoRA radio
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(LORA_BAND * 1E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("init ok");

  // Set the radio into receive mode
  LoRa.receive();
  delay(1500);
}

// ec, flow, humi, pH, temp, water
String Data[6];
float fData[6] = {0,};

void loop() {
  String packet = "";

  int packetSize = LoRa.parsePacket();

  Serial.println();
  Serial.println();
  Serial.println("센서값 수신 대기중..");
  // 패킷 수신 대기
  do{
    packetSize = LoRa.parsePacket();
    if (packetSize) {
      for (int i = 0; i < packetSize; i++) {
        packet += (char)LoRa.read();
      }    
      
      String rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
      //Serial.println(rssi);

      // 수신받은 패킷
      Serial.println("----SPI rcv----");
      Serial.print(packet);
      Serial.println("---------------");

      // 패킷 분할
      char packet_ch[packetSize + 1];
      strcpy(packet_ch, &packet[0]);
      char *result = strtok(packet_ch, "\n");
      int i = 0;
      while(result != NULL){
        Data[i] = result;        
        //printf("%s\n", Data[i]);
        result = strtok(NULL, "\n");
        i++;
      }
      
      break;
      displayLoraData(packetSize, packet, rssi);        
    }else{
      //Serial.println("packet rev failed..");
      delay(10);
    }
  }while(!packetSize);

  // type cast
  for(int i = 0; i < 6; i++){
    fData[i] = Data[i].toFloat();
  }
  
  // data update
  Serial.println("센서값 Firebase에 업데이트중..");
  json1.set("/collection", fData[0]);
  Firebase.updateNode(fbdo, "/ec", json1);
  json2.set("/collection", fData[1]);
  Firebase.updateNode(fbdo, "/flow", json2);
  json3.set("/collection", fData[2]);
  Firebase.updateNode(fbdo, "/humi", json3);
  json4.set("/collection", fData[3]);
  Firebase.updateNode(fbdo, "/pH", json4);
  json5.set("/collection", fData[4]);
  Firebase.updateNode(fbdo, "/temp", json5);
  json6.set("/collection", fData[5]);
  Firebase.updateNode(fbdo, "/waterTemp", json6);
  Serial.println("센서값 Firebase에 업데이트 완료");

  delay(5000);
  
  // data read
  int ecMax;
  int ecMin;
  int humiMax;
  int humiMin;
  int pHMax;
  int pHMin;
  int tempMax;
  int tempMin;
  int fanControl;
  int hotFanControl;
  int ledControl;
  int pumpControl;
  Serial.println("Firebase로부터 제어값 읽는중..");
  ecMax = getEcMax();
  ecMin = getEcMin();
  fanControl = getFanControl();
  hotFanControl = getHotFanControl();
  humiMax = getHumiMax();
  humiMin = getHumiMin();
  ledControl = getLedControl();
  pHMax = getPHMax();
  pHMin = getPHMin();
  pumpControl = getPumpControl();
  tempMax = getTempMax();
  tempMin = getTempMin();
  Serial.println("Firebase로부터 제어값 읽기 완료");  

  delay(3000);
  
  // send packet
  Serial.println("Master LoRa에 제어값 전송중..");
  LoRa.beginPacket();
  LoRa.println(ecMax); 
  LoRa.println(ecMin); 
  LoRa.println(fanControl); 
  LoRa.println(hotFanControl); 
  LoRa.println(humiMax); 
  LoRa.println(humiMin); 
  LoRa.println(ledControl); 
  LoRa.println(pHMax); 
  LoRa.println(pHMin); 
  LoRa.println(pumpControl); 
  LoRa.println(tempMax); 
  LoRa.println(tempMin); 
  LoRa.endPacket();
  Serial.println("Master LoRa에 제어값 전송 완료");
  
}

int getEcMax(){
  int max;

  if (Firebase.getInt(fbdo, "/ec/max")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/ec/max : ");
      max = fbdo.to<int>();
      //Serial.println(max);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return max;
}

int getEcMin(){
  int min;

  if (Firebase.getInt(fbdo, "/ec/min")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/ec/min : ");
      min = fbdo.to<int>();
      //Serial.println(min);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return min;
}

int getFanControl(){
  int control;

  if (Firebase.getInt(fbdo, "/fan/control")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/fan/control : ");
      control = fbdo.to<int>();
      //Serial.println(control);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return control;
}

int getHotFanControl(){
  int control;

  if (Firebase.getInt(fbdo, "/hot_fan/control")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/hot_fan/control : ");
      control = fbdo.to<int>();
      //Serial.println(control);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return control;
}

int getHumiMax(){
  int max;

  if (Firebase.getInt(fbdo, "/humi/max")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/humi/max : ");
      max = fbdo.to<int>();
      //Serial.println(max);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return max;
}

int getHumiMin(){
  int min;

  if (Firebase.getInt(fbdo, "/humi/min")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/humi/min : ");
      min = fbdo.to<int>();
      //Serial.println(min);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return min;
}

int getLedControl(){
  int control;

  if (Firebase.getInt(fbdo, "/led/control")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/led/control : ");
      control = fbdo.to<int>();
      //Serial.println(control);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return control;
}

int getPHMax(){
  int max;

  if (Firebase.getInt(fbdo, "/pH/max")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/pH/max : ");
      max = fbdo.to<int>();
      //Serial.println(max);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return max;
}

int getPHMin(){
  int min;

  if (Firebase.getInt(fbdo, "/pH/min")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/pH/min : ");
      min = fbdo.to<int>();
      //Serial.println(min);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return min;
}

int getPumpControl(){
  int control;

  if (Firebase.getInt(fbdo, "/pump/control")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/pump/control : ");
      control = fbdo.to<int>();
      //Serial.println(control);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return control;
}

int getTempMax(){
  int max;

  if (Firebase.getInt(fbdo, "/temp/max")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/temp/max : ");
      max = fbdo.to<int>();
      //Serial.println(max);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return max;
}

int getTempMin(){
  int min;

  if (Firebase.getInt(fbdo, "/temp/min")) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      //Serial.print("/temp/min : ");
      min = fbdo.to<int>();
      //Serial.println(min);
    }
  }else {
    Serial.println(fbdo.errorReason());
  }
  
  return min;
}

void displayLoraData(int packetSize, String packet, String rssi) {
  String packSize = String(packetSize, DEC);

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0 , 15 , "Received " + packSize + " bytes");
  display.drawStringMaxWidth(0 , 26 , 128, packet);
  display.drawString(0, 0, rssi);
  display.display();
}

void showLogo() {
  uint8_t x_off = (display.getWidth() - logo_width) / 2;
  uint8_t y_off = (display.getHeight() - logo_height) / 2;

  display.clear();
  display.drawXbm(x_off, y_off, logo_width, logo_height, logo_bits);
  display.display();
}
