# pillScope Plus
Oscilloscope based around the STM32F401 Black Pill and a color LCD screen, meant to be used as an educational tool
![The pillScope in its case](https://user-images.githubusercontent.com/60291077/177203708-384191ef-0c0f-4918-a163-46cf7b4721da.jpg)

## Why does it exist?
The goal of this project was to create a simple and easy to build but still very usable oscilloscope. I wanted to learn the basics of how a **digital storage oscilloscope** functions, while also ending up with something which can be used as an educational tool in the lab. A short description of how oscilloscopes (and this one in particular) work can be found [here](HowItWorks.md).

## Features
* -3.3V to 3.3V input range (can be increased if using attenuator probes)
* 1MOhm input impedance
* 10uS/div minimum timebase
* 1.6 MSa/S sampling rate
* On screen measurements:
  * min/max voltage
  * peak-to-peak voltage
  * frequency
* SCPI interface over UART
## Required parts
### Base parts:
* STM32F401CC Black Pill development board
* 128x160 ST7735-based TFT display
* 3 pushbuttons
* LM358 dual op-amp (rail-to-rail opamps should work better in this context, but this is what I had on hand)
* 2x 68kOhm resistors (to create a 1.65V offset voltage)
* 2x 500kOhm resistors (to create the input attenuator)

### Useful, but not mandatory:
* a 5V power supply
* an opto-isolated USB UART adapter
* a BNC connector, for using proper oscilloscope probes

## Schematics
The LCD screen is connected to SPI1 (PB3-SCK, PB5-MOSI, PB12-CS, PB13-RST, PB14-DC). The buttons are connected as follows:
* PB9: Up
* PB8: Select
* PB7: Down

The output of the analog frontend is connected to ADC1_IN0, which corresponds to PA0.\
The schematic of the frontend can be found in the [frontend.pdf](frontend.pdf) file.

## Using the oscilloscope
### The UI
The Select button cycles through the different parameters, which can be adjusted using the Up and Down buttons. 

### Auto calibration
Pressing Up and Down at the same time triggers the auto-calibration function. The tip and ground clip of the probed should be coupled together while calibrating. 

### Measuring things
The frontend of the instrument makes use of a virtual ground point which is 1.65V above the real ground. Because of this, the oscilloscope and the device under test must not be sharing the same power source. If you need to send data to the computer while measuring a device which would usually share ground with the power supply of the scope, you should connect the computer via an opto-isolated adapter, while powering the oscilloscope from an external source.

### SCPI interface
The instrument provides a SCPI interface over the UART. So far, it allows changing the settings and querying the measurements that are also displayed on the screen. SCPI parsing functionality is provided using [this library made by j123b567](https://github.com/j123b567/scpi-parser).

## Code
The code can be compiled with `make`. The actual oscilloscope code of this project is located in `Core\Src`, the [scope.c](Core/Src/scope.c), [ui.c](Core/Src/ui.c), [wave.c](Core/Src/wave.c) and [scpi_instrument.c](Core/Src/scpi_instrument.c) files.




