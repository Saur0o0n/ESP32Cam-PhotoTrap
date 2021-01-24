# ESP32Cam-PhotoTrap

I wanted to have some cheap (mostly because it can be stolen :( ) way to take some picture of animals in the forest, close to my new (future) home.
ESP32Cam looks like a perfect thinker board for the purpose, sadly pictures are crappy as hell, but it will have to do.

I've found some basic approach to the idea here (https://randomnerdtutorials.com/esp32-cam-pir-motion-detector-photo-capture/) - it work's but I found it not reliable enough and lacking. Anyway my first approach with IR sensor failed because of the snow, perhaps it could be tuned to some extend, but I've switch to microwave sensor.
I've also wanted to have time, when picture was taken, so I've used DS3231 chip from my box of goodies. This required some major changes in the code. And then more changes followed.

## Features
- uses DS3231 to name files with date/time (also sets system time - so file timestamp is also valid), as a bonus - it also measures temperature
- removed white LED - it made the whole ESP32Cam unstable (I've removed transistor - not the led itself - I thought it's gone be easier this way to rever changes)
- added Red led to blink when pictures is taken, when ESP32 shut's off and when there is an error
- added Yellow led to see when microwave sensor is triggered
- firmware can be updated by the .bin file on the SD card (it's much easier this way)
- it does not hang on errors - tries wait and reboot (it helps sometimes)
