// CECS 447: Project 2 - UART Communications
// File Name: LEDSW.c
// Purpose: Initializes Port F, sets up PF0 and PF4 as inputs for switches with interrupts, and PF1-3 as outputs for LEDs.
// Group: Robby Rimal 
// 				Jonathan Cerniaz 
// 				Joseph Guzman
// 				Afzal Hakim
// Last Modified: 9/16/2024

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "LEDSW.h"


void LEDSW_Init(void) {
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;        // (a) Activate clock for Port F
    while ((SYSCTL_RCGC2_R & SYSCTL_RCGC2_GPIOF) == 0) {} // Wait for clock to be ready
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;           // Unlock PortF PF0
    GPIO_PORTF_CR_R |= 0x1F;                     // Allow changes to PF4-0
    GPIO_PORTF_DIR_R |= 0x0E;                    // (b) Configure PF3-1 as outputs (LEDs)
    GPIO_PORTF_DIR_R &= ~0x11;                   // Configure PF4 and PF0 as inputs (switches)
    GPIO_PORTF_AFSEL_R &= ~0x1F;                 // Disable alternate function on PF4-0
    GPIO_PORTF_DEN_R |= 0x1F;                    // Enable digital I/O on PF4-0
    GPIO_PORTF_PCTL_R &= ~0x000FFFFF;            // Configure PF4-0 as GPIO
    GPIO_PORTF_AMSEL_R &= ~0x1F;                 // Disable analog functionality on PF4-0
    GPIO_PORTF_PUR_R |= 0x11;                    // Enable weak pull-up on PF0 and PF4 (switches)
    GPIO_PORTF_IS_R &= ~0x11;                    // (c) Configure PF0 and PF4 for edge-sensitive interrupts
    GPIO_PORTF_IBE_R &= ~0x11;                   // PF0 and PF4 are not both edges triggered
    GPIO_PORTF_IEV_R |= 0x11;                    // PF0 and PF4 interrupt on rising edge event
    GPIO_PORTF_ICR_R = 0x11;                     // (d) Clear interrupt flags for PF0 and PF4
    GPIO_PORTF_IM_R |= 0x11;                     // (e) Arm interrupt on PF0 and PF4
    NVIC_PRI7_R = (NVIC_PRI7_R & ~0x00E00000) | 0x00C00000; // (f) Set interrupt priority 6
    NVIC_EN0_R |= NVIC_EN0_PORTF;                // (g) Enable interrupt 30 in NVIC (Port F)
}
