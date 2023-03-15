#include <MFRC522.h>

/*
 * 
 * All the resources for this project: https://randomnerdtutorials.com/
 * Modified by Rui Santos
 * 
 * Created by FILIPEFLOP
 * 
 */
 
#include <SPI.h>

#include <SoftwareSerial.h>
#include "RedMP3.h"

#define MP3_RX 7//RX of Serial MP3 module connect to D7 of Arduino
#define MP3_TX 8//TX to D8, note that D8 can not be used as RX on Mega2560, you should modify this if you donot use Arduino UNO
MP3 mp3(MP3_RX, MP3_TX);
int8_t volume = 0x1e; //0~0x1e (30 adjustable level)
int8_t folderName = 0x01;
int8_t songPrefixInt8_t_prev = 0x00;

#define SS_PIN 10
#define RST_PIN 9

#define BLOCK_NUMBER 2

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

int analogPin = A0;
int analogValue = 0;

#define WRITE_BUFFER_LEN 16
byte writeBufferLen = WRITE_BUFFER_LEN;
byte writeBlockData[WRITE_BUFFER_LEN];

#define READ_BUFFER_LEN 18
byte readBufferLen = READ_BUFFER_LEN;
byte readBlockData[READ_BUFFER_LEN];

MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;


void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;


}
void loop() 
{

  /*analogValue = analogRead(analogPin);
  int8_t newVolume = map(analogValue, 10, 1015, 0, 0x1e);
  if (newVolume > 0x1e) {
    newVolume = 0x1e;
  }
  if (newVolume != volume) {
      volume = newVolume;
      Serial.print("volume: ");
      Serial.println(volume);
      mp3.setVolume(volume);
    }
*/
  // NOTE: This is for one without potentiometer
  mp3.setVolume(24); // max 30

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  

  bool isCardPresent = checkIfCardPresent();

  if (!isCardPresent){
    mp3.pause();
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

byte songPrefixBuffer[6];

for (int i =0; i < 6; i++){
  songPrefixBuffer[i] = readBlockData[i];
}
  Serial.println((char *)songPrefixBuffer);

  String songPrefix = (char *) songPrefixBuffer;
  Serial.println(songPrefix);

  int songPrefixInt = songPrefix.toInt();
  Serial.println(songPrefixInt);
  int8_t songPrefixInt8_t = songPrefixInt;

  delay(50);//you should wait for >=50ms between two plays
  if (songPrefixInt8_t != songPrefixInt8_t_prev){
    mp3.playWithFileName(folderName, songPrefixInt8_t);
  } else {
    mp3.play();
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




void WriteDataToBlock(int blockNum, byte blockData[]) 
{
  /* Authenticating the desired data block for write access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Authentication success");
  }

  
  /* Write data to the block */
  status = mfrc522.MIFARE_Write(blockNum, blockData, writeBufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Data was written into Block successfully");
  }
  
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

void PrintBuffer(byte buffer [], int bufferLen) {
for (int j=0 ; j<bufferLen ; j++)
   {
     Serial.write(buffer[j]);
   }
  Serial.print("\n");  
}
