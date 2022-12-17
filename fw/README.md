Based on Wokwi simulated version: https://wokwi.com/projects/348371928277320274:

 - 3 Common-cathode seven-sgement displays
 - 3 shift registers (74HC595)
 - Analog inputs on pin 26 & 27

Seven segment digits are shifted into the most-significant digit first.


## Pinout
| Pin      | Description     |
| -------- | --------------- |
| 26       | Analog ch0      |
| 27       | Analog ch1      |
| 8        | Serial data     |
| 9        | Serial clock    |
| 10       | Serial latch    |


## Build Instructions
1. Git clone the [pico-sdk](https://github.com/raspberrypi/pico-sdk).
1. Set environment variable export PICO_SDK_PATH=<path>
1. cd fw/build
1. cmake ../
1. make
1. Copy .uf2 to drive or use .hex or .bin
