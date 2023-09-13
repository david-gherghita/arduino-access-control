
#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"

PN532_SPI pn532spi(SPI, 5);
PN532 nfc(pn532spi);

void setup(void) {
  Serial.begin(115200);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 4) {
      // We probably have a Mifare Classic card ... 
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
	  
      uint8_t DEFAULT_KEY[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
      uint8_t MY_KEY[6] = {0x2B, 0x4E, 0x92, 0xFD, 0xB7, 0x25};
      uint8_t DEFAULT_BITS[4] = {0xff, 0x07, 0x80, 0x69};
      uint8_t MY_BITS[4] = {0x7F, 0x07, 0x88, 0x69};
      uint8_t my_data[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x80, 0x69, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
      uint8_t user1_data[16] = {0x57, 0xF7, 0xDA, 0xF6, 0xEE, 0x0A, 0x9A, 0xFC, 0xCB, 0x33, 0xAB, 0xD8, 0xA5, 0xFD, 0x5C, 0xD7};
      uint8_t user2_data[16] = {0x28, 0x63, 0x45, 0x97, 0x37, 0x16, 0x98, 0xC5, 0x95, 0x85, 0x4E, 0x12, 0x83, 0x15, 0xA7, 0x3E};

      uint8_t result[16] = {1};


      for (int index = 3; index <= 63; index += 4) {
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, index, 1, MY_KEY);
        success = nfc.mifareclassic_WriteDataBlock (index, my_data);
        success = nfc.mifareclassic_ReadDataBlock(index, result);
        //Serial.println("Reading Block " + String(15));
        Serial.print(String(index) + " ");
        nfc.PrintHexChar(result, 16);
        for (int i = 0; i < 16; i += 1) {
          result[i] = 1;
        }
        delay(10);
      }
      
/*
      for (int i = 0; i < 16; i++) {
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, i * 4 + 3, 1, DEFAULT_KEY);
  	  
        if (success) {
          
  		    //uint8_t data[16] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x80, 0x69, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
          uint8_t data[16] = {0x2B, 0x4E, 0x92, 0xFD, 0xB7, 0x25, 0x7F, 0x07, 0x88, 0x69, 0x2B, 0x4E, 0x92, 0xFD, 0xB7, 0x25};
          success = nfc.mifareclassic_WriteDataBlock (i * 4 + 3, data);
          success = nfc.mifareclassic_ReadDataBlock(i * 4 + 3, data);
  		
          if (success) {
            Serial.println("Reading Block " + String(i * 4 + 2));
            nfc.PrintHexChar(data, 16);
            Serial.println("");
  		  
            //delay(1000);
          } else {
            Serial.println("ERROR: BLOCK NOT AUTHENTICATED");
            //delay(3000);
          }
        } else {
          Serial.println("ERROR: WRONG KEY");
          //delay(3000);
        }
      }*/
    }
    delay(10000);
  }
}
