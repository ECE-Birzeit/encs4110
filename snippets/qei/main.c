// #region full
// #region defines
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOC       (1U << 2)      // RCGCGPIO bit for Port C (encoder)
#define RCGC_GPIOF       (1U << 5)      // RCGCGPIO bit for Port F (LEDs)
#define RCGC_QEI1        (1U << 1)      // RCGCQEI bit for QEI1

// The encoder's pins: PC5 = PhA1 (CLK), PC6 = PhB1 (DT)
#define QEI1_PHA         (1U << 5)
#define QEI1_PHB         (1U << 6)
#define PCTL_PC56_MASK   (0xFFU << 20)  // PC5's and PC6's fields in GPIOC->PCTL: bits 27:20
#define PCTL_PC56_QEI    (0x66U << 20)  // 6 selects PhA1 and PhB1

// LEDs
#define RED_LED          (1U << 1)      // PF1
#define GREEN_LED        (1U << 3)      // PF3

// QEI1 register bits
#define CTL_ENABLE       (1U << 0)      // CTL: start the module (can't be cleared again)
#define CTL_CAPMODE      (1U << 3)      // CTL: count the edges of both PhA and PhB
#define CTL_VELEN        (1U << 5)      // CTL: velocity capture on
#define CTL_FILTEN       (1U << 13)     // CTL: input filter on
#define CTL_FILTCNT_MAX  (0xFU << 16)   // CTL: filter samples every 17 clocks
#define STAT_DIRECTION   (1U << 1)      // STAT: 1 = last moved in reverse
#define INT_TIMER        (1U << 1)      // INTEN, RIS, ISC: velocity window ended

// The encoder and the speed measurement
#define PPR              20             // Pulses per revolution: 20 on the HW-040 (one per click)
#define EDGES            (PPR * 4)      // Counts per turn with CAPMODE: 80
#define WINDOW_HZ        10             // Speed windows per second: 100 ms each

volatile uint32_t position;             // QEI1->POS: 0 to EDGES - 1
volatile uint32_t clicks;               // 0 to PPR - 1
volatile uint32_t angle;                // Degrees: 0 to 355
volatile int32_t  rpm;                  // Speed, negative in reverse
// #endregion defines


// #region handler
// Every 100 ms: the QEI has copied the window's edge count into SPEED
void QEI1_Handler(void)
{
	QEI1->ISC = INT_TIMER;						// Clear the interrupt (write 1)

	rpm = QEI1->SPEED * WINDOW_HZ * 60 / EDGES;	// Edges per second, times 60, per 80 edges a turn
	if (QEI1->STAT & STAT_DIRECTION)			// SPEED has no sign: STAT gives it
		rpm = -rpm;
}
// #endregion handler


// #region main
int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOC | RCGC_GPIOF;	// Clocks to Port C and Port F
	SYSCTL->RCGCQEI  |= RCGC_QEI1;				// Clock to QEI1
	while ((SYSCTL->PRGPIO & (RCGC_GPIOC | RCGC_GPIOF)) != (RCGC_GPIOC | RCGC_GPIOF));
	while ((SYSCTL->PRQEI  & RCGC_QEI1) == 0);

	// Port C: change only bits 5 and 6. PC0 to PC3 are the debugger's JTAG pins.
	GPIOC->AFSEL |= QEI1_PHA | QEI1_PHB;								// Driven by a peripheral...
	GPIOC->PCTL   = (GPIOC->PCTL & ~PCTL_PC56_MASK) | PCTL_PC56_QEI;	// ...QEI1
	GPIOC->PUR   |= QEI1_PHA | QEI1_PHB;								// Pull-ups, for encoders without them
	GPIOC->DEN   |= QEI1_PHA | QEI1_PHB;

	GPIOF->DIR |= RED_LED | GREEN_LED;
	GPIOF->DEN |= RED_LED | GREEN_LED;

	QEI1->CTL    = CTL_FILTCNT_MAX | CTL_FILTEN		// Filter the bouncy contacts
	             | CTL_CAPMODE						// 4 counts per click
	             | CTL_VELEN;						// Speed measurement on
	QEI1->MAXPOS = EDGES - 1;						// One turn: 0 to 79, then back to 0
	QEI1->CTL   |= CTL_ENABLE;						// Start counting
	QEI1->POS    = 0;								// Starting position (only after ENABLE)

	QEI1->LOAD   = SystemCoreClock / WINDOW_HZ - 1;	// Window: 5,000,000 clocks = 100 ms
	QEI1->INTEN  = INT_TIMER;						// Interrupt at the end of every window
	NVIC_EnableIRQ(QEI1_IRQn);						// QEI1 is interrupt 38

	while (1)
	{
		position = QEI1->POS;					// The QEI counts by itself: just read it
		clicks   = position / 4;
		angle    = position * 360 / EDGES;

		if (QEI1->STAT & STAT_DIRECTION)		// Last moved in reverse
			GPIOF->DATA = (GPIOF->DATA & ~GREEN_LED) | RED_LED;
		else									// Last moved forward
			GPIOF->DATA = (GPIOF->DATA & ~RED_LED) | GREEN_LED;
	}
}
// #endregion main
// #endregion full
