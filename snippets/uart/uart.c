#include "uart.h"

// Clocks
#define RCGC_UART0      (1U << 0)       // RCGCUART bit for UART0
#define RCGC_GPIOA      (1U << 0)       // RCGCGPIO bit for Port A

// PCTL: 1 in a pin's 4-bit field selects UART0
#define PCTL_PA01_MASK  0xFFU           // PA0's and PA1's fields: bits 7:0
#define PCTL_PA01_UART  0x11U           // U0RX on PA0, U0TX on PA1

// UART0 register values
#define FR_RXFE         (1U << 4)       // FR: receive FIFO empty
#define FR_TXFF         (1U << 5)       // FR: transmit FIFO full
#define IBRD_115200     27              // 50 MHz / (16 * 115200) = 27.13...
#define FBRD_115200     8               // 0.13 * 64 + 0.5 = 8
#define LCRH_WLEN_8     (0x3U << 5)     // LCRH: 8 data bits
#define LCRH_FEN        (1U << 4)       // LCRH: FIFOs on
#define CC_SYSCLK       0x0             // CC: clock from the system clock
#define CTL_UARTEN      (1U << 0)       // CTL: UART on
#define CTL_TXE         (1U << 8)       // CTL: transmit on
#define CTL_RXE         (1U << 9)       // CTL: receive on

void UART0_Init(void)
{
	SYSCTL->RCGCUART |= RCGC_UART0;                 // Clocks to UART0 and Port A
	SYSCTL->RCGCGPIO |= RCGC_GPIOA;
	while ((SYSCTL->PRUART & RCGC_UART0) == 0);     // Wait until both are ready
	while ((SYSCTL->PRGPIO & RCGC_GPIOA) == 0);

	GPIOA->AFSEL |= U0_RX | U0_TX;                  // PA0 and PA1: alternate function...
	GPIOA->PCTL   = (GPIOA->PCTL & ~PCTL_PA01_MASK) | PCTL_PA01_UART;  // ...UART0
	GPIOA->DEN   |= U0_RX | U0_TX;

	UART0->CTL  = 0;                                // Off while configuring
	UART0->IBRD = IBRD_115200;                      // 115200 baud at 50 MHz
	UART0->FBRD = FBRD_115200;
	UART0->LCRH = LCRH_WLEN_8 | LCRH_FEN;           // 8 data bits, no parity, 1 stop bit, FIFOs on
	UART0->CC   = CC_SYSCLK;
	UART0->CTL  = CTL_UARTEN | CTL_TXE | CTL_RXE;   // On, transmit and receive
}

void UART0_WriteChar(char c)
{
	while (UART0->FR & FR_TXFF);                    // Wait while the transmit FIFO is full
	UART0->DR = c;
}

void UART0_WriteString(char *str)
{
	while (*str)                                    // Up to the terminating '\0'
		UART0_WriteChar(*str++);
}

char UART0_ReadChar(void)
{
	while (UART0->FR & FR_RXFE);                    // Wait while the receive FIFO is empty
	return (char)(UART0->DR & 0xFF);
}

void UART0_ReadString(char *buffer, int maxLen)
{
	int i = 0;
	char c;

	while (i < maxLen - 1)                          // Leave room for the '\0'
	{
		c = UART0_ReadChar();
		if (c == '\r' || c == '\n')                 // Enter ends the line
			break;
		buffer[i++] = c;
	}
	buffer[i] = '\0';
}
