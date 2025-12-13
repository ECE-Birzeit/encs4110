#include "uart.h"

void UART0_WriteChar(char c) {
    while ((UART0->FR & (1 << 5)) != 0); // Wait until TX FIFO is not full (TXFF bit 5)
    UART0->DR = c; // Write character to Data Register
}

void UART0_WriteString(char *str) {
    while (*str) { // Loop through each character until null terminator
        UART0_WriteChar(*(str++)); // Send each character
    }
}

void UART0_Init() {
    SYSCTL->RCGCUART |= (1 << 0); // Enable clock to UART0 module
    SYSCTL->RCGCGPIO |= (1 << 0); // Enable clock to GPIO Port A

    GPIOA->AFSEL |= U0_RX | U0_TX; // Enable alternate function on PA0 (RX) and PA1 (TX)
    GPIOA->PCTL |= (1 << 0) | (1 << 4); // Configure PA0 and PA1 for UART (U0RX/U0TX)
    GPIOA->DEN |= U0_RX | U0_TX; // Enable digital function for PA0 and PA1

    UART0->CTL = 0; // Disable UART0 while configuring
    UART0->IBRD = 27; // Set integer baud rate divisor for 115200 baud @ 50MHz
    UART0->FBRD = 8; // Set fractional baud rate divisor
    UART0->LCRH = 0x60; // 8 data bits, no parity, one stop bit, FIFOs enabled
    UART0->CC = 0; // Use system clock for UART
    UART0->CTL = (1 << 0) | (1 << 8) | (1 << 9); // Enable UART0, TX and RX
}

char UART0_ReadChar(void) {
    while (UART0->FR & (1 << 4)); // Wait while RX FIFO empty (RXFE bit 4)
    return (char)(UART0->DR & 0xFF); // Return received character
}

void UART0_ReadString(char *buffer, int maxLen) {
    int i = 0;
    char c;

    while (i < (maxLen - 1)) { // Leave space for null terminator
        c = UART0_ReadChar();
        if (c == '\r' || c == '\n') { // End of input
            break;
        }
        buffer[i++] = c;
    }

    buffer[i] = '\0'; // Null terminate the string
}
