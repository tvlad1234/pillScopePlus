#ifndef __WAVE_H
#define __WAVE_H

float adcToVoltage(uint16_t samp);
float frontendVoltage(uint16_t samp);
void traceScreen();
void findTrigger();

extern uint8_t topClip, bottomClip;

#endif