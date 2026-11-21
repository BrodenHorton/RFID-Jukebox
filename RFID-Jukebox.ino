#include "Song.h"
#include <SPI.h>
#include <MFRC522.h>
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN 7

#define FPSerial softSerial

int MAX_MISSED_READS = 3;

MFRC522 rfid(SS_PIN, RST_PIN);

SoftwareSerial softSerial(4, 5); // Parameters are (arduino_rx, arduino_tx)
DFRobotDFPlayerMini myDFPlayer;
 
// Specify the block number you want to read
byte targetBlock = 1; 

bool isCardPresent = false;
int missedReadsCounter = 0;

Song songs[3] = {
  {"cat", 1},
  {"creator", 2},
  {"ocean_man", 3}
};

void setup() {
  FPSerial.begin(9600);
  Serial.begin(9600);
  SPI.begin();

  rfid.PCD_Init();
  Serial.println(F("Scan your MIFARE card..."));

  initDFPlayerMini();
}

void initDFPlayerMini() {
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */false)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(24);
}

void loop() {
  bool isCardDetected = (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) || tryWakingCard();

  if(!isCardPresent) {
    if(isCardDetected) {
      isCardPresent = true;

      Serial.println(F("Card detected."));

      if(!isAuthenticated())
        return;

      // Read data from the block
      byte buffer[18];
      byte size = sizeof(buffer);
      MFRC522::StatusCode status = rfid.MIFARE_Read(targetBlock, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(rfid.GetStatusCodeName(status));
        return;
      }

      printBlockAsHex(buffer);

      String songId = getBlockStringValue(buffer);
      processSong(songId);
      Serial.println();
    }
  }
  else {
    if(!isCardDetected) {
      isCardPresent = false;
      Serial.println("Card pulled away");
      Serial.println();
    }
  }

  //Halt PICC and stop crypto
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(500);
}

bool isAuthenticated() {
  // MIFARE Classic cards require authentication for each sector
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF; // Standard default factory key
  }

  MFRC522::StatusCode status;

  // Determine which sector the target block belongs to
  byte sector = targetBlock / 4;
  byte trailerBlock = sector * 4 + 3;

  // Authenticate using Key A
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  return true;
}

bool tryWakingCard() {
  // Buffer required by PICC_WakeupA to store the 2-byte ATQA response
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // WUPA wakes both IDLE (new) and HALT (sleeping) cards
  MFRC522::StatusCode status = rfid.PICC_WakeupA(bufferATQA, &bufferSize);

  return status == MFRC522::STATUS_OK;
}

void processSong(String songId) {
  if(!songsContains(songId)) {
    Serial.print("No song found with Id: ");
    Serial.println(songId);
    return;
  }

  Song* song = getSongById(songId);
  if(song == NULL)
    return;

  int songFileIndex = song->getFileIndex();
  Serial.print("Song Id: ");
  Serial.println(songId);
  Serial.print("Song File Index: ");
  Serial.println(songFileIndex);
}

String getBlockStringValue(byte* buffer) {
  byte terminatingBuffer[17];
  for (byte i = 0; i < 16; i++) {
    terminatingBuffer[i] = buffer[i];
  }
  terminatingBuffer[16] = 0x00;

  return String((char*)terminatingBuffer);
}

bool songsContains(String songId) {
  int songslength = sizeof(songs) / sizeof(songs[0]);
  for(int i = 0; i < songslength; i++) {
    if(songs[i].getId().equals(songId))
      return true;
  }

  return false;
}

Song* getSongById(String songId) {
  int songslength = sizeof(songs) / sizeof(songs[0]);
  for(int i = 0; i < songslength; i++) {
    if(songs[i].getId().equals(songId))
      return &songs[i];
  }

  Serial.print("Invalid songId argument in getSongId: ");
  Serial.println(songId);

  return NULL;
}

void printBlockAsString(byte* buffer) {
  Serial.print(F("Data in block "));
  Serial.print(targetBlock);
  Serial.print(F(": "));
  for (byte i = 0; i < 16; i++) {
    Serial.write(buffer[i]);
  }
  Serial.println();
}

void printBlockAsHex(byte* buffer) {
  Serial.print(F("Hex: "));
  for (byte i = 0; i < 16; i++) {
    if (buffer[i] < 0x10) {
      Serial.print(F("0"));
    }
    Serial.print(buffer[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
}




