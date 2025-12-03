#include "TM4C123.h"
#include "uart.h"
#include "main.h"
#include <stdio.h>
#include <stdint.h>

volatile uint32_t POS, COUNT, SPEED;
volatile int32_t ANGLE;

int main(void)
{
    char buffer[128];

    // ------------------ ENABLE CLOCKS ------------------
    SYSCTL->RCGCQEI  |= (1<<1);    // QEI1
    SYSCTL->RCGCGPIO |= (1<<2);    // GPIOC

    __asm__("NOP"); __asm__("NOP"); __asm__("NOP");

    // Initialize delay functions
    SysTick_Init();
    
    UART0_Init();

    // ------------------ CONFIGURE PC5, PC6 ------------------
    GPIOC->LOCK = 0x4C4F434B;
    GPIOC->CR  |= (PhA1 | PhB1);
    GPIOC->AFSEL |= (PhA1 | PhB1);
    GPIOC->PCTL &= ~(0xFF << 20);
    GPIOC->PCTL |=  (0x66 << 20);
    GPIOC->DEN |= (PhA1 | PhB1);

    // ------------------ CONFIGURE QEI1 ------------------
    QEI1->CTL = 0;

    // Enable: QEI, Filtration, CAPMODE (both PhA+PhB), velocity capture
    QEI1->CTL =
        (1<<0)  |   // ENABLE
        (1<<3)  |   // CAPMODE
        (1<<5)  |   // VELEN
        (1<<13);    // FILTEN

    QEI1->MAXPOS = CPR - 1;

    // Set velocity timer period
    QEI1->LOAD = LOAD_VAL;


    UART0_WriteString("QEI + UART ready...\r\n");

    // ------------------ MAIN LOOP ------------------
    while (1)
    {
        POS   = QEI1->POS;
        COUNT = QEI1->COUNT;   // current accumulator (during window)
        SPEED = QEI1->SPEED;   // last completed window
        ANGLE = (POS * 360) / CPR;

        // Integer RPM, safe for fast rotation
        uint32_t rpm_int = SPEED * RPM_FACTOR;

        sprintf(buffer,
                "POS=%u  ANGLE=%d deg  SPEED=%u  RPM=%du\r\n",
                POS, ANGLE, SPEED, rpm_int);

        UART0_WriteString(buffer);

        // Wait for next QEI velocity sample
        // (match SAMPLE_HZ)
        delay_ms(1000 / SAMPLE_HZ);
    }
}
