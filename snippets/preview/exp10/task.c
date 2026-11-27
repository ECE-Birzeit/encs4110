// Experiment 10 Task: an LED command line.
//   Type a command in PuTTY and press Enter: red, green, blue, off, or status.
//   The board answers each one; anything else gets a short error.
// Provided: the Activity's UART functions, the LEDs, show(), equals(), UART0_WriteNumber(),
// and main. The "red" command is done as an example.
// Write the four TODOs (highlighted).
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOA     (1U << 0)        // RCGCGPIO bit for Port A
#define RCGC_GPIOF     (1U << 5)        // RCGCGPIO bit for Port F
#define RCGC_UART0     (1U << 0)        // RCGCUART bit for UART0

// UART0's pins: wired to the LaunchPad's USB connection (the virtual COM port)
#define U0RX           (1U << 0)        // PA0
#define U0TX           (1U << 1)        // PA1
#define PCTL_PA01_MASK (0xFFU << 0)     // PA0's and PA1's fields in GPIOA->PCTL: bits 7:0
#define PCTL_PA01_UART (0x11U << 0)     // 1 selects U0RX and U0TX

// Port F pins
#define RED_LED        (1U << 1)        // PF1
#define BLUE_LED       (1U << 2)        // PF2
#define GREEN_LED      (1U << 3)        // PF3
#define LEDS           (RED_LED | BLUE_LED | GREEN_LED)

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

uint32_t commands;                      // How many lines have been typed since reset


// ---------------------------------------------------------------- provided: don't change
// The Activity's functions, unchanged
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
		if (c == '\b' || c == 0x7F)			// Backspace (PuTTY sends 0x7F)
		{
			if (n > 0)							// Nothing to delete at an empty prompt
			{
				n--;
				UART0_WriteString("\b \b");		// Move back, blank it, move back again
			}
		}
		else if (c >= ' ' && c < 0x7F && n < size - 1)	// A printable character that still fits
		{
			buf[n++] = c;
			UART0_WriteChar(c);					// Echo it
		}
	}
	buf[n] = '\0';
	UART0_WriteString("\r\n");
}

// Print n in decimal: 0, 7, 1234...
void UART0_WriteNumber(uint32_t n)
{
	char digits[10];							// A uint32_t has at most 10 digits
	uint32_t count = 0;

	do
	{
		digits[count++] = (char)('0' + n % 10);	// The last digit first...
		n /= 10;
	} while (n > 0);

	while (count > 0)
		UART0_WriteChar(digits[--count]);		// ...so print them in reverse
}

// 1 if the strings a and b are the same, 0 if not
int equals(const char *a, const char *b)
{
	while (*a && *a == *b)
	{
		a++;
		b++;
	}
	return *a == *b;							// Both ended together: the same
}

// Port F: the three LEDs as outputs, all off
void LEDs_Init(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);

	GPIOF->DIR  |= LEDS;
	GPIOF->DEN  |= LEDS;
	GPIOF->DATA &= ~LEDS;
}

// Light exactly the LEDs in `led` (0 for off), turning the others off
void show(uint32_t led)
{
	GPIOF->DATA = (GPIOF->DATA & ~LEDS) | led;
}
// ---------------------------------------------------------------- end of provided part


// Do one command, and answer it
void run_command(const char *line)
{
	uint32_t lit;

	if (line[0] == '\0')						// Just Enter: nothing to do
		return;
	commands++;

	if (equals(line, "red"))					// A worked example
	{
		show(RED_LED);
		UART0_WriteString("OK\r\n");
	}
	// TODO 1: add "green" and "blue" the same way, and "off": all the LEDs off // [!code highlight]
	else if (equals(line, "status"))
	{
		lit = GPIOF->DATA & LEDS;				// The LED bits that are on now
		UART0_WriteString("LED: ");
		// TODO 2: print red, green, blue or off, depending on lit // [!code highlight]
		(void)lit;								// Delete this line when you use lit
		UART0_WriteString(", commands: ");
		// TODO 3: print the number of commands, then end the line with "\r\n" // [!code highlight]
	}
	// TODO 4: anything else: print "Unknown command: ", the line, and "\r\n" // [!code highlight]
}


int main(void)
{
	char line[LINE_MAX];

	UART0_Init();
	LEDs_Init();
	UART0_WriteString("\r\nLED command line: red, green, blue, off, status\r\n");

	while (1)
	{
		UART0_WriteString("> ");				// A prompt
		UART0_ReadLine(line, LINE_MAX);
		run_command(line);
	}
}
