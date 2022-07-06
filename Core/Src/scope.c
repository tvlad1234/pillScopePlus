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

int atten = 10; // Attenuation
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

float offsetVoltage = 1.6540283; // Reference voltage for the analog frontend

extern UART_HandleTypeDef huart1;
uint8_t uartBuf[15];

// Initialize the scope
void scopeInit()
{
    ST7735_initR(INITR_BLACKTAB, &hspi1); // Initialize the LCD
    setRotation(1);
    createFramebuf(); // Create the framebuffer for the LCD
    clearDisplay();
    splash(); // Splash screen

    // Set the sampling rate
    sampRate = (16000 * 1000) / tdiv;
    sampPer = tdiv / 16.0;
    setTimerFreq(sampRate);

    // Initialize the UART
    HAL_UART_Receive_IT(&huart1, uartBuf, 1);
}

// This function acquires one buffer worth of data
void sample()
{
    HAL_TIM_Base_Start(&htim3);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuf, BUFFER_LEN);
    while (!finishedConversion)
        ;
    HAL_TIM_Base_Stop(&htim3);
    finishedConversion = 0;
}

// This runs in an infinite loop
void scopeLoop()
{
    // Acquire one buffer
    sample();

    // Find the trigger point
    findTrigger();
    if (trigged)
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);

    // Run the UI
    ui();

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);
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

// This runs after receiving a character over the UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    extern uint8_t outputFlag;
    if (uartBuf[0] == 's')
        outputFlag = 2;
    else if (uartBuf[0] == 'S')
        outputFlag = 4;
    HAL_UART_Receive_IT(&huart1, uartBuf, 1);
}