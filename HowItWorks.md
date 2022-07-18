# How does it work?
## But first of all, what's an oscilloscope?
An oscilloscope, colloquially called _scope_ is a measurement instrument which graphically displays electrical voltage in relation to time. This allows the user to visualise and analyze high-speed electrical signals which cannot be seen otherwise.

In the past, oscilloscopes were constructed using cathode ray tubes, similar to the ones found in old TVs. They worked by sweeping an electron beam across a phosphor-coated screen, which was being vertically deflected by the input signal. This would cause the screen to glow in every point struck by the beam, allowing visualisation of the input signal as a waveform. This process was purely analog and happened in real-time. As such, all measurements had to be done manually, by physically measuring the waveform on the screen. This also meant that the waveforms couldn't be stored anywhere (analog storage oscilloscopes existed but they were very uncommon). 

Most oscilloscopes nowadays are *digital storage oscillscopes*. As their name suggests, they sample and store the incoming signal in a digital manner. This way, the signal can be analyzed using a computer, making advanced and automatic measurements possible.

## How does a digital storage oscilloscope work?
### Conditioning the signal
Before it can be measured by the various components of an oscilloscope, the input signal must be conditioned in some way. This usually involves scaling and attenuating it by a component called the *analog frontend*. The actual measuring element of our instrument can only sample voltages in the 0 - 3.3V range. In order to extend this range into a useful domain, we attenuate the input signal by half and center it at around 1.65V. This is done by using a voltage divider and an op-amp to create a reference voltage and another divider and opamp to attenuate and buffer the signal. Doing so allows the instrument measure voltages ranging from -3.3V to +3.3V. The schematic of the frontend can be found in the [frontend.pdf](frontend.pdf) file.

### Converting voltage into numbers
Electrical voltage is a physical quantity which must be sampled and quantized into a numerical value. This is done by an _ADC_ (**A**nalog to **D**igital **C**onverter). The ADC used in this project is integrated into the microcontroller and it's a *successive aproximation register ADC*. This means that the ADC takes the input voltage and compares it to different voltages until it finds the closest match. Two of the main characteristics of an ADC are its resolution and maximum sampling rate. The resolution tells us how many bits it uses to represent numbers. Our ADC is 12-bit, meaning that there are 2<sup>12</sup> different values it can output. The maximum sampling rate measures how fast the ADC can measure the incoming signal. Our ADC is capable of up to 1.6 MSa/S (megasamples per second, or milions of samples per second).

### Sampling a signal
In order to digitally reconstruct a waveform, the signal must be sampled at precise intervals of time. We're achieving this by using one of the timers of our microcontroller in order to generate the timebase of our scope. This way, the ADC captures a sample on every firing of the timer. Oscilloscopes generally use dedicated hardware to trigger the sampling on precise segments of the input signal. For the sake of simplicity, our oscilloscope simulates this in software by aligning the displayed waveform with the selected trigger condition.

### Getting the data into the memory
Digital oscilloscopes usually have dedicated sample memory and hardware which handles the transfer of data from the ADC without the main processor being involved. We're using a similar approach here too, by making use of the *DMA* (**D**irect **M**emory **A**ccess) engine of the microcontroller. The processor tells the DMA to start pulling data from the ADC into a buffer in memory and then waits. Each time the ADC is triggered, a DMA transfer occurs. Once the buffer is full, the DMA stops and tells the processor that it has finished transfering data. With the sampled signal in memory, we can now display the waveform on the screen and perform different measurements.