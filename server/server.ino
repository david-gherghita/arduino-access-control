#include <esp_now.h>
#include <WiFi.h>

#include "leds.h"
#include "random.h"
#include "rfid.h"
#include "security.h"

uint8_t destinationAddress[] = {0x30, 0xC6, 0xF7, 0x2F, 0x2A, 0x60};
esp_now_peer_info_t peerInfo;
char recvData[250];
char sendData[250];

// START DEBUG
void printHex(uint8_t num) {
  char hexCar[2];
  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}
// END DEBUG

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  if (strncmp(recvData, "REQ", 3) == 0) {
    generateRandomChallenge();
    memcpy(sendData, (void*)randomChallenge, sizeof(randomChallenge));
    Serial.println(sizeof(randomChallenge));
    esp_now_send(destinationAddress, (uint8_t *) &randomChallenge, sizeof(sendData));
  } else {
    if (memcmp(recvData, randomChallenge, 32) == 0) {
      Serial.println("CHALLENGE OK");
      if (memcmp(recvData + 32, USER_1_DATA, 16) == 0) {
        if (memcmp(recvData + 48, USER_1_PIN, 4) == 0) {
          blinkGreen();
        } else {
          blinkRed();
        }
      } else if (memcmp(recvData + 32, USER_2_DATA, 16) == 0) {
        if (memcmp(recvData + 48, USER_2_PIN, 4) == 0) {
          blinkGreen();
        } else {
          blinkRed();
        }
      }
    } else {
      Serial.println("CHALLENGE FAILED");
    }
    for (int i = 0; i < 52; i++) {
      printHex(recvData[i]);
    }
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);

  // Output LEDs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Init Random
  randomSeed((analogRead(RANDOM_PIN)));

  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_pmk((uint8_t *)PMK_KEY_STR);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register Peer
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
}

void loop() {
  delay(1000);
}
