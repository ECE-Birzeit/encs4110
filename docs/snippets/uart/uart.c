#include "uart.h"
#define MAX_STR_LEN 50
// Function to send a single character via UART0
void UART0_WriteChar(char c) {
    // Wait until the transmit FIFO is not full
    // UART0->FR bit 5 (TXFF) = 1 means FIFO is full, so wait until it becomes 0
    while ((UART0->FR & (1 << 5)) != 0);
    // Write the character to the Data Register to transmit
    UART0->DR = c;
}

// Function to send a null-terminated string via UART0
void UART0_WriteString(char *str) {
    // Loop through each character until null terminator
    while (*str) {
        // Send each character using UART0_WriteChar
        UART0_WriteChar(*(str++));
    }
}

// UART0 initialization function
void UART0_Init() {
    // Enable clock to UART0 module (bit 0)
    SYSCTL->RCGCUART |= (1 << 0);
    // Enable clock to GPIO Port A (bit 0)
    SYSCTL->RCGCGPIO |= (1 << 0);

    // Enable alternate function on PA0 (RX) and PA1 (TX)
    GPIOA->AFSEL |= U0_RX | U0_TX;
    // Configure PA0 and PA1 pins for UART function in Port Control Register
    // U0_RX corresponds to PA0 (bits 3:0) and U0_TX corresponds to PA1 (bits 7:4)
    GPIOA->PCTL |= (1 << 0) | (1 << 4);
    // Enable digital function for PA0 and PA1 pins
    GPIOA->DEN |= U0_RX | U0_TX;

    // Disable UART0 while configuring
    UART0->CTL = 0;
    // Set integer baud rate divisor for 115200 baud with 50MHz clock
    UART0->IBRD = 27;
    // Set fractional baud rate divisor
    UART0->FBRD = 8;
    // Configure Line Control for 8 data bits, no parity, one stop bit, and FIFOs enabled
    // 0x60 = 0b01100000: bit 6 (FEN) = 1 enable FIFO, bits 5-6 (WLEN) = 11 for 8 bits
    UART0->LCRH = 0x60;
    // Use system clock for UART
    UART0->CC = 0;
    // Enable UART0, TX and RX
    // Bit 0 = UARTEN, bit 8 = TXE, bit 9 = RXE
    UART0->CTL = (1 << 0) | (1 << 8) | (1 << 9);
}

char UART0_ReadChar(void) {
    while (UART0->FR & (1 << 4));  // Wait while RX FIFO empty
    return (char)(UART0->DR & 0xFF);
}

void UART0_ReadString(char *buffer, int maxLen) {
    int i = 0;
    char c;

    while (i < (maxLen - 1)) {   // Leave space for null terminator
        c = UART0_ReadChar();

        // Echo character back (optional)
        UART0_WriteChar(c);

        if (c == '\r' || c == '\n') {  // End of input
			UART0_WriteString("\r\n");
            break;
        }

        buffer[i++] = c;
    }

    buffer[i] = '\0';  // Null terminate the string
}

