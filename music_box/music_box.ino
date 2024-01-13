#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "RedMP3.h"

#define MP3_RX 4//RX of Serial MP3 module connect to D7 of Arduino
#define MP3_TX 5//TX to D8, note that D8 can not be used as RX on Mega2560, you should modify this if you donot use Arduino UNO

#define SS_PIN 10
#define RST_PIN 9

#define BLOCK_NUMBER 2

MP3 mp3(MP3_RX, MP3_TX);
int8_t volume; //0~0x1e (30 adjustable level)
int8_t folderName = 0x01;
int8_t songPrefixInt8_t_prev = 0x00;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

int analogPin = A0;
int analogValue = 0;

#define READ_BUFFER_LEN 18
byte readBufferLen = READ_BUFFER_LEN;
byte readBlockData[READ_BUFFER_LEN];

#define SONG_PREFIX_BUFFER_LEN 3

MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;


void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  volume = 1; // Resets volume. It's set by analog input.
}                                                                                                                                                                           
void loop()
{
  analogValue = analogRead(analogPin);
  int8_t newVolume = map(analogValue, 0, 1015, 0x1e, 1);
  if (newVolume > 0x1e) {
    newVolume = 0x1e;
  }
  if (newVolume != volume) {
    volume = newVolume;
    Serial.print("value: ");
    Serial.println(analogValue);
    Serial.print("volume: ");
    Serial.println(volume);
    mp3.setVolume(volume);
  }

  bool isCardPresent = checkIfCardPresent();

  if (!isCardPresent) {
    if (mp3.getStatus() == STATUS_PLAY) {
      mp3.pause();
    }
    return;
  }

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  ReadDataFromBlock(BLOCK_NUMBER, readBlockData);

  byte songPrefixBuffer[SONG_PREFIX_BUFFER_LEN];

  for (int i = 0; i < SONG_PREFIX_BUFFER_LEN; i++) {
    songPrefixBuffer[i] = readBlockData[i];
  }
  Serial.println((char *)songPrefixBuffer);

  String songPrefix = (char *) songPrefixBuffer;
  Serial.println(songPrefix);

  int songPrefixInt = songPrefix.toInt();
  Serial.println(songPrefixInt);
  int8_t songPrefixInt8_t = songPrefixInt;

  delay(50);//you should wait for >=50ms between two plays
  if (songPrefixInt8_t != songPrefixInt8_t_prev) {
    mp3.playWithFileName(folderName, songPrefixInt8_t);
  } else {
    if (mp3.getStatus() == STATUS_STOP) {
      mp3.playWithFileName(folderName, songPrefixInt8_t); // Song has stopped playing, repeat it.
    } else { // Resume playback
      mp3.play();
    }
  }

  songPrefixInt8_t_prev = songPrefixInt8_t;

  delay(1000);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

}

bool checkIfCardPresent()
{
  MFRC522::StatusCode result;
  byte buffer[2];
  byte buffersize = 2;
  bool isCardPresent = false;

  result = mfrc522.PICC_REQA_or_WUPA(mfrc522.PICC_CMD_WUPA, buffer, &buffersize);
  //Serial.println("Presence state:");
  //Serial.println(result, HEX);
  if (result == mfrc522.STATUS_OK)
  {
    //Serial.println("Card is present!");
    isCardPresent = true;
  }
  else
  {
    //Serial.println("Card is not present!");
  }
  mfrc522.PICC_HaltA();
  return isCardPresent;
}


void ReadDataFromBlock(int blockNum, byte readBlockData[])
{
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Authentication success");
  }

  if (readBufferLen != READ_BUFFER_LEN) {
    Serial.println("ERROR: buffer size changed for some reason");
    readBufferLen = READ_BUFFER_LEN;
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &readBufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Block was read successfully");
  }

}
