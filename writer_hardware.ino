#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define ENCODER_A 2
#define ENCODER_B 3

#define PUSHBUTTON 6

#define RST_PIN 9
#define SS_PIN 10

#define BLOCK_NUMBER 2



MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
//byte writeBufferLen = 16;
char writeBlockData[4];
byte readBufferLen = 18;
byte readBlockData[18];
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int selected_song_number;
byte read_string[3];

int encoder_A_state;
int encoder_B_state;
int encoder_A_prev = 0;

int pushButtonState = 0;

#define STATE_THING_OFF 0
#define STATE_THING_ON 1
#define STATE_WRITTEN 2
int state = STATE_THING_OFF;

void setup() {

  selected_song_number = 0;
  
  Serial.begin(9600);
  SPI.begin();      // Initiate  SPI bus
  Serial.println("Done spi and serial init.");

   //////// INIT ENCODER
   pinMode(PUSHBUTTON, INPUT_PULLUP);
   pinMode(ENCODER_A, INPUT_PULLUP);
   pinMode(ENCODER_B, INPUT_PULLUP);
   
  Serial.println("Done encoder init.");
  
  //////// INIT OLED ////////
   // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  delay(2000);         // wait for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(2);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 10);        // position to display
  oled.println("Starting..."); // text to display
  oled.display();               // show on OLED

  Serial.println("Done oled init.");
  
  ////////// MFRC522 SETUP
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.

  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  
  mfrc522.PCD_Init();   // Initiate MFRC522

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  Serial.println("Done mfrc522 init.");
  
  attachInterrupt(0, doEncoder, CHANGE);
}


void loop() {
  
  PrintOLED();
  
  if (state == STATE_THING_OFF) {
    delay(500);

    Serial.println("Approximate your card to the reader...");
   
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
  
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    bool read_something = ReadDataFromBlock(BLOCK_NUMBER, readBlockData);
    if (read_something) {
      state = STATE_THING_ON;
      return;
    } else {
      delay(500);
      Serial.println(F("\n**End Reading**\n"));
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();  
    }
  } 
  
  
  else if (state == STATE_THING_ON) {
    
    pushButtonState = !digitalRead(PUSHBUTTON); // Opposite for some reason??!
    if (pushButtonState) {
      Serial.println("Pushed button to write");
      sprintf (writeBlockData, "%03i", selected_song_number);
      WriteDataToBlock(BLOCK_NUMBER, (byte *)writeBlockData);
      state = STATE_WRITTEN;
      return;
    }
  }
  
  
  else if (state == STATE_WRITTEN) {
      ReadDataFromBlock(BLOCK_NUMBER, readBlockData);
      delay(500);
      Serial.println(F("\n**End Reading**\n"));
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      state = STATE_THING_OFF;
  }

 
/*
  WriteDataToBlock(BLOCK_NUMBER, writeBlockData);
  
  ReadDataFromBlock(BLOCK_NUMBER, readBlockData);
  */
 
  

}

void doEncoder()
{
  encoder_A_state = digitalRead(ENCODER_A);
  encoder_B_state = digitalRead(ENCODER_B);
  if((!encoder_A_state) && (encoder_A_prev)){
      // A has gone from high to low 
      if(encoder_B_state) {
        // B is high so clockwise
        // increase the brightness, dont go over 255
        selected_song_number -= 1;      
      }   
      else {
        // B is low so selected_song_numberer-clockwise      
        // decrease the brightness, dont go below 0
        selected_song_number += 1;             
      }
      if (selected_song_number > 999) {
        selected_song_number = 0;
      } else if (selected_song_number < 0) {
          selected_song_number = 999;  
      }
    }
    encoder_A_prev = encoder_A_state;     // Store value of A for next time 
}


void PrintOLED() {
  oled.clearDisplay();

  if (state == STATE_THING_OFF) {
    oled.setCursor(0, 10);
    oled.println("SCANNING...");
  }
  else if (state == STATE_THING_ON) {
    oled.setCursor(0, 10);
    oled.println(String((char *)read_string));

    oled.setCursor(40, 10);
    oled.println("->");
    
    oled.setCursor(70, 10);
    oled.println(selected_song_number);
  }
  oled.display();
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


bool ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return false;
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
    return false;
  }
  else
  {
     Serial.println("Block was read successfully");
     for (int i = 0; i < 3; i++) {
      read_string[i] = readBlockData[i];
      Serial.write(read_string[i]);
    }
    Serial.println();
   
    return true;
  }
  
}