# CECS 447 Project 2 - UART Communication System

## Overview

This project implements a UART-based communication system using two TM4C123 LaunchPads (MCU1 and MCU2) and two PC serial terminals. The system operates in three different modes, allowing for LED control, color exchange, and a chat room functionality.

## Features

- **Mode 1**: PC1-MCU1 LED Control
  - Control LED colors on MCU1
  - Adjust LED brightness using PWM

- **Mode 2**: Color Exchange
  - Involves all four components (PC1, MCU1, MCU2, PC2)
  - Alternating color selection and transmission between MCUs

- **Mode 3**: Chat Room
  - Real-time messaging between two PC terminals via MCUs
  - UART Rx Interrupt-driven communication

## Hardware Requirements

- 2x TM4C123 LaunchPads
- 2x PCs with serial terminal applications (e.g., Tera Term, PuTTY)
- USB cables for connecting LaunchPads to PCs

## Software Requirements

- Code Composer Studio or Keil uVision IDE
- Serial terminal application

## Setup

1. Connect MCU1 to PC1 and MCU2 to PC2 via USB.
2. Connect MCU1's UART2 to MCU2's UART3 for inter-MCU communication.
3. Configure serial terminals with the following settings:
   - PC1↔MCU1, PC2↔MCU2: 57600 bps
   - MCU1↔MCU2: 38400 bps

## Usage

1. Compile and flash the respective code to MCU1 and MCU2.
2. Open serial terminals on both PCs.
3. Follow the on-screen menu to select and operate in different modes.

## Mode Descriptions

### Mode 1: PC1-MCU1 LED Control
- Control LED colors on MCU1
- Adjust LED brightness

### Mode 2: Color Exchange
- Use SW2 to cycle through colors
- Press SW1 to send color to the other MCU
- Enter '^' on PC1 terminal to exit

### Mode 3: Chat Room
- Type messages on either PC terminal
- Press SW1 on either MCU to exit

## Extra Challenge

Implement Mode 3 using UART DMA for MCU1 UART2 and MCU2 UART3.

## Contributors


