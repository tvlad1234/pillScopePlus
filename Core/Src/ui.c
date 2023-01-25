#include "main.h"
#include "scope.h"
#include "ui.h"
#include "wave.h"
#include "splash.h"

#define WHITE ST7735_WHITE
#define BLACK ST7735_BLACK

#define MENUPOS 134

uint8_t autocalFlag = 0;

// Vertical autocalibration
void autoCal()
{
    clearDisplay();
    setCursor(0, 0);
    setTextColor(BLACK, WHITE);
    printString("Autocalibration\n\n");
    setTextColor(WHITE, BLACK);
    printString("Couple input to ground\nThen press Select");
    flushDisplay();
    while (HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
        ;
    HAL_Delay(150);

    sample();

    clearDisplay();
    setCursor(0, 0);
    setTextColor(BLACK, WHITE);
    printString("Autocalibration\n\n");
    setTextColor(WHITE, BLACK);

    uint32_t adcAvg = 0;
    for (int i = 0; i < BUFFER_LEN; i++)
        adcAvg += adcBuf[i];
    adcAvg /= BUFFER_LEN;

    offsetVoltage = adcToVoltage(adcAvg);

    char st[15];
    printFloat(offsetVoltage, 2, st);
    printf("Offset voltage: %sV\n", st);

    printFloat(frontendVoltage(0), 2, st);
    printf("Min input voltage: %sV\n", st);

    printFloat(frontendVoltage(4096), 2, st);
    printf("Max input voltage: %sV\n", st);

    flushDisplay();

    while (HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
        ;
    HAL_Delay(150);
}

// A little startup splash screen
void splash()
{
    drawBitmap(0, 0, 160, 128, logo);
    setTextColor(BLACK, WHITE);

    printString(" FW compiled: ");
    printString(__DATE__);
    flushDisplay();
    HAL_Delay(2500);
}

// The main UI function
void ui()
{
    clearDisplay();

    if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) && !HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin))
    {
        autocalFlag = 1;
        if (!HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin)) // Reset if all 3 buttons are pressed at the same time
            HAL_NVIC_SystemReset();
    }

    if (autocalFlag) // Check if we need to calibrate
    {
        autoCal();
        autocalFlag = 0;
    }

    traceScreen(); // Draw the wave
    sideInfo();    // Print info on the side
    settingsBar();

    flushDisplay();
}

// This function displays voltage info in the side menu
void sideInfo()
{
    char st[15];
    printFloat(minVoltage, 1, st);
    setTextColor(BLACK, WHITE);
    setCursor(MENUPOS, 1);
    printString("Min:");
    setTextColor(WHITE, BLACK);
    setCursor(MENUPOS, 10);
    printf("%s\n", st);

    printFloat(maxVoltage, 1, st);
    setTextColor(BLACK, WHITE);
    setCursor(MENUPOS, 21);
    printString("Max:");
    setTextColor(WHITE, BLACK);
    setCursor(MENUPOS, 30);
    printf("%s\n", st);

    setTextColor(BLACK, WHITE);
    setCursor(MENUPOS, 41);
    printString("Ppk:");
    setTextColor(WHITE, BLACK);
    setCursor(MENUPOS, 51);
    printFloat(maxVoltage - minVoltage, 1, st);
    printf("%sV\n", st);

    setTextColor(BLACK, WHITE);
    setCursor(MENUPOS, 61);
    printString("Freq");
    setTextColor(WHITE, BLACK);
    setCursor(MENUPOS, 71);

    if (measuredFreq >= 1000)
    {
        if (measuredFreq >= 100000)
            printf("%d\n", (int)measuredFreq / 1000);
        else
        {
            printFloat(measuredFreq / 1000, 1, st);
            printString(st);
        }
        setCursor(MENUPOS, 81);
        printString("kHz");
    }
    else
    {
        printf("%d\n", (int)measuredFreq);
        setCursor(MENUPOS, 81);
        printString("Hz");
    }

    setCursor(MENUPOS, 91);
    if (trigged)
    {
        setTextColor(ST7735_GREEN, BLACK);
        printString("Trig");
    }
}

// This function adjusts the settings
void settingsBar()
{
    static uint8_t sel = 0;
    char st[10];

    // Print top row
    if (topClip || bottomClip)
        setTextColor(ST7735_RED, BLACK);
    else
        setTextColor(WHITE, BLACK);
    setCursor(0, 105);
    printString("Vdiv");

    setTextColor(WHITE, BLACK);
    setCursor(30, 105);
    printString("Trig");

    setCursor(60, 105);
    printString("Slope");

    setCursor(95, 105);
    printString("Atten");

    setCursor(130, 105);
    if (tdiv < 100)
        printString("us/d");
    else
        printString("ms/d");

    // Print bottom row
    if (sel == 0)
    {
        if (topClip || bottomClip)
            setTextColor(ST7735_RED, WHITE);
        else
            setTextColor(BLACK, WHITE);
    }
    else if (topClip || bottomClip)
        setTextColor(ST7735_RED, BLACK);
    else
        setTextColor(WHITE, BLACK);
    setCursor(0, 115);
    printFloat(vdiv, 1, st);
    printf("%sV\n", st);

    setTextColor(WHITE, BLACK);
    if (sel == 1)
    {
        setTextColor(BLACK, WHITE);
        drawFastHLine(0, (uint16_t)((PIXDIV * YDIV / 2 - 1) - (trigVoltage * PIXDIV / vdiv)), XDIV * PIXDIV, ST7735_RED);
    }
    setCursor(30, 115);
    printFloat(trigVoltage, 1, st);
    printf("%s\n", st);

    setTextColor(WHITE, BLACK);
    setCursor(60, 115);
    if (sel == 2)
        setTextColor(BLACK, WHITE);
    if (trig == RISING)
        printf("Rise\n");
    else
        printf("Fall\n");

    setTextColor(WHITE, BLACK);
    setCursor(95, 115);
    if (sel == 3)
        setTextColor(BLACK, WHITE);
    printf("%dx\n", atten);

    setTextColor(WHITE, BLACK);
    if (sel == 4)
        setTextColor(BLACK, WHITE);
    setCursor(130, 115);
    if (tdiv < 100)
        printf("%d\n", (int)tdiv);
    else if (tdiv < 1000)
        printf("0.%d\n", (int)tdiv / 100);
    else
        printf("%d\n", (int)tdiv / 1000);

    // Handle buttons
    if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin))
    {
        if (sel == 0) // volts per div
        {
            if (vdiv > 0.5)
                vdiv -= 0.5;
        }
        else if (sel == 1) // trigger level
        {
            trigVoltage -= 0.1;
        }
        else if (sel == 2) // trigger slope
        {
            trig = FALLING;
        }
        else if (sel == 3) // attenuation
        {
            atten = 1;
        }
        else if (sel == 4) // tdiv
        {
            if (tdiv > 10)
            {
                if (tdiv > 1000)
                    tdiv -= 1000;
                else if (tdiv > 100)
                    tdiv -= 100;
                else if (tdiv > 10)
                    tdiv -= 10;
            }

            sampRate = (PIXDIV * 1000 * 1000) / tdiv;
            sampPer = tdiv / (float)PIXDIV;
            setTimerFreq(sampRate);
        }
        HAL_Delay(150);
    }

    if (!HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin))
    {
        if (sel == 0) // vdiv
        {
            if (vdiv < 9)
                vdiv += 0.5;
        }
        else if (sel == 1) // trigLevel
        {
            trigVoltage += 0.1;
        }
        else if (sel == 2) // trigType
        {
            trig = RISING;
        }
        else if (sel == 3) // atten
        {
            atten = 10;
        }
        else if (sel == 4) // tdiv
        {
            if (tdiv >= 1000)
                tdiv += 1000;
            else if (tdiv >= 100)
                tdiv += 100;
            else if (tdiv >= 10)
                tdiv += 10;

            sampRate = (PIXDIV * 1000 * 1000) / tdiv;
            sampPer = tdiv / (float)PIXDIV;
            setTimerFreq(sampRate);
        }
        HAL_Delay(150);
    }

    if (!HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
    {
        sel++;
        HAL_Delay(150);
    }
    if (sel > 4)
        sel = 0;
}

