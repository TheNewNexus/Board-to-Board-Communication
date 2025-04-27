// CECS 447: Project 2 - UART Communications
// File Name: MCU1.c
// Purpose: Sets up UART for data transmission, reception, and handling input/output.
// Group: Robby Rimal, Jonathan Cerniaz, Joseph Guzman, Afzal Hakim

#include "PLL.h"
#include "UART0.h"
#include "UART2.h"
#include "LEDSW.h"
#include "tm4c123gh6pm.h"

#include <stdint.h>
#include <stdbool.h>

#define MAX_STR_LEN 20 // Maximum string length

// Define bit values for all colors
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
const int color_wheel_size = 8; // Number of colors in the color wheel
// Define mode values
#define MODE_1 0x1F
#define MODE_2 0x2F
#define MODE_3 0x3F

// Define period and duty cycle values for PWM
#define PERIOD 500000		   // Number of machine cycles for 10ms (based on 50MHz system clock)
#define HALF_DUTY (PERIOD / 2) // 50% duty cycle

// Define bit addresses for the onboard three LEDs and two switches
#define LED (*((volatile uint32_t *)0x40025038)) // Onboard RGB LED
#define SW1 0x10
#define SW2 0x01

// Function prototypes
extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);
extern void DisableInterrupts(void);
void Mode1(void);
void Mode2(void);
void Mode3(void);
void DisplayMenu(void);
void SetColor(void);
void SetBrightness(void);
void SysTick_Init(void);
uint32_t StringToDecimal(uint8_t str[]);
void DecimalToString(uint8_t str[], uint32_t n);
void mode2Menu(void);
void displayColor(void);
void mode3Menu(void);

// Global variables
volatile unsigned long strIndex0 = 0;		   // Index for string0
volatile unsigned long strIndex2 = 0;		   // Index for string2
uint8_t string0[MAX_STR_LEN];				   // String buffer for UART0
uint8_t string2[MAX_STR_LEN];				   // String buffer for UART2
bool endOfString0 = false;					   // Flag to indicate end of string input
bool isPwmHigh = true;						   // PWM high or low state
volatile unsigned long currentLedColor = DARK; // Current LED color
volatile unsigned long highTime = HALF_DUTY;   // High time for PWM
volatile unsigned long lowTime = HALF_DUTY;	   // Low time for PWM
uint8_t ledColorIndex = 0;					   // Index for LED color
bool waitForInterruptFlag = true;			   // Flag to wait for interrupt in Mode2
bool uart2InterruptFlag = true;				   // Flag for UART2 interrupt in Mode2
bool isUart2Waiting = false;				   // Flag to indicate if UART2 is waiting
bool CHANGE_COLOR_FLAG = true;
bool SEND_COLOR_FLAG = true;
uint8_t selectedMode = 0; // Current selected mode (Mode1, Mode2, Mode3)

// Main function
int main(void)
{
	DisableInterrupts();
	PLL_Init();
	SysTick_Init();
	UART0_Init(true, false);				// Initialize UART0 (PC <-> MCU1)
	UART2_Init(true, false);				// Initialize UART2 (MCU1 <-> MCU2)
	LEDSW_Init();							// Initialize onboard LEDs and push buttons
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; // Turn off SysTick
	EnableInterrupts();

	// Set initial LED color to DARK (off)
	LED = DARK;

	while (1)
	{
		// Display the main menu
		DisplayMenu();

		// Handle user input based on menu selection
		switch (string0[0])
		{
		case '1':
			Mode1();
			break;
		case '2':
			Mode2();
			break;
		case '3':
			Mode3();
			break;
		default:
			break;
		}

		// Print new line after each operation
		OutCRLF0();
	}
}

// Mode1: LED Color and Brightness Control
void Mode1(void)
{
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; // Turn off SysTick
	UART2_OutChar(MODE_1);					// Send mode 1 to MCU2
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;	// Turn on SysTick
	bool exitMode1 = false;
	selectedMode = 1;

	while (!exitMode1)
	{
		// Display Mode 1 Menu
		OutCRLF0();
		UART0_OutString((uint8_t *)"Mode 1 Menu: ");
		OutCRLF0();
		UART0_OutString((uint8_t *)"1. Choose an LED color.");
		OutCRLF0();
		UART0_OutString((uint8_t *)"2. Change the brightness of the LED.");
		OutCRLF0();
		UART0_OutString((uint8_t *)"3. Exit Mode 1.");
		OutCRLF0();

		// Wait for user input
		while (!endOfString0)
		{
			WaitForInterrupt();
		}

		// Process user input for Mode1
		endOfString0 = false;
		strIndex0 = 0;
		OutCRLF0();

		switch (string0[0])
		{
		case '1':
			SetColor();
			break;
		case '2':
			SetBrightness();
			break;
		case '3':
			exitMode1 = true;
			break;
		default:
			break;
		}
	}

	// Reset LED to DARK and brightness to 50% duty cycle when exiting Mode1
	currentLedColor = DARK;
	LED = currentLedColor;
	highTime = HALF_DUTY;
	lowTime = HALF_DUTY;
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; // Turn off SysTick
	OutCRLF0();
}

// Mode2: MCU1 <-> MCU2 Color Wheel
void Mode2(void)
{
	UART2_OutChar(MODE_2); // Send mode 2 to MCU2
	bool exitMode2 = false;
	waitForInterruptFlag = true;
	uart2InterruptFlag = true;
	isUart2Waiting = false;
	selectedMode = 2;

	while (!exitMode2)
	{
		mode2Menu();
		while (uart2InterruptFlag && !(string0[0] == 0x5E))
		{
			if (isUart2Waiting)
			{
				currentLedColor = color_wheel[ledColorIndex];
				UART0_OutString((uint8_t *)"Current color: ");
				displayColor();
				isUart2Waiting = false;
				LED = currentLedColor;
				UART2_OutChar(ledColorIndex); // Send color index to MCU2[3]
			}
			WaitForInterrupt();
		}

		uart2InterruptFlag = true;

		if (string0[0] == 0x5E)
		{
			exitMode2 = true;
			
		}
	}

	ledColorIndex = 0;
	currentLedColor = DARK;
	LED = DARK;
	UART2_OutChar(MODE_1);
	strIndex0 = 0;
	OutCRLF0();
}

// Mode3: Chat Room (PC <-> MCU1 <-> MCU2 <-> PC)
void Mode3(void)
{
	bool exitMode3 = false;
	UART2_OutChar(MODE_3); // Send mode 3 to MCU2
	uint8_t charIndex = 0;
	mode3Menu();
	selectedMode = 3;

	while (!exitMode3)
	{
		// Wait until the full string is received
		while ((!endOfString0) && (selectedMode != 0))
		{
			WaitForInterrupt();
		}
		endOfString0 = false;
		strIndex0 = 0;
		OutCRLF0();

		// Send the received string to MCU2
		while ((charIndex != MAX_STR_LEN) && (selectedMode != 0))
		{
			UART2_OutChar(string0[charIndex]);
			charIndex += 1;
		}
		charIndex = 0;

		// Wait for a response from MCU2
		while ((!isUart2Waiting) && (selectedMode != 0))
		{
			WaitForInterrupt();
		}
		isUart2Waiting = false;

		// Display the received message from MCU2
		UART0_OutString((uint8_t *)string2);
		OutCRLF0();

		// Check if mode is changed to exit
		if (selectedMode == 0)
		{
			exitMode3 = true;
		}
	}

	UART2_OutChar(MODE_1);
}

// Displays the Mode 2 Menu
void mode2Menu(void)
{
	OutCRLF0();
	UART0_OutString((uint8_t *)"Mode 2 MCU1: press ^ to exit this mode");
	OutCRLF0();
	UART0_OutString((uint8_t *)"In color Wheel State.");
	OutCRLF0();
	UART0_OutString((uint8_t *)"Please press SW2 to cycle through colors:");
	OutCRLF0();
	UART0_OutString((uint8_t *)"Dark, Red, Green, Blue, Yellow, Cyan, Purple, White");
	OutCRLF0();
	UART0_OutString((uint8_t *)"Press SW1 to send the selected color to MCU2.");
	OutCRLF0();
	UART0_OutString((uint8_t *)"Current color: ");

	// Set current LED color based on the index
	currentLedColor = color_wheel[ledColorIndex];

	// Update LED color
	LED = currentLedColor;
	isUart2Waiting = false;

	// Display the selected color
	displayColor();
	
}

// Displays the currently selected LED color
void displayColor(void)
{
	const char *colorNames[] = {"Dark", "Red", "Green", "Blue", "Yellow", "Cyan", "Purple", "White"};
	UART0_OutString((uint8_t *)colorNames[ledColorIndex]);
	OutCRLF0();
}

// Function to set the LED color in Mode1
void SetColor(void)
{
	OutCRLF0();
	UART0_OutString((uint8_t *)"Please select a color from the following list: ");
	OutCRLF0();
	UART0_OutString((uint8_t *)"d(dark), r(red), g(green), b(blue), y(yellow), c(cyan), p(purple), w(white)");
	OutCRLF0();

	while (!endOfString0)
	{ // wait until the whole string is received.
		WaitForInterrupt();
	}
	endOfString0 = false;
	strIndex0 = 0;
	OutCRLF0();
	// Match the input to the index in the arrays
	int index = -1;
	switch (string0[0])
	{
	case 'd':
		index = 0;
		OutCRLF0();
		UART0_OutString((uint8_t *)"LED is off.");
		break;
	case 'r':
		index = 1;
		OutCRLF0();
		UART0_OutString((uint8_t *)"RED LED is on.");
		break;
	case 'g':
		index = 2;
		OutCRLF0();
		UART0_OutString((uint8_t *)"Green LED is on.");
		break;
	case 'b':
		index = 3;
		OutCRLF0();
		UART0_OutString((uint8_t *)"Blue LED is on.");
		break;
	case 'y':
		index = 4;
		OutCRLF0();
		UART0_OutString((uint8_t *)"Yellow LED is on.");
		break;
	case 'c':
		index = 5;
		OutCRLF0();
		UART0_OutString((uint8_t *)"Cyan LED is on.");
		break;
	case 'p':
		index = 6;
		OutCRLF0();
		UART0_OutString((uint8_t *)"Purple LED is on.");
		break;	
	case 'w':
		index = 7;
		OutCRLF0();
		UART0_OutString((uint8_t *)"White LED is on.");
		break;
	default:
		OutCRLF0();
		UART0_OutString((uint8_t *)"Invalid selection.");
		break;
	}

	// Set the LED color and print the corresponding message
	currentLedColor = color_wheel[index];
	OutCRLF0();

	LED = currentLedColor; // Set the selected color to the onboard LED
}

// Function to set the LED brightness in Mode1
void SetBrightness(void)
{
	OutCRLF0();
	UART0_OutString((uint8_t *)"Please enter a brightness percentage (0-100):");
	OutCRLF0();

	while (!endOfString0)
	{ // wait until the whole string is received.
		WaitForInterrupt();
	}
	endOfString0 = false;
	strIndex0 = 0;
	OutCRLF0();

	// Convert string to a decimal number
	uint32_t brightnessPercentage = StringToDecimal(string0);

	// Validate input percentage
	if (brightnessPercentage > 100)
	{
		OutCRLF0();
		UART0_OutString((uint8_t *)"Invalid brightness percentage.");
		OutCRLF0();
		return;
	}

	// Convert decimal to string for display
	DecimalToString(string0, brightnessPercentage);

	// Display the selected brightness level
	OutCRLF0();
	UART0_OutString((uint8_t *)" LED is set to ");
	UART0_OutString((uint8_t *)string0);
	UART0_OutString((uint8_t *)"% brightness.");
	OutCRLF0();

	// Set high and low times for PWM based on the brightness
	highTime = (PERIOD * brightnessPercentage / 100);
	lowTime = PERIOD - highTime;

	// Ensure the times are not zero, as that would cause issues
	if (highTime == 0)
	{
		highTime = 10;
	}
	if (lowTime == 0)
	{
		lowTime = 10;
	}
}

// Displays the main menu
void DisplayMenu(void)
{
	OutCRLF0();
	UART0_OutString((uint8_t *)"Welcome to CECS 447 Project 2 - UART ");
	OutCRLF0();
	UART0_OutString((uint8_t *)"MCU1 Main Menu");
	OutCRLF0();
	UART0_OutString((uint8_t *)"1. PC <-> MCU1 LED Control.");
	OutCRLF0();
	UART0_OutString((uint8_t *)"2. MCU1 <-> MCU2 Color Wheel.");
	OutCRLF0();
	UART0_OutString((uint8_t *)"3. PC <-> MCU1 <-> MCU2 <-> PC Chat Room");
	OutCRLF0();
	UART0_OutString((uint8_t *)"Please choose a communication mode");
	OutCRLF0();
	UART0_OutString((uint8_t *)"(enter 1, 2, or 3):");
	OutCRLF0();

	while (!endOfString0)
	{
		WaitForInterrupt();
	}
	endOfString0 = false;
	strIndex0 = 0;
	OutCRLF0();
}

// Displays the Mode 3 Menu
void mode3Menu(void)
{
	OutCRLF0();
	UART0_OutString((uint8_t *)"Mode 3 MCU1: Chat Room");
	OutCRLF0();
	UART0_OutString((uint8_t *)"Press SW1 to exit the chat room.");
	OutCRLF0();
	UART0_OutString((uint8_t *)"Type a message and press Enter (max 20 characters):");
	OutCRLF0();
}

// UART0 interrupt handler (for receiving input)
void UART0_Handler(void)
{
	uint8_t receivedChar;

	if (UART0_RIS_R & UART_RIS_RXRIS) // Received data interrupt
	{
		if ((UART0_FR_R & UART_FR_RXFE) == 0)
		{
			receivedChar = UART0_DR_R & 0xFF;
			if (receivedChar == CR) // End of string
			{
				endOfString0 = true;
				string0[strIndex0] = '\0'; // Null-terminate the string
			}
			else if (strIndex0 < (MAX_STR_LEN - 1))
			{
				if (receivedChar == BS)
				{
					UART0_OutChar(BS);
					strIndex0--;
				}
				else
				{
					string0[strIndex0++] = receivedChar; // Add received character to string
					UART0_OutChar(receivedChar);
				}
			}
		}
		UART0_ICR_R = UART_ICR_RXIC; // Clear the interrupt flag
	}
}

// UART2 interrupt handler (for receiving input from MCU2)
void UART2_Handler(void)
{
	if (UART2_RIS_R & UART_RIS_RXRIS)
	{
		if ((UART2_FR_R & UART_FR_RXFE) == 0)
		{
			if (selectedMode == 2)
			{
				ledColorIndex = UART2_DR_R;
				currentLedColor = color_wheel[ledColorIndex];
				LED = currentLedColor;
				isUart2Waiting = false;
				uart2InterruptFlag = false; // Set the flag to false when a color is received
			}

			if (selectedMode == 3)
			{
				string2[strIndex2] = UART2_DR_R;
				strIndex2++;
				if (strIndex2 > 19)
				{
					isUart2Waiting = true;
					strIndex2 = 0;
				}
			}

			if (UART2_DR_R == MODE_1)
			{
				selectedMode = 0;
			}
		}
		UART2_ICR_R = UART_ICR_RXIC; // Clear the interrupt flag
	}
}

// GPIO PortF interrupt handler (for handling switches)
void GPIOPortF_Handler(void)
{
	for (int i = 0; i <= 199999; i++)
	{
	} // delay for button press

	if (GPIO_PORTF_RIS_R & SW2)
	{ // SW 2 PRESS
		GPIO_PORTF_ICR_R |= SW2;
		if (selectedMode == 2)
		{
			ledColorIndex = (ledColorIndex + 1) % color_wheel_size;
			currentLedColor = color_wheel[ledColorIndex];
			LED = currentLedColor;
			UART0_OutString((uint8_t *)"Current color: ");
			displayColor();
			//OutCRLF0();
		}
	}

	if (GPIO_PORTF_RIS_R & SW1)
	{ // SW 1 PRESS
		GPIO_PORTF_ICR_R |= SW1;
		if (selectedMode == 2)
		{
			OutCRLF0();
			UART2_OutChar(ledColorIndex);
			waitForInterruptFlag = false;
			
			UART0_OutString((uint8_t *)"Mode 2 MCU1: press ^ to exit");
			OutCRLF0();
			UART0_OutString((uint8_t *)"Current Color: ");
			displayColor(); // display current LED
			UART0_OutString((uint8_t *)"Waiting for color from MCU2...");
			OutCRLF0();
		}
		else if (selectedMode == 3)
		{
			selectedMode = 0;
		}

		for (int i = 0; i <= 299999; i++)
		{
		} // delay for button press
	}
}

// SysTick initialization
void SysTick_Init(void)
{
	NVIC_ST_CTRL_R = 0;																   // Disable SysTick during setup
	NVIC_ST_RELOAD_R = HALF_DUTY - 1;												   // Set reload value for 50% duty cycle
	NVIC_ST_CURRENT_R = 0;															   // Clear current register
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x1FFFFFFF) | 0x40000000;					   // Set priority
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC + NVIC_ST_CTRL_INTEN + NVIC_ST_CTRL_ENABLE; // Enable SysTick
}

// SysTick interrupt handler
void SysTick_Handler(void)
{
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; // Turn off SysTick

	if (isPwmHigh)
	{
		NVIC_ST_RELOAD_R = lowTime; // Set low time for the non-duty cycle
		isPwmHigh = false;
		LED = DARK;
	}
	else
	{
		NVIC_ST_RELOAD_R = highTime; // Set high time for the duty cycle
		isPwmHigh = true;
		LED = currentLedColor;
	}

	NVIC_ST_CURRENT_R = 0;				   // Clear current register
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE; // Enable SysTick for next cycle
}

// Convert a string to a decimal number
uint32_t StringToDecimal(uint8_t str[])
{
	uint32_t number = 0;
	uint8_t character, idx = 0;

	character = str[idx];
	while (character != '\0')
	{
		if (character >= '0' && character <= '9')
		{
			number = 10 * number + (character - '0'); // Convert each character to a digit
		}
		else
		{
			return number; // Stop if a non-decimal character is found
		}
		character = str[++idx]; // Get the next character
	}
	return number;
}

// Convert a decimal number to a string
void DecimalToString(uint8_t str[], uint32_t n)
{
	static uint8_t idx = 0;

	if ((n / 10) != 0)
	{
		DecimalToString(str, n / 10); // Recursive call to process each digit
		DecimalToString(str, n % 10); // Process remainder
	}
	else
	{
		str[idx++] = n % 10 + '0'; // Convert digit to character
		return;
	}

	str[idx] = '\0'; // Null-terminate the string
	return;
}