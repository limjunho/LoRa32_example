#include <SPI.h>
#include <Wire.h>
#include <SSD1306.h>
#include <LoRa.h>
#include "images.h"

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

SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

// Forward declarations
void displayLoraData(String countStr);
void showLogo();

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Transmitter");

  // Configure the LED an an output
  pinMode(LED_BUILTIN, OUTPUT);

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
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, "LoRa Transmitter");
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
}

// ecMax, ecMin, fanControl, hotFanControl, humiMax, humiMin, ledControl, pHMax, pHMin, pumpControl, tempMax, tempMin
String Data[12];
int iData[12] = {0,};

void loop() {
  float ec = random(50);
  float flow = random(50);
  float humi = random(50);
  float pH = random(50);
  float temp = random(50);
  float waterTemp = random(50);
  
  static int counter = 0;

  // send packet
  Serial.println();
  Serial.println();
  Serial.println("Slave LoRa에 센서값 전송중..");
  LoRa.beginPacket();
  LoRa.println(ec);
  LoRa.println(flow);
  LoRa.println(humi);
  LoRa.println(pH);
  LoRa.println(temp);  
  LoRa.println(waterTemp);  
  LoRa.endPacket();
  Serial.println("Slave LoRa에 센서값 전송 완료");

  String countStr = String(counter, DEC);
  Serial.print("sequence : ");
  Serial.println(countStr);

  displayLoraData(countStr);

  // toggle the led to give a visual indication the packet was sent
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);

  counter++;

  delay(3000);

  String packet = "";

  int cnt = 0;
  int packetSize;
  Serial.println("제어값 수신 대기중..");
  do{
    packetSize = LoRa.parsePacket();
    if (packetSize) {
      for (int i = 0; i < packetSize; i++) {
        packet += (char)LoRa.read();
      }    
      
      String rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
      //Serial.println(rssi);

      // 수신받은 패킷
      /*
      Serial.println("----SPI rcv----");
      Serial.print(packet);
      Serial.println("---------------");
      */

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
      //displayLoraData(packetSize, packet, rssi);        
    }else{
      delay(10);
      // 30초이상 데이터 수신이 없는 경우 수신대기 중지
      if(cnt >= 3000){
        break;
      }else{
        cnt++;
      }      
    }
  }while(!packetSize);

  // type cast
  for(int i = 0; i < 12; i++){
    iData[i] = Data[i].toInt();
  }

  Serial.print("exMax : ");
  Serial.println(iData[0]);
  Serial.print("exMin : ");
  Serial.println(iData[1]);
  Serial.print("fanControl : ");
  Serial.println(iData[2]);
  Serial.print("hotFanControl : ");
  Serial.println(iData[3]);
  Serial.print("humiMax : ");
  Serial.println(iData[4]);
  Serial.print("humiMin : ");
  Serial.println(iData[5]);
  Serial.print("ledControl : ");
  Serial.println(iData[6]);
  Serial.print("pHMax : ");
  Serial.println(iData[7]);
  Serial.print("pHMin : ");
  Serial.println(iData[8]);
  Serial.print("pumpControl : ");
  Serial.println(iData[9]);
  Serial.print("tempMax : ");
  Serial.println(iData[10]);
  Serial.print("tempMin : ");
  Serial.println(iData[11]);

  Serial.println("30초 후 센서값 전송");
  delay(30000);
}

void displayLoraData(String countStr) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  
  display.drawString(0, 0, "Sending packet: ");
  display.drawString(90, 0, countStr);
  display.display();
}

void showLogo() {
  uint8_t x_off = (display.getWidth() - logo_width) / 2;
  uint8_t y_off = (display.getHeight() - logo_height) / 2;

  display.clear();
  display.drawXbm(x_off, y_off, logo_width, logo_height, logo_bits);
  display.display();
}
