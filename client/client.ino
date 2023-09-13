#include <arduino-timer.h>
#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>

#include "keypad.h"
#include "leds.h"
#include "rfid.h"
#include "security.h"

uint8_t destinationAddress[] = {0x58, 0xBF, 0x25, 0x9A, 0x4D, 0x24};
esp_now_peer_info_t peerInfo;
char sendData[250];
uint8_t readData[16];
uint8_t pinData[4];
uint8_t pinCount;

PN532_SPI pn532spi(SPI, 5);
PN532 nfc(pn532spi);

auto timer = timer_create_default();
bool canRead = true;

bool resetRead(void *argument) {
  blinkOff();
  canRead = true;
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&sendData, incomingData, 32);
  memcpy(&sendData[32], readData, 16);
  memcpy(&sendData[48], pinData, 4);
  nfc.PrintHexChar((uint8_t*)sendData, 52);
  esp_now_send(destinationAddress, (uint8_t *) &sendData, 52);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);

  // Output LEDs
  pinMode(LED_PIN, OUTPUT);

  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_pmk((uint8_t *)PMK_KEY_STR);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register peer
  memcpy(peerInfo.peer_addr, destinationAddress, 6);
  peerInfo.channel = 0;  
  for (uint8_t i = 0; i < 16; i++) {
    peerInfo.lmk[i] = LMK_KEY_STR[i];
  }
  peerInfo.encrypt = true;    
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Register send callback function
   esp_now_register_send_cb(OnDataSent);

  // RFID
  nfc.begin();
  nfc.SAMConfig();
}
 
void loop() {
  uint8_t success;
  uint8_t uid[4];
  uint8_t uidLength;
  uint8_t pin[4];

  if (canRead) {
    // Read card
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (success) {
      canRead = false;
      blinkOn();
      pinCount = 0;
      timer.in(5000, resetRead);
  
      // Read data from card
      nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 1, MY_KEY);
      nfc.mifareclassic_ReadDataBlock(4, readData);
      //nfc.PrintHexChar(readData, 16);
    }
  }

  if (!canRead) {
    // Read PIN
    char key = keypad.getKey();
    if (key) {
      pinData[pinCount] = key;
      pinCount++;
      Serial.println(key);
    }
    if (pinCount == 4) {
      // Send magic number request
      strcpy(sendData, "REQ");
      esp_now_send(destinationAddress, (uint8_t *) &sendData, sizeof(sendData));
      // Reset read
      timer.cancel();
      resetRead(NULL);
    }
  }
  timer.tick();
}
