#include "main.h"
#include "scope.h"
#include "ui.h"
#include "wave.h"

#include "usbd_cdc_if.h"

#define WHITE ST7735_WHITE
#define BLACK ST7735_BLACK

#define MENUPOS 134

// All kinds of variables, you'll see what these do in scope.c
extern uint16_t adcBuf[BUFFER_LEN];
extern int atten;
extern float vdiv;
extern float trigVoltage;
extern uint8_t trig;
extern int trigPoint;

extern float tdiv;
extern uint32_t sampRate;
extern float sampPer;

extern float maxVoltage, minVoltage;
extern float measuredFreq, sigPer;

uint8_t outputFlag = 0; // whether or not we should output data to the USB port

// A little startup splash screen
void splash()
{
    printString("pillScope Plus\nCompiled ");
    printString(__DATE__);
    flushDisplay();
    HAL_Delay(2500);
}

// The main UI function
void ui()
{
    clearDisplay();
    drawWave(); // Draw the wave
    sideInfo(); // Print info on the side
    settingsBar();

    if (outputFlag) // If the computer requested data, we send it. This flag is modified in the USB receive handler in usbd_cdc_if.c
    {
        outputCSV();
        outputFlag = 0;
    }

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
}

// This function adjusts the settings
void settingsBar()
{
    static uint8_t sel = 0;
    char st[10];

    // Print top row
    setTextColor(WHITE, BLACK);
    setCursor(0, 100);
    printString("Vdiv");

    setCursor(30, 100);
    printString("Trig");

    setCursor(60, 100);
    printString("Slope");

    setCursor(95, 100);
    printString("Atten");

    setCursor(130, 100);
    if (tdiv < 1000)
        printString("us/d");
    else
        printString("ms/d");

    // Print bottom row
    setTextColor(WHITE, BLACK);
    if (sel == 0)
        setTextColor(BLACK, WHITE);
    setCursor(0, 110);
    printFloat(vdiv, 1, st);
    printf("%sV\n", st);

    setTextColor(WHITE, BLACK);
    if (sel == 1)
    {
        setTextColor(BLACK, WHITE);
        drawFastHLine(0, (uint16_t)((PIXDIV * YDIV / 2 - 1) - (trigVoltage * PIXDIV / vdiv)), XDIV * PIXDIV, ST7735_RED);
    }
    setCursor(30, 110);
    printFloat(trigVoltage, 1, st);
    printf("%s\n", st);

    setTextColor(WHITE, BLACK);
    setCursor(60, 110);
    if (sel == 2)
        setTextColor(BLACK, WHITE);
    if (trig == RISING)
        printf("Rise\n");
    else
        printf("Fall\n");

    setTextColor(WHITE, BLACK);
    setCursor(95, 110);
    if (sel == 3)
        setTextColor(BLACK, WHITE);
    printf("%dx\n", atten);

    setTextColor(WHITE, BLACK);
    if (sel == 4)
        setTextColor(BLACK, WHITE);
    setCursor(130, 110);
    if (tdiv < 1000)
        printf("%d\n", (int)tdiv);
    else
        printf("%d\n", (int)tdiv / 1000);

    // Handle buttons
    if (!HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin))
    {
        if (sel == 0) // volts per div
        {
            if (vdiv > 0.5)
                vdiv -= 0.5;
        }
        else if (sel == 1) // trigger level
        {
            if (trigVoltage > -4)
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
            if (tdiv > 20)
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
        HAL_Delay(100);
    }

    if (!HAL_GPIO_ReadPin(BTN4_GPIO_Port, BTN4_Pin))
    {
        if (sel == 0) // vdiv
        {
            if (vdiv < 2.5)
                vdiv += 0.5;
        }
        else if (sel == 1) // trigLevel
        {
            if (trigVoltage < 4)
                trigVoltage += 0.1;
        }
        else if (sel == 2) // trigType
        {
            trig = RISING;
        }
        else if (sel == 3) // atten
        {
            atten = 2;
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
        HAL_Delay(100);
    }

    if (!HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
    {
        sel++;
        HAL_Delay(150);
    }
    if (sel > 4)
        sel = 0;
}

// This function dumps the captured waveform as TekScope-compatible CSV data
void outputCSV()
{
    char st[10];
    char s1[10];
    uint8_t buffer[30] = "";

    setCursor(12, 5);
    setTextColor(BLACK, WHITE);
    printString("Sending data");
    flushDisplay();

    sprintf(buffer, "\033[2J\033[H\033[3J");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Model,TekscopeSW\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Label,CH1\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Waveform Type,ANALOG\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Horizontal Units,s\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    printFloat(sampPer, 2, st);
    sprintf(buffer, "Sample Interval,%sE-06\n\r", st);
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Record Length,%d\n\r", BUFFER_LEN);
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Zero Index,%d\n\r", trigPoint);
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Vertical Units,V\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, ",\n\rLabels,\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "TIME,CH1\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    for (int i = 0; i < BUFFER_LEN; i++)
    {
        float voltage = adcToVoltage(adcBuf[i]);
        printFloat(voltage, 1, st);
        printFloat((float)i * sampPer, 3, s1);
        sprintf(buffer, "%sE-06,%s\n\r", s1, st);
        CDC_Transmit_FS(buffer, strlen(buffer));
        HAL_Delay(5);
    }
}
