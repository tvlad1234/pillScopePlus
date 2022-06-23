# pillScope Plus
Oscilloscope based around the STM32F401 Black Pill and a color LCD screen
## Features
-3.3V to 3.3V input range (can be increased if using attenuator probes)\
Approximatively 1MOhm input impedance\
Timebase goes down to 20uS/div\
On screen measurements: min/max voltage, peak-to-peak voltage, frequency\
Captured waveforms can be sent over USB in TekScope-compatible CSV format.
## Required parts
### Base parts:
STM32F401CC Black Pill development board\
128x160 ST7735-based OLED display\
4 pushbuttons
### Analog frontend:
LM358 dual op-amp (rail-to-rail opamps should work better in this context, but this is what I had on hand)\
2x 68kOhm resistors (to create a 1.65V offset voltage)\
2x 500kOhm resistors (to create the input attenuator)
### 2x probes:
just a 1MOhm resistor, in series with the input

## Schematics
The LCD screen is connected to SPI1 (PB3-SCK, PB5-MOSI, PB12-CS, PB13-RST, PB14-DC). The buttons are connected as follows:\
PB9: Up\
PB8: Select\
PB7: Down


The output of the analog frontend is connected to ADC1_IN0, which corresponds to PA0.

The analog frontend consists of:\
a 1.65V voltage reference, which serves as a virtual ground point for the input,\
a 2x voltage divider at the input,\
an LM358 dual op-amp, which buffers the reference voltage and the output of the input attenuator

The schematic of the analog frontend can be found in the frontend.pdf file.

## Saving wavevorms
The captured waveforms can be sent to a computer over USB. Sending `s` or `S` to the USB serial port will tell the scope to output the captured waveform in CSV format. This data can then be imported into the Tektronix TekScope app for further analysis.

## Code
The code can be compiled with `make`. The actual oscilloscope code of this project is located in `Core\Src`, the `scope.c`, `ui.c`, `wave.c` files. Feel free to take a look, as they're commented for ease of understanding.




