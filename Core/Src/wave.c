#include "main.h"
#include "scope.h"
#include "wave.h"

#define WHITE ST7735_WHITE
#define BLACK ST7735_BLACK
#define WAVE_COLOR ST7735_YELLOW

// All kinds of variables, you'll see what these do in scope.c
extern uint16_t adcBuf[BUFFER_LEN];
extern int atten;
extern float vdiv;
extern float trigVoltage;
extern uint8_t trig, trigged;
extern int trigPoint;

extern float tdiv;
extern uint32_t sampRate;
extern float sampPer;

extern float maxVoltage, minVoltage;
extern float measuredFreq, sigPer;

// Convert ADC value to volts
float adcToVoltage(uint16_t samp)
{
    return atten * 2 * (((3.3 * samp) / 4096.0) - 1.65);
}

// Draw the waveform on the screen
void drawWave()
{
    drawGraticule(64, 96, 16, 4, 4); // Draw the graticule

    maxVoltage = LOWER_VOLTAGE;
    minVoltage = UPPER_VOLTAGE;

    for (int i = 0; i <= 94; i++)
    {
        // If we're looping through the buffer, let's compute the minimum and maximum voltage values while we're at it
        float voltage1 = adcToVoltage(adcBuf[i + trigPoint]);
        float voltage2 = adcToVoltage(adcBuf[i + trigPoint + 1]);
        if (voltage2 > maxVoltage)
            maxVoltage = voltage2;
        if (voltage2 < minVoltage)
            minVoltage = voltage2;

        // Draw lines between sample points
        drawLine(i, 31 - (voltage1 * 16 / vdiv), i + 1, 31 - (voltage2 * 16 / vdiv), WAVE_COLOR);
    }
}

// This function finds the trigger point and also computes the frequency of thge signal
void findTrigger()
{
    int trigLevel = (4096.0 * (trigVoltage / (2.0 * atten) + 1.65)) / 3.3; // ADC level at which we should trigger
    int trigPoint2;                                                        // another trigger point, this will help us determine the period of the signal

    trigPoint = 0;
    trigged = 0;
    measuredFreq = 0;

    // The trigged variable will be 0 if we're not triggering, 1 if we only found 1 trigger point and 2 if we have at least two trigger points

    for (int i = 1; i < BUFFER_LEN / 2 && trigged != 2; i++) // we're looking for trigger points in the first half of the buffer
        if ((trig == RISING && adcBuf[i] >= trigLevel && adcBuf[i - 1] < trigLevel) || (trig == FALLING && adcBuf[i] <= trigLevel && adcBuf[i - 1] > trigLevel))
        {
            if (!trigged) // Looking for the first trigger point
            {
                trigPoint = i;
                trigged = 1;
            }
            else // Looking for the second one
            {
                trigPoint2 = i;
                trigged = 2;
            }
        }

    if (trigged == 2) // If we found at least two trigger points
    {
        sigPer = sampPer * (trigPoint2 - trigPoint); // we compute the period of the signal in uS
        measuredFreq = 1000000.0 / sigPer;           // and then we convert it into frequency, in Hz
    }
}