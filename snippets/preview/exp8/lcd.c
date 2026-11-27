// lcd.c: the I2C and LCD driver from the Experiment 8 Activity, as a file of its own.
#include "lcd.h"

// Clocks
#define RCGC_GPIOB       (1U << 1)      // RCGCGPIO bit for Port B (I2C0 pins)
#define RCGC_I2C0        (1U << 0)      // RCGCI2C bit for I2C0

// I2C0's pins
#define I2C0SCL          (1U << 2)      // PB2
#define I2C0SDA          (1U << 3)      // PB3
#define PCTL_PB23_MASK   (0xFFU << 8)   // PB2's and PB3's fields in GPIOB->PCTL: bits 15:8
#define PCTL_PB23_I2C0   (0x33U << 8)   // 3 selects I2C0SCL and I2C0SDA


// I2C0 register bits
#define MCR_MFE          (1U << 4)      // MCR: master function enable
#define MTPR_100KHZ      24             // MTPR: 50 MHz / (2 * (6 + 4) * 100 kHz) - 1
#define MCS_RUN          (1U << 0)      // MCS written: send
#define MCS_START        (1U << 1)      // MCS written: START first
#define MCS_STOP         (1U << 2)      // MCS written: STOP after
#define MCS_BUSY         (1U << 0)      // MCS read: transfer in progress
#define MCS_ERROR        (1U << 1)      // MCS read: no ACK

// The LCD backpack: its address, and what each bit of the PCF8574 drives
#define LCD_ADDR         0x27
#define LCD_RS           (1U << 0)      // P0: 0 = command, 1 = character
#define LCD_RW           (1U << 1)      // P1: always 0 (write)
#define LCD_EN           (1U << 2)      // P2: enable, the LCD latches on its falling edge
#define LCD_BL           (1U << 3)      // P3: backlight
                                        // P4 to P7: D4 to D7


void I2C0_Init(void)
{
	SYSCTL->RCGCI2C  |= RCGC_I2C0;				// Clock to I2C0
	SYSCTL->RCGCGPIO |= RCGC_GPIOB;				// Clock to Port B
	while ((SYSCTL->PRGPIO & RCGC_GPIOB) == 0);	// Wait until both are ready
	while ((SYSCTL->PRI2C  & RCGC_I2C0)  == 0);

	GPIOB->AFSEL |= I2C0SCL | I2C0SDA;									// Driven by a peripheral...
	GPIOB->PCTL   = (GPIOB->PCTL & ~PCTL_PB23_MASK) | PCTL_PB23_I2C0;	// ...I2C0
	GPIOB->ODR   |= I2C0SDA;											// SDA open drain (not SCL)
	GPIOB->DEN   |= I2C0SCL | I2C0SDA;

	I2C0->MCR  = MCR_MFE;						// I2C0 is a master
	I2C0->MTPR = MTPR_100KHZ;					// SCL at 100 kHz
}


// Send one byte to the device at addr. Returns 0 if it answered (ACK), -1 if not.
int I2C0_Write(uint8_t addr, uint8_t data)
{
	I2C0->MSA = addr << 1;						// Address in bits 7:1, bit 0 = 0: write
	I2C0->MDR = data;
	I2C0->MCS = MCS_STOP | MCS_START | MCS_RUN;	// START, address, data, STOP

	while ((I2C0->MCS & MCS_BUSY) == 0);		// BUSY rises a few cycles after the write...
	while (I2C0->MCS & MCS_BUSY);				// ...and falls when the byte is done (about 0.2 ms)

	return (I2C0->MCS & MCS_ERROR) ? -1 : 0;
}

// A 1 ms clock: once main calls SysTick_Config(SystemCoreClock / 1000), SysTick interrupts
// every millisecond, and main.c's SysTick_Handler counts in ms. Like Arduino's millis().
volatile uint32_t ms;

uint32_t millis(void)
{
	return ms;
}

// Wait n milliseconds. Never call it from an interrupt handler: SysTick has the lowest
// priority, so inside another handler ms stops counting and this would wait forever.
void delay_ms(uint32_t n)
{
	uint32_t start = ms;
	while (ms - start < n);						// Unsigned subtraction: still right when ms wraps
}

// One nibble on D7-D4 with an enable pulse: three bytes to the backpack.
// Each byte takes about 0.2 ms, far longer than any of the LCD's timing minimums.
void LCD_SendNibble(uint8_t nibble, uint8_t rs)
{
	uint8_t out = (nibble << 4) | rs | LCD_BL;	// Backlight stays on in every byte

	I2C0_Write(LCD_ADDR, out);					// Data and RS settle, E low
	I2C0_Write(LCD_ADDR, out | LCD_EN);			// E high
	I2C0_Write(LCD_ADDR, out);					// E low: the LCD latches the nibble
}


void LCD_Command(uint8_t cmd)
{
	LCD_SendNibble(cmd >> 4,   0);				// Upper nibble, RS = 0
	LCD_SendNibble(cmd & 0x0F, 0);				// Lower nibble
	if (cmd <= 0x02)							// Clear and Home take 1.6 ms
		delay_ms(2);
}

void LCD_Data(uint8_t ch)
{
	LCD_SendNibble(ch >> 4,   LCD_RS);			// Upper nibble, RS = 1
	LCD_SendNibble(ch & 0x0F, LCD_RS);			// Lower nibble
}

void LCD_Init(void)
{
	delay_ms(50);								// Let the LCD power up

	LCD_SendNibble(0x03, 0);					// Same sequence as Experiment 8:
	delay_ms(5);								// three 0x3s...
	LCD_SendNibble(0x03, 0);
	delay_ms(1);
	LCD_SendNibble(0x03, 0);
	delay_ms(1);
	LCD_SendNibble(0x02, 0);					// ...then 0x2: 4-bit mode
	delay_ms(1);

	LCD_Command(0x28);							// 4-bit, 2 lines, 5x8 dots
	LCD_Command(0x0C);							// Display on, cursor off
	LCD_Command(0x06);							// Cursor moves right
	LCD_Command(0x01);							// Clear
}


void LCD_SetCursor(uint8_t row, uint8_t col)
{
	LCD_Command((row == 0 ? 0x80 : 0xC0) + col);
}

void LCD_Print(const char *str)						// const: a string literal can't be changed
{
	while (*str)
		LCD_Data(*str++);
}

void LCD_PrintNumber(uint32_t n)
{
	char digits[10];
	int  i = 0;

	do {										// Collect the digits, last one first
		digits[i++] = '0' + n % 10;
		n /= 10;
	} while (n > 0);

	while (i > 0)								// Send them first one first
		LCD_Data(digits[--i]);
}
