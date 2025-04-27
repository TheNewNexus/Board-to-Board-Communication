# Advanced Board-to-Board UART Communication System for Embedded Applications

## Overview

This project presents a sophisticated UART (Universal Asynchronous Receiver/Transmitter) communication system designed for two TM4C123 LaunchPads (designated as MCU1 and MCU2) and interfaced with two PC serial terminals. The system is engineered to operate in three distinct modes, showcasing advanced functionalities such as LED control with PWM (Pulse Width Modulation), bidirectional color data exchange, and a real-time chat room application. This robust system leverages interrupt-driven communication and is designed for seamless integration in embedded environments.

## Key Features

-   **Modular Design:** The system is structured into three operational modes, each designed to highlight different aspects of UART communication.
-   **Advanced LED Control (Mode 1):**
    -   Precise control over LED color mixing on MCU1 using RGB channels.
    -   Dynamic adjustment of LED brightness via high-resolution PWM, enabling smooth transitions and dimming effects.
-   **Bidirectional Color Exchange (Mode 2):**
    -   Full-duplex color data transmission between MCU1 and MCU2, facilitated by PC1 and PC2.
    -   Implements a color selection mechanism with synchronized transmission, ensuring data integrity and minimal latency.
-   **Real-time Chat Room (Mode 3):**
    -   Low-latency, bi-directional chat application between PC terminals, mediated by MCU1 and MCU2.
    -   Employs UART Rx Interrupts for efficient data handling, ensuring responsiveness and minimal CPU overhead.

## Hardware Specifications

-   2x TM4C123 LaunchPads with advanced peripheral support.
-   2x Host PCs equipped with serial communication interfaces (e.g., USB-to-Serial adapters).
-   High-quality USB cables ensuring stable data and power transmission to the LaunchPads.

## Software Stack

-   **Development IDEs:** Code Composer Studio (CCS) or Keil uVision, configured for ARM Cortex-M4F architecture.
-   **Serial Terminal Emulators:** Tera Term, PuTTY, or similar, with advanced configuration options for debugging and data monitoring.
-   **Custom Libraries:** Developed in C, including optimized UART drivers, interrupt handlers, and PWM modules.

## System Setup

1.  Establish physical connections: Connect MCU1 to PC1 and MCU2 to PC2 using USB cables.
2.  Inter-MCU UART Configuration: Connect MCU1's UART2 TX/RX pins to MCU2's UART3 RX/TX pins, respectively, ensuring proper signal routing.
3.  Configure serial terminals with the following communication parameters:
    -   PC1↔MCU1, PC2↔MCU2: 57600 bps, 8-N-1 (8 data bits, no parity, 1 stop bit)
    -   MCU1↔MCU2: 38400 bps, 8-N-1
4.  Ensure that all connections are secure and properly grounded to minimize signal noise and interference.

## Operational Instructions

1.  Compile and flash the respective firmware onto MCU1 and MCU2 using the designated IDE.
2.  Launch serial terminal applications on both PCs, configuring them to connect to the appropriate COM ports.
3.  Upon successful connection, the system will present a menu-driven interface, allowing users to select and operate in different modes.

## Mode Details

### Mode 1: Advanced LED Control

-   Utilize PC1 to precisely control the color and brightness of the RGB LED on MCU1.
-   Implement a user-friendly interface for specifying RGB values and PWM duty cycles.

### Mode 2: Synchronized Color Exchange

-   Employ SW2 on MCU1 to cycle through a predefined color palette.
-   Press SW1 to initiate a synchronized color transmission to MCU2, which will then update its LED color accordingly.
-   Terminate the mode by entering '^' in the PC1 terminal.

### Mode 3: Real-time Chat Room

-   Engage in real-time text-based communication between PC1 and PC2, with MCU1 and MCU2 acting as transparent intermediaries.
-   Exit the chat room by pressing SW1 on either MCU.

