// Experiment 11 Task: an up/down counter on the LCD.
//   Line 1 shows "Counter", line 2 the count, starting at 0.
//   SW1 adds 1. SW2 takes 1 away, but the count never goes below 0.
// Provided: the Activity's I2C and LCD functions, the buttons, and their debounce.
// Write the four TODOs (highlighted).
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOB       (1U << 1)      // RCGCGPIO bit for Port B (I2C0 pins)
#define RCGC_GPIOF       (1U << 5)      // RCGCGPIO bit for Port F (LED, buttons)
#define RCGC_I2C0        (1U << 0)      // RCGCI2C bit for I2C0

// I2C0's pins
#define I2C0SCL          (1U << 2)      // PB2
#define I2C0SDA          (1U << 3)      // PB3
#define PCTL_PB23_MASK   (0xFFU << 8)   // PB2's and PB3's fields in GPIOB->PCTL: bits 15:8
#define PCTL_PB23_I2C0   (0x33U << 8)   // 3 selects I2C0SCL and I2C0SDA

// Port F pins
#define SW2              (1U << 0)      // PF0: count down (locked at reset)
#define RED_LED          (1U << 1)      // PF1: on if the LCD doesn't answer
#define SW1              (1U << 4)      // PF4: count up
#define SWITCHES         (SW1 | SW2)
#define GPIO_UNLOCK      0x4C4F434B     // "LOCK" in ASCII: written to GPIOF->LOCK to unlock CR

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


#define DEBOUNCE_MS      20             // How long a button must stay up after a release

volatile uint32_t count;                // The count (Watch window)
volatile uint32_t changed;              // 1 = count changed, main must redraw it

void on_press(uint32_t button);


// ---------------------------------------------------------------- provided: don't change
// The Activity's I2C and LCD functions, unchanged
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
// every millisecond and counts (SysTick_Handler is further down, with the buttons).
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

	LCD_SendNibble(0x03, 0);					// The initialization sequence from the Background:
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

// The Experiment 7 Task's debounce: each button has a ready flag, and a falling edge only
// counts while its button is ready. SysTick makes it ready again once it's been released.
volatile uint32_t ready[2] = { 1, 1 };  // [0] SW1, [1] SW2: 1 = its next falling edge is a press
volatile uint32_t up_ms[2];             // How long each button has been up, while not ready

// Called every 1 ms for each button. Once a pressed button has been up for DEBOUNCE_MS
// in a row, it's ready for its next press.
void check_release(uint32_t sw, uint32_t i)
{
	if (ready[i])                       // Not waiting for this one
		return;
	if ((GPIOF->DATA & sw) == 0)        // Down (or bouncing)
		up_ms[i] = 0;
	else if (++up_ms[i] >= DEBOUNCE_MS) // Up, and quiet for DEBOUNCE_MS
		ready[i] = 1;
}

// Every 1 ms: the clock for millis() and delay_ms(), and the two buttons' releases
void SysTick_Handler(void)
{
	ms++;
	check_release(SW1, 0);
	check_release(SW2, 1);
}

// Every falling edge of SW1 or SW2, bounces included. Only a ready button's edge is a press:
// it calls on_press() once, and the button isn't ready again until it's released.
void GPIOF_Handler(void)
{
	uint32_t edge = GPIOF->MIS & SWITCHES;  // Which button caused the interrupt
	GPIOF->ICR = edge;

	if ((edge & SW1) && ready[0])
	{
		up_ms[0] = 0;
		ready[0] = 0;
		on_press(SW1);
	}
	if ((edge & SW2) && ready[1])
	{
		up_ms[1] = 0;
		ready[1] = 0;
		on_press(SW2);
	}
}
// ---------------------------------------------------------------- end of provided part


// A debounced press of `button` (SW1 or SW2). It runs inside GPIOF_Handler, so it must be
// quick: no LCD functions and no delay_ms() here. Change the count, and tell main.
void on_press(uint32_t button)
{
	// TODO 1: SW1 adds 1 to count. SW2 takes 1 away, but only while count is above 0 // [!code highlight]

	// TODO 2: tell main the count has changed, so it redraws it // [!code highlight]
}

// Show count on line 2, from its first column
void show_count(void)
{
	// TODO 3: clear the old digits with spaces, then print count at the start of line 2 // [!code highlight]
}


int main(void)
{
	// Provided: the red LED and both buttons, then the 1 ms clock, the bus, and the LCD
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);

	GPIOF->LOCK = GPIO_UNLOCK;                      // Unlock PF0 for SW2
	GPIOF->CR  |= SW2;
	GPIOF->LOCK = 0;

	GPIOF->DIR = (GPIOF->DIR | RED_LED) & ~SWITCHES;    // Red LED out, switches in
	GPIOF->PUR |= SWITCHES;                         // Pull-ups: pressed reads 0
	GPIOF->DEN |= RED_LED | SWITCHES;
	GPIOF->IS  &= ~SWITCHES;                        // Both buttons: edge interrupts...
	GPIOF->IBE &= ~SWITCHES;                        // ...one direction...
	GPIOF->IEV &= ~SWITCHES;                        // ...falling: a press
	GPIOF->ICR  = SWITCHES;
	GPIOF->IM  |= SWITCHES;
	NVIC_EnableIRQ(GPIOF_IRQn);

	SysTick_Config(SystemCoreClock / 1000);         // 1 ms clock, after Port F: it reads the buttons
	I2C0_Init();

	if (I2C0_Write(LCD_ADDR, LCD_BL) != 0)          // Backlight on. No ACK: nothing at LCD_ADDR
	{
		GPIOF->DATA |= RED_LED;
		while (1);
	}

	LCD_Init();
	LCD_SetCursor(0, 0);
	LCD_Print("Counter");
	show_count();                                   // The starting 0

	while (1)
	{
		// TODO 4: when the count has changed: clear the flag, then redraw it with show_count() // [!code highlight]
	}
}
