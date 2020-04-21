#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

const uint32_t NUM_READERS = 8;

//Hardware GPIO pin assignments
const uint32_t PN532_SCK_PIN =  13;
const uint32_t PN532_MISO_PIN = 12;
const uint32_t PN532_MOSI_PIN = 11;
const uint32_t PN532_SS1_PIN =  3;
const uint32_t PN532_SS2_PIN =  4;
const uint32_t PN532_SS3_PIN =  5;
const uint32_t PN532_SS4_PIN =  6;
const uint32_t PN532_SS5_PIN = 7;
const uint32_t PN532_SS6_PIN = 8;
const uint32_t PN532_SS7_PIN = 9;
const uint32_t PN532_SS8_PIN = 10;

//The tags that will allow device to open
const uint32_t ACCEPTED_TAG_ID1 = 3019359746;
const uint32_t ACCEPTED_TAG_ID2 = 1138753795;
const uint32_t ACCEPTED_TAG_ID3 = 2747364354;
const uint32_t ACCEPTED_TAG_ID4 = 1131808771;
const uint32_t ACCEPTED_TAG_ID5 = 1944383746;
const uint32_t ACCEPTED_TAG_ID6 = 334337283;
const uint32_t ACCEPTED_TAG_ID7 = 2749166338;
const uint32_t ACCEPTED_TAG_ID8 = 2581401742;

bool isLocked = true; // Init system state as locked

//An array of the chip select pins for each RFID reader
const uint32_t SS_PINS[NUM_READERS] = {PN532_SS1_PIN, PN532_SS2_PIN, PN532_SS3_PIN, PN532_SS4_PIN, PN532_SS5_PIN, PN532_SS6_PIN, PN532_SS7_PIN, PN532_SS8_PIN};  //

//An array of the accepted RFID tags for each RFID reader
const uint32_t NFC_ACCEPTED_TAGS[NUM_READERS] = {ACCEPTED_TAG_ID1, ACCEPTED_TAG_ID2, ACCEPTED_TAG_ID3, ACCEPTED_TAG_ID4, ACCEPTED_TAG_ID5, ACCEPTED_TAG_ID6, ACCEPTED_TAG_ID7, ACCEPTED_TAG_ID8};   //

//An array of RFID readers
Adafruit_PN532 *nfcReaders[NUM_READERS];


/**************************************************************************/
void setup(void) {
  Serial.begin(38400);
  Serial.println("Hallo! Het programma is van start!");

  for (int x = 0; x < NUM_READERS; x++)
  {
    nfcReaders[x] = new Adafruit_PN532(PN532_SCK_PIN, PN532_MISO_PIN, PN532_MOSI_PIN, SS_PINS[x]);
    nfcReaders[x]->begin();
    
    uint32_t versiondata = 9999;
    while (versiondata == 9999) {
      versiondata = nfcReaders[x]->getFirmwareVersion();
      Serial.print("Didn't find PN53x board #");
      Serial.println(x);
    }
    
    // Initialisation check
    Serial.print("Found chip PN5");
    Serial.println(String((versiondata >> 24) & 0xFF, HEX));
    Serial.print("Firmware ver. ");
    Serial.print(String((versiondata >> 16) & 0xFF, DEC));
    Serial.print('.');
    Serial.println(String((versiondata >> 8) & 0xFF, DEC));

    nfcReaders[x]->SAMConfig();
    nfcReaders[x]->setPassiveActivationRetries(1);
  }
      // Initialisation complete
      Serial.println("All borden zijn geïnitialiseerd");
      Serial.println();
}

/**************************************************************************/
void loop(void) {
  if (isLocked == true)
  {
    uint32_t readNfcTags[NUM_READERS];
    uint32_t countOfMatchingTags = 0;

    for (int w = 0; w < NUM_READERS; w++)
    {
      readNfcTags[w] = 0;
    }

    for (int x = 0; x < NUM_READERS; x++) {
      uint8_t success;
      uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
      uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
      uint32_t cardid = 0;

      Serial.print("READER #");
      Serial.println(x+1);

      success = nfcReaders[x]->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
      if (success) {
        // Display some basic information about the card
        Serial.print("Found an ISO14443A card with ");
        Serial.print("UID Value: ");
        nfcReaders[x]->PrintHex(uid, uidLength);

        if (uidLength == 4)
        {
          // We probably have a Mifare Classic card ...
          cardid = uid[0];
          cardid <<= 8;
          cardid |= uid[1];
          cardid <<= 8;
          cardid |= uid[2];
          cardid <<= 8;
          cardid |= uid[3];
        }
      }

      Serial.print("tag ID#");
      Serial.println(cardid);
      readNfcTags[x] = cardid;
      Serial.println();
      delay(500);
    }

    // Count the amount of matching tags
    for (int z = 0; z < NUM_READERS; z++) {
      if (readNfcTags[z] == NFC_ACCEPTED_TAGS[z])
      {
        countOfMatchingTags++;
      }
    }
    
    // Unlock if all tags match
    if (countOfMatchingTags == NUM_READERS) {
      Serial.println("Access granted");
      isLocked = false;
    } else {
      Serial.println("Access denied");
      isLocked = true;
    }
  }
}
