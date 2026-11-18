#include "Song.h"
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 7

MFRC522 rfid(SS_PIN, RST_PIN);
 
// Specify the block number you want to read
byte targetBlock = 1; 

Song songs[3] = {
  {"cat", "song_1"},
  {"creator", "song_2"},
  {"ocean_man", "iconic_song"}
};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println(F("Scan your MIFARE card..."));
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

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

  // Halt PICC and stop crypto
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(2000);
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

void processSong(String songId) {
  if(!songsContains(songId)) {
    Serial.print("No song found with Id: ");
    Serial.println(songId);
    return;
  }

  Song* song = getSongById(songId);
  if(song == NULL)
    return;

  String songFilePath = song->getSongFilePath();
  Serial.print("Song Id: ");
  Serial.println(songId);
  Serial.print("Song File Path: ");
  Serial.println(songFilePath);
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
    if(songs[i].getSongId().equals(songId))
      return true;
  }

  return false;
}

Song* getSongById(String songId) {
  int songslength = sizeof(songs) / sizeof(songs[0]);
  for(int i = 0; i < songslength; i++) {
    if(songs[i].getSongId().equals(songId))
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




