#include "trafficlight.h"
#include "uart.h"

volatile uint32_t global_ms   = 0;
volatile uint8_t  ped_request = 0;

phase_t  phase            = PHASE_RED;
uint32_t phase_started_ms = 0;

void TrafficLight_GPIO_Init(void)
{
    // TODO: enable the clock for GPIOF
    // TODO: PF1-3 as outputs -- DIR, DEN
    // TODO: PF4 as an input -- DIR cleared, DEN, PUR (pull-up)
}

void SysTick_Handler(void)
{
    // TODO: increment global_ms
}

void TrafficLight_Run(void)
{
    // TODO -- Step 3: drive the normal RED -> GREEN -> YELLOW -> RED cycle,
    //         each phase lasting its *_SECONDS duration, using global_ms
    //         and phase_started_ms -- update the RGB LED to match `phase`
    //         on every transition, and print the new phase + remaining
    //         seconds over UART about once a second
    // TODO -- Step 4: when leaving RED, insert a WALK phase instead of
    //         GREEN if ped_request is set -- see the assignment page for
    //         exactly when a request should and shouldn't be honored
}

void GPIOF_ButtonInterrupt_Init(void)
{
    // TODO -- Step 4: configure PF4 for a falling-edge interrupt + NVIC (GPIOF = IRQ 30)
}

void GPIOF_Handler(void)
{
    // TODO -- Step 4: clear the interrupt, set ped_request = 1
}
