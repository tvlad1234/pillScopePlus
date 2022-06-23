#include "main.h"
#include "scope.h"
#include "ui.h"
#include "wave.h"

/// Hardware handles
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim3;

uint16_t adcBuf[BUFFER_LEN];             // this is where we'll store data
volatile uint8_t finishedConversion = 0; // this lets us know when we're done capturing data

int atten = 1;  // Attenuation
float vdiv = 2; // Volts per division

uint8_t trigged;       // whether or not we're triggered
int trigPoint;         // triggering point
float trigVoltage = 0; // Trigger level
uint8_t trig = RISING; // Trigger slope

float tdiv = 20;   // uS per division
uint32_t sampRate; // Sample rate
float sampPer;     // Sample period in uS (how long it takes to measure one sample)

float maxVoltage, minVoltage; // Voltage measurements
float measuredFreq, sigPer;   // Time measurements

// Initialize the scope
void scopeInit()
{
    ST7735_initR(INITR_BLACKTAB, &hspi1); // Initialize the LCD
    setRotation(2);
    createFramebuf(); // Create the framebuffer for the LCD
    clearDisplay();
    splash(); // Splash screen

    sampRate = (16000 * 1000) / tdiv;
    sampPer = tdiv / 16.0;
    setTimerFreq(sampRate);

    HAL_TIM_Base_Start(&htim3);                                // Start the timebase timer
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuf, BUFFER_LEN); // Start the ADC
}

// This runs in an infinite loop
void scopeLoop()
{
    if (finishedConversion) // We finished aquiring one buffer
    {
        // HAL_ADC_Stop_DMA(&hadc1);
        HAL_TIM_Base_Stop(&htim3);
        // Find the trigger point
        findTrigger();
        if (trigged)
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);

        // Run the UI
        ui();

        // Start again
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);
        finishedConversion = 0;
        HAL_TIM_Base_Start(&htim3);
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuf, BUFFER_LEN);
    }
}

// This sets the sampling rate
void setTimerFreq(uint32_t freq)
{
    uint16_t arr = (SYSCLK_FREQ / ((CLOCKTIM_PRESC + 1) * freq)) - 1;
    htim3.Instance->ARR = arr;
}

// This runs after the ADC has finished sampling one whole buffer
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    finishedConversion = 1;
}