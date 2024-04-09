#include <SPI.h>
#include <Adafruit_GFX.h>            // Core graphics library
#include <Adafruit_ST7735.h>      // Hardware-specific library
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h> 


// Add external font
#include "Font4x7Fixed.h"

// Pins for nodeMCU esp32 board
#define __CS 4
#define __RST 16
#define __DC 17
#define __LED 2
#define ROTATION 2 

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

Adafruit_ST7735 tft = Adafruit_ST7735(__CS,  __DC, __RST);

String BytesToStrOpt(const uint8_t* b, uint32_t size) {
  String str;
  char buf[3];
  
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) {
      str += '0';
    }
    
    sprintf(buf, "%02X", b[i]);
    str += buf;
    if (i < size - 1) {
      str += ':';
    }
  }

  return str;
}


typedef struct {
  String ssid;
  int rssi;
  uint8_t ch;
  uint8_t bssid[6];
}  _Network;

_Network _networks[32]; // 32 = maximum number of networks

void clearArray() {
  for (int i = 0; i < 32; i++) { // 32 = maximum number of networks
    _Network _network;
    _networks[i] = _network;
  }
}

int NumOfNet; 

void performScan() {
  
  Serial.println();
  Serial.print("[======== ! Scanning networks ========]");
  
  digitalWrite(__LED, HIGH);
  int wtick = 0;
  int n = WiFi.scanNetworks();
  NumOfNet = n; 
  clearArray();
  digitalWrite(__LED, LOW);
  
  if (n >= 0) {
    Serial.println();
    Serial.print("! "); Serial.print(n); Serial.print(" networks found:"); 
    
    for (int i = 0; i < n && i < 32; ++i) { // 32 = maximum number of networks
      _Network network;
      network.ssid = WiFi.SSID(i);
      network.rssi = WiFi.RSSI(i);
      
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }
      
      network.ch = WiFi.channel(i);
      _networks[i] = network;  
      
      wtick+=1;
      Serial.println();
      Serial.print(wtick); Serial.print("-> "); 
      Serial.print("SSID: "); Serial.print(network.ssid); 
      Serial.print(" RSSI: "); Serial.print(network.rssi); 
      Serial.print(" CH: "); Serial.print(network.ch);
      Serial.print(" BSSID: ");
      
      for (int j = 0; j < 6; j++) {
        Serial.print(network.bssid[j], HEX); 
        
        if (j < 5) {
          Serial.print(":");
        }
      }
    }
  }
  Serial.println();
  Serial.print("[================================]");
}

void DispNetwOnScreen(int BSSID_FLAG) { // BSSID_FLAG = 1 => display networks with BSSID
  
  tft.fillScreen(0x0000);
  tft.fillRect(0, 0, 128, 7, BLUE);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.println();
  tft.print("~ "); tft.print(NumOfNet); tft.print(" networks found ~"); 
  tft.setCursor(0, 9);
  
  for (int x = 0; x < 32; x++) { // 32 = maximum number of networks
    if (_networks[x].ssid == "") {
      break;
    } else {
      tft.println();
      tft.setTextColor(GREEN); tft.print(x+1); tft.print(" ");
      tft.setTextColor(CYAN); tft.print(_networks[x].ssid); tft.print(" ");

      if (_networks[x].rssi <= -80) {
        tft.setTextColor(RED);
      } else if (_networks[x].rssi >= -80 && _networks[x].rssi <= -70) {
        tft.setTextColor(YELLOW);
      } else {
        tft.setTextColor(GREEN);
      }
      tft.print(_networks[x].rssi); tft.print(" ");
      tft.setTextColor(WHITE); tft.print(String(_networks[x].ch)); tft.print(" ");

      if (BSSID_FLAG == 1) {
        tft.println();
        tft.print("   BSSID -> "); tft.print(BytesToStrOpt(_networks[x].bssid, 6));
      }
    }
  }
}

void DispBSSID_ONLY() {
  
  tft.fillScreen(0x0000);
  tft.fillRect(0, 0, 128, 7, BLUE);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.println();
  tft.print("~ "); tft.print(NumOfNet); tft.print(" networks found ~"); 
  tft.setCursor(0, 9);

  for (int x = 0; x < 32; x++) {
    if (_networks[x].ssid == "") {
      break;
    } else {
      tft.println();
      tft.setTextColor(GREEN); tft.print(x+1); tft.print(" "); 
      tft.setTextColor(WHITE); tft.print("BSSID -> "); tft.print(BytesToStrOpt(_networks[x].bssid, 6));
    }
  }
}

void setup() {
  
  Serial.setTxBufferSize(1024);
  Serial.begin(115200);

  pinMode(__LED, OUTPUT);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(ROTATION);
  tft.setFont(&Font4x7Fixed);

}

unsigned long PerfScanTime = 0;
unsigned long DispNetTime = 0;

void loop() {
  if (millis() - PerfScanTime >= 5*1000) { // call func every 5 seconds
    performScan(); 
    PerfScanTime = millis();
  }

  if (millis() - DispNetTime >= 2*1000) { // call func every 2 seconds
    DispNetwOnScreen(0);
    DispNetTime = millis(); 
  }

}
