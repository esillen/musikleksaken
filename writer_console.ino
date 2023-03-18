#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include "RedMP3.h"

#define MP3_RX 7//RX of Serial MP3 module connect to D7 of Arduino
#define MP3_TX 8//TX to D8, note that D8 can not be used as RX on Mega2560, you should modify this if you donot use Arduino UNO
MP3 mp3(MP3_RX, MP3_TX);
int8_t index  = 0x01;//the first song in the TF card
int8_t volume = 0x1a;//0~0x1e (30 adjustable level)

#define RST_PIN 9
#define SS_PIN 10

#define BLOCK_NUMBER 2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


byte writeBufferLen = 16;
byte writeBlockData[16];


byte readBufferLen = 18;
byte readBlockData[18];

MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;


void setup() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  // put your setup code here, to run once:
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;


}

void loop() {

  delay(500);

  Serial.println("Approximate your card to the reader...");


  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  // put your main code here, to run repeatedly:
  Serial.println("Card detected!!!");

  ReadDataFromBlock(BLOCK_NUMBER, readBlockData);
  PrintBuffer(readBlockData, readBufferLen);

  Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
  // Ask personal data: Family name
  Serial.println(F("Schreibe bitte die drei ziffern!! Bitte mit # enden!!"));
  byte len = Serial.readBytesUntil('#', (char *) writeBlockData, writeBufferLen) ; // read from serial
  for (byte i = len; i < writeBufferLen; i++) writeBlockData[i] = ' ';     // pad with spaces

  WriteDataToBlock(BLOCK_NUMBER, writeBlockData);


  ReadDataFromBlock(BLOCK_NUMBER, readBlockData);
  PrintBuffer(readBlockData, readBufferLen);

  delay(500);
  Serial.println(F("\n**End Reading**\n"));
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
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
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
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
