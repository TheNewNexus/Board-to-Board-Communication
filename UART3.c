// CECS 447: Project 2 - UART Communications
// File Name: UART3.c
// Purpose: UART driver for initializing, sending, and receiving data on the TM4C123 microcontroller.
// Group: Robby Rimal 
// 				Jonathan Cerniaz 
// 				Joseph Guzman
// 				Afzal Hakim

#include "tm4c123gh6pm.h"
#include "UART3.h"
#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

#define NVIC_EN1_UART3 0x08000000     // UART3 IRQ number 59

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 50 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART3_Init(bool RxInt, bool TxInt){
	if ((SYSCTL_RCGCUART_R&SYSCTL_RCGC1_UART3)==0) {
		SYSCTL_RCGCUART_R |= SYSCTL_RCGC1_UART3;	// Activate UART2 clocks
	while ((SYSCTL_RCGCUART_R&SYSCTL_RCGC1_UART3)==0){};
	}
	if ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOC)==0) {
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC;	// Activate C clocks
	while ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOC)==0){};
	}

  UART3_CTL_R = 0;                      // disable UART
  UART3_IBRD_R = 81;                    // IBRD = int(50,000,000 / (16 * 38400)) = int(81.3802083)
  UART3_FBRD_R = 24;                     // FBRD = int(0.3802083 * 64 + 0.5) = 24
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
//  UART3_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART3_LCRH_R = UART_LCRH_WLEN_8;
	
	// take care of interrupt setup
	if ( RxInt | TxInt) {
		NVIC_PRI14_R = (NVIC_PRI14_R&~0xE0000000)|0x80000000; // bits 15-13, priority 4
		NVIC_EN1_R = NVIC_EN1_UART3;           // enable UART2 interrupt in NVIC
		if (RxInt) {
			UART3_IM_R |= UART_IM_RXIM;         // Enable RX interrupt
		}
		
		if (TxInt) {
			UART3_IM_R |= UART_IM_TXIM;         // Enable TX interrupt
		}
	}

  UART3_CTL_R |= UART_CTL_RXE|UART_CTL_TXE|UART_CTL_UARTEN;// enable Tx, RX and UART
  GPIO_PORTC_AFSEL_R |= 0xC0;           // enable alt funct on PC6-7
  GPIO_PORTC_DEN_R |= 0xC0;             // enable digital I/O on PC6-7
                                        // configure PC6-7 as UART
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0x00FFFFFF)+0x11000000;
  GPIO_PORTC_AMSEL_R &= ~0xC0;          // disable analog functionality on PC6-7
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
uint8_t UART3_InChar(void){
  while((UART3_FR_R&UART_FR_RXFE) != 0); // wait until the receiving FIFO is not empty
  return((uint8_t)(UART3_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART3_OutChar(uint8_t data){
  while((UART3_FR_R&UART_FR_TXFF) != 0);
  UART3_DR_R = data;
}
