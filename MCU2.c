// CECS 447: Project 2 - UART Communications
// File Name: MCU2.c
// Purpose: Controls LED colors and brightness using UART and SysTick PWM,
//          also has a menu to select modes and adjust settings.
// Group: Robby Rimal, Jonathan Cerniaz, Joseph Guzman, Afzal Hakim

#include "PLL.h"
#include "UART0.h"
#include "UART3.h"
#include "LEDSW.h"
#include "tm4c123gh6pm.h"

#include <stdint.h>
#include <stdbool.h>

#define MAX_STR_LEN 20

// define bit values for all Colors
#define DARK 0x00
#define RED 0x02
#define BLUE 0x04
#define GREEN 0x08
#define YELLOW 0x0A
#define PURPLE 0x06
#define WHITE 0x0E
#define CYAN 0x0C

// TODO: define all colors in the color wheel
const uint8_t color_wheel[] = {DARK, RED, GREEN, BLUE, YELLOW, CYAN, PURPLE, WHITE};
const int color_wheel_size = sizeof(color_wheel) / sizeof(color_wheel[0]);

#define MODE1 0x1F
#define MODE2 0x2F
#define MODE3 0x3F

#define PERIOD 500000        // number of machine cycles for 10ms, value is based on 50MHz system clock
#define HALF_DUTY PERIOD / 2 // 50% duty cycle

// TODO: define bit addresses for the onboard three LEDs and two switches
#define LED (*((volatile uint32_t *)0x40025038)) // use onboard RGB LED
#define SW1 0x10
#define SW2 0x01

extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);
extern void DisableInterrupts(void);
void Mode1(void);
void Mode2(void);
void Mode3(void);
void DisplayMenu(void);
uint32_t StringToDecimal(uint8_t str[]);
void DecimalToString(uint8_t str[], uint32_t n);
void mode2Menu(void);
void mode3Menu(void);
void displayColor(void);

// globals
volatile unsigned long strIndex0 = 0; // string0 index
volatile unsigned long strIndex3 = 0; // string3 index
uint8_t string0[MAX_STR_LEN];         // string0
uint8_t string3[MAX_STR_LEN];         // string3
bool end_of_str0 = false;
bool isPwmHigh = true;               // PWM high or low
volatile unsigned long color = DARK; // color mode
volatile unsigned long H = HALF_DUTY;
volatile unsigned long L = HALF_DUTY;
uint8_t ledColorIndex = 0;
bool waitForInterruptFlag = true;
bool uart3InterruptFlag = true;
bool isUART3Waiting = false;
bool CHANGE_COLOR_FLAG = false;
bool SEND_COLOR_FLAG = false;
uint8_t selectedMode = 1;

int main(void)
{
    DisableInterrupts();
    PLL_Init();
    UART0_Init(true, false); // for PC<->MCU1
    UART3_Init(true, false); // for MCU1<->MCU2
    LEDSW_Init();            // Initialize the onboard three LEDs and two push buttons
    EnableInterrupts();

    LED = DARK;

    while (1)
    {
        // displays the main menu
        DisplayMenu();
        switch (selectedMode)
        {
        case 1:
            Mode1();
            break;
        case 2:
            Mode2();
            break;
        case 3:
            Mode3();
            break;
        default:
            break;
        }
        OutCRLF0();
    }
}

void Mode1(void)
{
    while (selectedMode == 1)
    {
    }
    OutCRLF0();
}

void Mode2(void)
{
bool exit = false;
waitForInterruptFlag = true;
uart3InterruptFlag = true;
isUART3Waiting = false;
bool isFirstRun = true;

while (!exit)
{
    if (isFirstRun)
    {
        // Display waiting message for the first run
        OutCRLF0();
        UART0_OutString((uint8_t *)"Mode 2 MCU2");
        OutCRLF0();
        UART0_OutString((uint8_t *)"Waiting for color from MCU1...");
        OutCRLF0();
        isFirstRun = false;
    }
    else
    {
        // Display current color and waiting message for subsequent runs
				OutCRLF0();
        UART0_OutString((uint8_t *)"Mode 2 MCU2");
        OutCRLF0();
        UART0_OutString((uint8_t *)"Current Color: ");
        displayColor(); // display current LED
        UART0_OutString((uint8_t *)"Waiting for color from MCU1...");
        OutCRLF0();
    }

    while (uart3InterruptFlag && !(selectedMode == 1))
    {
        WaitForInterrupt();
    }

    uart3InterruptFlag = true;

    if (selectedMode == 1)
    {
        exit = true;
    }
    else
    {
        mode2Menu(); // display mode 2 menu

        while (waitForInterruptFlag && !(selectedMode == 1))
        {
            if (isUART3Waiting)
            {
                color = color_wheel[ledColorIndex];
                UART0_OutString((uint8_t *)"Current color: ");
                displayColor(); // display current LED
                isUART3Waiting = false;
                LED = color;
                //UART3_OutChar(ledColorIndex); // Send color index to MCU1
            }

            WaitForInterrupt();
        }

        waitForInterruptFlag = true;
    }
}

ledColorIndex = 0;
color = DARK;
LED = DARK;
OutCRLF0();

}

void Mode3(void)
{
    bool exit = false;
    uint8_t index = 0;
    mode3Menu();

    while (!exit)
    {
        while ((!isUART3Waiting) && (selectedMode != 1))
        {
            WaitForInterrupt();
        }
        isUART3Waiting = false;

        UART0_OutString((uint8_t *)string3);
        OutCRLF0();

        while ((!end_of_str0) && (selectedMode != 1))
        { // wait until the whole string is received.
            WaitForInterrupt();
        }
        end_of_str0 = false;
        strIndex0 = 0;
        OutCRLF0();

        while ((index != MAX_STR_LEN) && (selectedMode != 1))
        {
            UART3_OutChar(string0[index]);
            index += 1;
        }
        index = 0;

        if (selectedMode == 1)
        {
            waitForInterruptFlag = true;
            exit = true;
        }
    }

    UART3_OutChar(MODE1);
}

void mode2Menu(void)
{
    // Display Mode 2 Menu
    OutCRLF0();
    UART0_OutString((uint8_t *)"Mode 2 MCU2");
    OutCRLF0();
    UART0_OutString((uint8_t *)"In color Wheel State.");
    OutCRLF0();
    UART0_OutString((uint8_t *)"Please press SW2 to cycle through colors:");
    OutCRLF0();
    UART0_OutString((uint8_t *)"Dark, Red, Green, Blue, Yellow, Cyan, Purple, White");
    OutCRLF0();
    UART0_OutString((uint8_t *)"Press SW1 to send the selected color to MCU1.");
    OutCRLF0();
    UART0_OutString((uint8_t *)"Current color: ");

    color = color_wheel[ledColorIndex];
    LED = color;
    isUART3Waiting = false;
    displayColor();
}

void displayColor(void)
{
    const char *colorNames[] = {"Dark", "Red", "Green", "Blue", "Yellow", "Cyan", "Purple", "White"};
    UART0_OutString((uint8_t *)colorNames[ledColorIndex]);
    OutCRLF0();
}

void DisplayMenu(void)
{
    UART0_OutString((uint8_t *)"Welcome to CECS 447 Project 2 - UART ");
    OutCRLF0();
    UART0_OutString((uint8_t *)"MCU2");
    OutCRLF0();
    UART0_OutString((uint8_t *)"Waiting for command from MCU1...");
    OutCRLF0();
}

void mode3Menu(void)
{
    UART0_OutString((uint8_t *)"Mode 3 MCU2: Chat Room");
    OutCRLF0();
    UART0_OutString((uint8_t *)"Press sw1 at any time to exit the chat room.");
    OutCRLF0();
    UART0_OutString((uint8_t *)"Waiting for a message from MCU1...");
    OutCRLF0();
}

// Take care of Rx interrupt and ignore Tx interrupt
void UART0_Handler(void)
{
    uint8_t chr;

    if (UART0_RIS_R & UART_RIS_RXRIS)
    { // received one item
        if ((UART0_FR_R & UART_FR_RXFE) == 0)
        {
            chr = UART0_DR_R & 0xFF;
            if (chr == CR)
            { // reach end of the string
                end_of_str0 = true;
                string0[strIndex0] = NULL; // add null terminator to end a C string.
            }
            else if (strIndex0 < (MAX_STR_LEN - 1))
            { // save one spot for C null terminator '\0'.
                if (chr == BS)
                {
                    UART0_OutChar(BS);
                    strIndex0--;
                }
                else
                {
                    string0[strIndex0++] = chr; // add the latest received symbol to end a C string.
                    UART0_OutChar(chr);
                }
            }
        }
        UART0_ICR_R = UART_ICR_RXIC; // acknowledge RX FIFO
    }
}

uint32_t StringToDecimal(uint8_t str[])
{
    uint32_t number = 0;
    ;
    uint8_t character, idx = 0;

    character = str[idx];
    while (character != NULL)
    {
        if ((character >= '0') && (character <= '9'))
        {
            number = 10 * number + (character - '0'); // this line overflows if above 2^32-1
        }
        else
        { // none decimal digit fond, stop converting and return previous digits
            return number;
        }
        character = str[++idx]; // get the next digit
    }
    return number;
}

void DecimalToString(uint8_t str[], uint32_t n)
{
    static uint8_t idx = 0;

    if ((n / 10) != 0)
    {
        DecimalToString(str, n / 10);
        DecimalToString(str, n % 10);
    }
    else
    {
        str[idx++] = n % 10 + '0';
        return;
    }

    str[idx] = '\0';
    return;
}

// Take care of Rx interrupt and ignore Tx interrupt
void UART3_Handler(void)
{
    if (UART3_RIS_R & UART_RIS_RXRIS)
    { // received one item
        if ((UART3_FR_R & UART_FR_RXFE) == 0)
        {
            if (selectedMode == 2)
            {
							if (UART3_DR_R == MODE1)
                {
                    selectedMode = 1;
                }
                else if (ledColorIndex == UART3_DR_R)
                {
                    isUART3Waiting = true;
                    uart3InterruptFlag = false;
                }
                else
                {
                    ledColorIndex = UART3_DR_R;
                    isUART3Waiting = true;
                    uart3InterruptFlag = false;
                }
            }

            if (selectedMode == 3)
            {
                string3[strIndex3] = UART3_DR_R;
                strIndex3 += 1;
                if (strIndex3 > 19)
                {
                    isUART3Waiting = true;
                    strIndex3 = 0;
                }
            }

            UART3_ICR_R = UART_ICR_RXIC; // acknowledge RX FIFO

            if (UART3_DR_R == MODE1)
            {
                selectedMode = 1;
            }
            else if (UART3_DR_R == MODE2)
            {
                selectedMode = 2;
            }
            else if (UART3_DR_R == MODE3)
            {
                selectedMode = 3;
            }
        }
    }
}

void GPIOPortF_Handler(void)
{
    for (int i = 0; i <= 299999; i++)
    {
    } // delay for button press

    if (GPIO_PORTF_RIS_R & SW2)
    { // SW 2 PRESS
        GPIO_PORTF_ICR_R |= SW2;
        ledColorIndex = (ledColorIndex + 1) % color_wheel_size;
        isUART3Waiting = true;
    }

    if (GPIO_PORTF_RIS_R & SW1)
    { // SW 1 PRESS
        GPIO_PORTF_ICR_R |= SW1;
        if (selectedMode == 2)
        {
            UART3_OutChar(ledColorIndex); // Send the color index to MCU1
            waitForInterruptFlag = false;
        }
        else if (selectedMode == 3)
        {
            selectedMode = 1;
        }
    }
    for (int i = 0; i <= 299999; i++)
    {
    } // delay for button press
}
