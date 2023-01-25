#ifndef __SCOPE_H
#define __SCOPE_H

#define PIXDIV 16
#define XDIV 8
#define YDIV 6

#define CLOCKTIM_PRESC 0
#define SYSCLK_FREQ 100000000
#define BUFFER_LEN (2 * PIXDIV * XDIV)

#define UPPER_VOLTAGE (atten * 3.3)
#define LOWER_VOLTAGE (atten * -3.3)

#define RISING 1
#define FALLING 0

extern float offsetVoltage;
extern uint16_t adcBuf[BUFFER_LEN];
extern int atten;
extern float vdiv;
extern float trigVoltage;
extern uint8_t trig;
extern uint8_t trigged;
extern int trigPoint;

extern float tdiv;
extern uint32_t sampRate;
extern float sampPer;

extern float maxVoltage, minVoltage;
extern float measuredFreq, sigPer;


void sendSerial(char *data, size_t s);
void scopeInit();
void scopeLoop();
void sample();

#endif