# The music box

## The music box hardware

### You will need

- Arduino (I use Nano because it is small and easy to build into something)
- RC522 RFID reader/writer
- LED for status
- 10K Ohm potentiometer for controlling the volume. I use one with a on/off switch so that I can control power with it as well as volume
- A power bank or other 5V power source.
- OPEN-SMART serial mp3 player with speaker and micro SD card
- A bunch of RFID tags. I use stickers because they are easy to put in toys.

### Wire schema 

#### from Arduino:

D4 -> MP3 RX
D5 -> MP3 TX

D9 -> RFID RST
D10 -> RFID SDA
D11 -> RFID MOSI
D12 -> RFID MISO
D13 -> RFID SCK

A0 -> Potentiometer middle pin

3.3v -> RFID 3.3V
5v -> Common 5v
GND -> Common Ground 


#### When done wiring the Arduino, wire the following:

Common Ground -> RFID GND
Common Ground -> MP3 GND
Common Ground -> Potentiometer GND
Common Ground -> LED Cathode
Common 5v -> MP3 5V
Common 5v -> Potentiometer 5V
Common 5v -> LED Anode (via 330 Ohm resistor. Don't forget this!)

## Prepare Music
On the SD card, arrange the music files so they are in a folder called 01. 

Then name the mp3 files in that folder like so
```
001_song.mp3
002_another_song.mp3
003_a_third_song.mp3
639_yet_another_song_number_skips_does_not_matter.mp3
...
```

## The writer(s)
There are two method of writing to the RFID tags. Using the console and using hardware. 


### The console writer 
1. Wire everything up and upload `writer_console.ino` to your Arduino. Turn on the Arduino console.
2. Place a RFID tag on the reader. The console should wait for input.
3. Write three digits (your input) followed by a `#`, for example `012#` and press enter.
4. The three digits should now be written to the RFID!

### The hardware writer
This method is more complicated and requires extra hardware but is useful if you have many tags that you need to write to quickly.

#### You will need
- Arduino (I use Nano)
- RFID reader
- SSD1306 OLED display
- EC11 Rotary Encoder

#### Wire schema

D2 -> EC11 A
D3 -> EC11 B

D6 -> EC11 pushbutton

D9 -> RFID RST
D10 -> RFID SDA
D11 -> RFID MOSI
D12 -> RFID MISO
D13 -> RFID SCK

A4 -> OLED SDA
A5 -> OLED SCL

3.3v -> RFID 3.3v
GND -> Common Ground -> OLED, RFID and EC11 GND
5V -> Common 5v -> OLED and EC11 5v

#### Using the hardware writer
Upload `writer_hardware.ino` to your Arduino.
The OLED screen will light up and display "Scanning...". Place a RFID tag on the reader and it will change to displaying the previously written number (empty if nothing is written) and the "number to be written".

Turn the dial until you reach the desired value, then press the button and the number will be written to the RFID tag!


## Troubleshooting

### A specific song will not play
Make sure your songs are proper mp3 files. Your computer may still play them but the redmp3 hardware requires the files to be correct.

Try them in isolation by just playing that song using the REDMP3 library. If that does not work, your song must be converted to proper mp3. I cannot help but there are great resources online.
If it does work, check the console to see if the RFID tag corresponds to that song.

### The led was very bright and then broke
You must wire it in series with a resistor. I use 330 Ohm. More Ohms also work for less brightness.

### The reader is not working!
Check the wiring. Do a smallest viable test, then consult the internet. One of my 20 or so tested RC522 had a hardware fault so that is something that happens.

### Writing to a specific RFID tag does not work!
The RFID protocol is outside the scope of this guide but the code assumes that the RFID tag is fresh from the factory and was not tampered with, and has the standard RFID key FFFFFFFFFFFFh.
