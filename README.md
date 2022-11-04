# Power Switch
The power switcher is a low-cost remote power switch for DC power-supplies between 5-40V. For the post part, it's a simple DC power pass-through with some current-monitoring and the ability to switch the downstream on and off using a virtual terminal via USB connection to a PC. It is also possible to power the down-stream device on and off using a push-button.

The voltage and current are monitored at X frequency. Over-current thresholds are adjustable to suit whatever device is attached down-stream in addition to what device is providing the DC power up-stream. Using a 30V 1A power-supply obviates the need to adjust the current threshold to be near or below 1A or to set an under-voltage threshold above the minimum requirements of the device being powered. It is not trivial to automatically set these thresholds, so these are currently expected to be set manually.

## Goals
This device was created to help replace expensive bench power-supplies without compromising safety or losing remote control entirely.

## Hardware
The hardware is based on the [RP2040](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf) ARM micro-controller. The current is monitored using a TI INA230 device.

## Firmware
Built on the [pico-sdk](https://github.com/raspberrypi/pico-sdk). See the [pico-sdk docs](https://raspberrypi.github.io/pico-sdk-doxygen/index.html).

## Software
Python interface
Unique serial number provides mapping ability for systems with more than one unit attached.

