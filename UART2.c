// CECS 447: Project 2 - UART Communications
// File Name: UART2.c
// Purpose: UART driver for initializing, sending, and receiving data on the TM4C123 microcontroller.
// Group: Robby Rimal 
// 				Jonathan Cerniaz 
// 				Joseph Guzman
// 				Afzal Hakim

#include "tm4c123gh6pm.h"
#include "UART2.h"
#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

#define NVIC_EN1_UART2 0x00000002     // UART2 IRQ number 33

//------------UART_Init------------
// Initialize the UART for 38400 baud rate (assuming 50 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART2_Init(bool RxInt, bool TxInt){
	if ((SYSCTL_RCGCUART_R&SYSCTL_RCGC1_UART2)==0) {
		SYSCTL_RCGCUART_R |= SYSCTL_RCGC1_UART2;	// Activate UART2 clocks
	while ((SYSCTL_RCGCUART_R&SYSCTL_RCGC1_UART2)==0){};
	}
	if ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOD)==0) {
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;	// Activate D clocks
	while ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOD)==0){};
	}

  UART2_CTL_R = 0;                      // disable UART
  UART2_IBRD_R = 81;                    // IBRD = int(50,000,000 / (16 * 38400)) = int(81.3802083)
  UART2_FBRD_R = 24;                     // FBRD = int(0.3802083 * 64 + 0.5) = 24
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
//  UART2_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART2_LCRH_R = UART_LCRH_WLEN_8;
	
	// take care of interrupt setup
	if ( RxInt | TxInt) {
		NVIC_PRI8_R = (NVIC_PRI8_R&~0x0000E000)|0x00008000; // bits 15-13, priority 4
		NVIC_EN1_R = NVIC_EN1_UART2;           // enable UART2 interrupt in NVIC
		if (RxInt) {
			UART2_IM_R |= UART_IM_RXIM;         // Enable RX interrupt
		}
		
		if (TxInt) {
			UART2_IM_R |= UART_IM_TXIM;         // Enable TX interrupt
		}
	}

  UART2_CTL_R |= UART_CTL_RXE|UART_CTL_TXE|UART_CTL_UARTEN;// enable Tx, RX and UART
	GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;
	GPIO_PORTD_CR_R |= 0x80;
  GPIO_PORTD_AFSEL_R |= 0xC0;           // enable alt funct on PD6-7
  GPIO_PORTD_DEN_R |= 0xC0;             // enable digital I/O on PCD-7
                                        // configure PD6-7 as UART
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0x00FFFFFF)+0x11000000;
  GPIO_PORTD_AMSEL_R &= ~0xC0;          // disable analog functionality on PD6-7
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
uint8_t UART2_InChar(void){
  while((UART2_FR_R&UART_FR_RXFE) != 0); // wait until the receiving FIFO is not empty
  return((uint8_t)(UART2_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART2_OutChar(uint8_t data){
  while((UART2_FR_R&UART_FR_TXFF) != 0);
  UART2_DR_R = data;
}
