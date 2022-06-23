#include "main.h"
#include "scope.h"
#include "ui.h"
#include "wave.h"

#include "usbd_cdc_if.h"

#define WHITE ST7735_WHITE
#define BLACK ST7735_BLACK

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

int currentMenu = 1;
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
    sideMenu(); // Handle the side menu

    if (outputFlag) // If the computer requested data, we send it. This flag is modified in the USB receive handler in usbd_cdc_if.c
    {
        outputCSV();
        outputFlag = 0;
    }

    flushDisplay();
}

// This function handles the side menu
void sideMenu()
{
    switch (currentMenu)
    {
    case 1: // Print voltage measurements
        voltageInfo();
        break;
    case 2: // Adjust vertical parameters
        voltsMenu();
        break;
    case 3: // Adjust time
        timeMenu();
        break;
    case 4: // USB menu
        usbMenu();
        break;
    default:
        break;
    }

    // The menu button cycles between menus
    if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin))
    {
        currentMenu++;
        if (currentMenu > 4)
            currentMenu = 1;
        HAL_Delay(250);
    }
}

// This function displays voltage info in the side menu
void voltageInfo()
{
    char st[15];
    printFloat(minVoltage, 1, st);
    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    printString("Min:");
    setTextColor(WHITE, BLACK);
    setCursor(100, 10);
    printf("%s\n", st);

    printFloat(maxVoltage, 1, st);
    setTextColor(BLACK, WHITE);
    setCursor(100, 21);
    printString("Max:");
    setTextColor(WHITE, BLACK);
    setCursor(100, 30);
    printf("%s\n", st);

    setTextColor(BLACK, WHITE);
    setCursor(100, 41);
    printString("Ppk:");
    setTextColor(WHITE, BLACK);
    setCursor(100, 51);
    printFloat(maxVoltage - minVoltage, 1, st);
    printf("%sV\n", st);
}

// This function adjusts the volts per div, trigger level, trigger slope and attenuation
void voltsMenu()
{
    static uint8_t sel = 0;
    char st[10];

    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    printString("Vdiv");

    setTextColor(BLACK, WHITE);
    setCursor(100, 21);
    printString("Trig");

    setTextColor(WHITE, BLACK);
    if (sel == 0)
        setTextColor(BLACK, WHITE);
    setCursor(100, 10);
    printFloat(vdiv, 1, st);
    printf("%sV\n", st);

    setTextColor(WHITE, BLACK);
    if (sel == 1)
    {
        setTextColor(BLACK, WHITE);
        drawFastHLine(0, (uint16_t)(31 - (trigVoltage * 16 / vdiv)), 96, ST7735_RED);
    }
    setCursor(100, 30);
    printFloat(trigVoltage, 1, st);
    printf("%s\n", st);

    setTextColor(WHITE, BLACK);
    setCursor(100, 40);
    if (sel == 2)
        setTextColor(BLACK, WHITE);
    if (trig == RISING)
        printf("Rise\n");
    else
        printf("Fall\n");

    setTextColor(WHITE, BLACK);
    setCursor(100, 50);
    if (sel == 3)
        setTextColor(BLACK, WHITE);
    printf("%dx\n", atten);

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
        HAL_Delay(100);
    }

    if (!HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
    {
        sel++;
        HAL_Delay(150);
    }
    if (sel > 3)
        sel = 0;
}

// This function sets the time per div and displays the frequency of the input signal
void timeMenu()
{
    char st[10];

    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    if (tdiv < 1000)
        printString("us/d");
    else
        printString("ms/d");

    setTextColor(WHITE, BLACK);
    setCursor(100, 10);

    if (tdiv < 1000)
        printf("%d\n", (int)tdiv);
    else
        printf("%d\n", (int)tdiv / 1000);

    setTextColor(BLACK, WHITE);
    setCursor(100, 21);
    printString("Freq");
    setTextColor(WHITE, BLACK);
    setCursor(100, 30);

    if (measuredFreq >= 1000)
    {
        if (measuredFreq >= 100000)
            printf("%d\n", (int)measuredFreq / 1000);
        else
        {
            printFloat(measuredFreq / 1000, 1, st);
            printString(st);
        }
        setCursor(100, 40);
        printString("kHz");
    }
    else
    {
        printf("%d\n", (int)measuredFreq);
        setCursor(100, 40);
        printString("Hz");
    }

    if (!HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin))
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

        sampRate = (16000 * 1000) / tdiv;
        sampPer = tdiv / 16.0;
        setTimerFreq(sampRate);

        HAL_Delay(100);
    }

    if (!HAL_GPIO_ReadPin(BTN4_GPIO_Port, BTN4_Pin))
    {
        if (tdiv >= 1000)
            tdiv += 1000;
        else if (tdiv >= 100)
            tdiv += 100;
        else if (tdiv >= 10)
            tdiv += 10;

        sampRate = (16000 * 1000) / tdiv;
        sampPer = tdiv / 16.0;
        setTimerFreq(sampRate);

        HAL_Delay(100);
    }
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

// This menu allows sending the waveform to the computer
void usbMenu()
{
    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    printString("USB");
    setTextColor(WHITE, BLACK);
    setCursor(100, 10);
    printString("Send");
    if (!HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
    {
        outputCSV();
        HAL_Delay(1000);
    }
}

// This function draws the graticule onto the screen
void drawGraticule(int hei, int wit, int pix, int divx, int divy)
{
    for (int i = 0; i <= wit; i += pix)
        dottedVLine(i, 0, hei);

    for (int i = 0; i <= hei; i += pix)
        dottedHLine(0, i, wit);
}

// This function draws a dotted horizontal line, used for drawing the graticule
void dottedHLine(int x, int y, int l)
{
    for (int i = 0; i <= l; i++)
    {   
        if(i%2)
            drawPixel(x + i, y, WHITE);
        else drawPixel(x + i, y, BLACK);

    }
}

// This function draws a dotted vertical line, used for drawing the graticule
void dottedVLine(int x, int y, int l)
{
    for (int i = 0; i <= l; i++)
    {
        if(i%2)
            drawPixel(x, y + i, WHITE);
        else drawPixel(x, y + i, BLACK);
    }
}
