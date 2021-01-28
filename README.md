# ESP32Cam-PhotoTrap

I wanted to have some cheap (mostly because it can be stolen :( ) way to take some picture of animals in the forest, close to my new (future) home.
ESP32Cam looks like a perfect thinker board for the purpose, sadly pictures are crappy as hell, but it will have to do.

I've found some basic approach to the idea here (https://randomnerdtutorials.com/esp32-cam-pir-motion-detector-photo-capture/) - it work's but I found it not reliable enough and lacking. Anyway my first approach with IR sensor failed because of the snow, perhaps it could be tuned to some extend, but I've switch to microwave sensor.
I've also wanted to have time, when picture was taken, so I've used DS3231 chip from my box of goodies. This required some major changes in the code. And then more changes followed.

## Features
- uses DS3231 to name files with date/time (also sets system time - so file timestamp is also valid), as a bonus - it also measures temperature
- removed white LED - it made the whole ESP32Cam unstable (I've removed transistor - not the led itself - I thought it's gone be easier this way to revert changes)
- added Red led to blink when pictures is taken, when ESP32 shuts off and when there is an error
- added Yellow led to see when microwave sensor is triggered
- firmware can be updated by the .bin file on the SD card (it's much easier this way)
- it does not hang on errors - tries wait and reboot (it helps sometimes)
- it shows errors with blinking

## BOM
- ESP32 Cam board with compatible camera like OV2640
- DS3231 RTC board
- Microwave (RCWL-0516) or PIR sensor with trigger pin
- power supply. I've used 3Ah cellphone battery, Li-ion protection (charge/discharge) board and step-up board to have 5V
- some diodes, switch and hermetic case
- also some resistors, capacitors etc.

## Connecting
Connect DS3231 to ESP32Cam pins 14,15 (shared with MMC)
Connect RCWL-0516 output pin, to ESP32Cam gpio13 over transistor
... this section is not finished :)

## Blinking

### Yelow LED
- yellow light on, microwave sensor is triggered

### Red LED
- on >1s picture is taken (right at that moment)
- 2 fast (150ms) flashes, devices went to deep sleep
- 4 slower flashes - unknown error
- 6 slower flashes - Camera init failed with error: Failed to initialize camera, check connections; check camera
- 7 slower flashes - Picture capture failed: Could not take picture, check camera
- 8 slower flashes - Failed to start SD interface: No sdcard, connection to SD failed
- 9 slower flashes - Card Mount Failed: Wrong SD card type or something wrong with FS on sdcard
- 10 slower flashes - Failed to open file in writing mode: Failed to write picture to sdcard: Card can be full or corrupted

