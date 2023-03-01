# pillScope Plus
Oscilloscope based around the STM32F401 Black Pill and a color LCD screen, meant to be used as an educational tool
![The pillScope in its case](https://user-images.githubusercontent.com/60291077/177203708-384191ef-0c0f-4918-a163-46cf7b4721da.jpg)

## Why does it exist?
The goal of this project was to create a simple and easy to build but still very usable oscillsocope. I wanted to learn the basics of how a **digital storage oscilloscope** functions, while also ending up with something which can be used as an educational tool in the lab. A short description of how oscillscopes (and this one in particular) work can be found [here](HowItWorks.md).

## Features
* -3.3V to 3.3V input range (can be increased if using attenuator probes)
* 1MOhm input impedance
* 10uS/div minimum timebase
* 1.6 MSa/S sampling rate
* On screen measurements:
  * min/max voltage
  * peak-to-peak voltage
  * frequency
* Captured waveforms can be sent to a computer over UART and analyzed in the Tektronix TekScope app
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
The frontend of the instrument makes use of a virtual ground point which is 1.65V above the real ground. Because of this, the oscilloscope and the device under test must not be sharing the same ground reference. If you need to send data to the computer while measuring a device which shares ground with the scope, you should connect the computer via an opto-isolated adapter, while powering the oscilloscope from an external source.

### Saving captured waveforms
The captured waveforms can be sent to a computer over UART.

#### CSV Output
Sending `s` (lowercase s) to the UART tells the instrument to output the captured waveform in CSV format, which is compatible with the Tektronix TekScope app.

![putty](https://user-images.githubusercontent.com/60291077/177576118-8649bee9-bdd7-459b-9a0d-911d3b135e4e.png)

#### Direct TekScope output 
Sending `S` (capital S) tells the scope to output raw data, which can be read by a [companion app](https://github.com/tvlad1234/tekscopeIngest). This app automatically streams the captured data to TekScope, which allows almost real-time waveform analysis on the computer.
![companion](https://user-images.githubusercontent.com/60291077/177576844-adfbda6b-5129-4aee-bc59-4e8e6be63796.png)

## Code
The code can be compiled with `make`. The actual oscilloscope code of this project is located in `Core\Src`, the [scope.c](Core/Src/scope.c), [ui.c](Core/Src/ui.c) and [wave.c](Core/Src/wave.c) files. Feel free to take a look, as they're commented for ease of understanding.




