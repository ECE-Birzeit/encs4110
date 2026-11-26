// #region full
// #region defines
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOA     (1U << 0)        // RCGCGPIO bit for Port A
#define RCGC_UART0     (1U << 0)        // RCGCUART bit for UART0

// UART0's pins: wired to the LaunchPad's USB connection (the virtual COM port)
#define U0RX           (1U << 0)        // PA0
#define U0TX           (1U << 1)        // PA1
#define PCTL_PA01_MASK (0xFFU << 0)     // PA0's and PA1's fields in GPIOA->PCTL: bits 7:0
#define PCTL_PA01_UART (0x11U << 0)     // 1 selects U0RX and U0TX

// UART0 register bits
#define CTL_UARTEN     (1U << 0)        // CTL: UART enable
#define CTL_TXE        (1U << 8)        // CTL: transmit enable
#define CTL_RXE        (1U << 9)        // CTL: receive enable
#define LCRH_WLEN_8    (0x3U << 5)      // LCRH: 8 data bits (no parity, 1 stop bit by default)
#define LCRH_FEN       (1U << 4)        // LCRH: FIFOs on
#define FR_RXFE        (1U << 4)        // FR: receive FIFO empty
#define FR_TXFF        (1U << 5)        // FR: transmit FIFO full

// 115200 baud at 50 MHz: 50,000,000 / (16 * 115200) = 27.1267
#define BAUD_IBRD      27               // Integer part
#define BAUD_FBRD      8                // Fraction: 0.1267 * 64 + 0.5 = 8

#define LINE_MAX       32               // Longest line read, including the terminating 0
// #endregion defines


// #region send
void UART0_Init(void)
{
	SYSCTL->RCGCUART |= RCGC_UART0;
	SYSCTL->RCGCGPIO |= RCGC_GPIOA;
	while ((SYSCTL->PRUART & RCGC_UART0) == 0);
	while ((SYSCTL->PRGPIO & RCGC_GPIOA) == 0);

	GPIOA->AFSEL |= U0RX | U0TX;										// Driven by a peripheral...
	GPIOA->PCTL   = (GPIOA->PCTL & ~PCTL_PA01_MASK) | PCTL_PA01_UART;	// ...UART0
	GPIOA->DEN   |= U0RX | U0TX;

	UART0->CTL  = 0;							// Off while configuring
	UART0->IBRD = BAUD_IBRD;					// Baud rate: 115200
	UART0->FBRD = BAUD_FBRD;
	UART0->LCRH = LCRH_WLEN_8 | LCRH_FEN;		// 8N1, FIFOs on (written after the divisors)
	UART0->CC   = 0;							// Clock: the system clock
	UART0->CTL  = CTL_UARTEN | CTL_TXE | CTL_RXE;	// On: send and receive
}

void UART0_WriteChar(char c)
{
	while (UART0->FR & FR_TXFF);				// Wait for room in the transmit FIFO
	UART0->DR = (uint8_t)c;
}

void UART0_WriteString(const char *str)
{
	while (*str)
		UART0_WriteChar(*str++);
}
// #endregion send


// #region receive
char UART0_ReadChar(void)
{
	while (UART0->FR & FR_RXFE);				// Wait until a character has arrived
	return (char)(UART0->DR & 0xFF);			// Bits 11:8 are error flags: drop them
}

// Read a line typed in the terminal, echoing it back so you can see what you type.
// Stops at Enter; Backspace removes the last character. Always 0-terminated.
void UART0_ReadLine(char *buf, uint32_t size)
{
	uint32_t n = 0;
	char c;

	while (1)
	{
		c = UART0_ReadChar();
		if (c == '\r' || c == '\n')				// Enter: the line is done
			break;
		if ((c == '\b' || c == 0x7F) && n > 0)	// Backspace (PuTTY sends 0x7F)
		{
			n--;
			UART0_WriteString("\b \b");			// Move back, blank it, move back again
		}
		else if (c >= ' ' && n < size - 1)		// A printable character that still fits
		{
			buf[n++] = c;
			UART0_WriteChar(c);					// Echo it
		}
	}
	buf[n] = '\0';
	UART0_WriteString("\r\n");
}
// #endregion receive


// #region main
int main(void)
{
	char line[LINE_MAX];

	UART0_Init();
	UART0_WriteString("\r\nENCS4110 UART ready\r\n");

	while (1)
	{
		UART0_WriteString("> ");				// A prompt
		UART0_ReadLine(line, LINE_MAX);
		UART0_WriteString("You typed: ");
		UART0_WriteString(line);
		UART0_WriteString("\r\n");
	}
}
// #endregion main
// #endregion full
