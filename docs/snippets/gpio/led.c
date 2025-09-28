
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long *)0x400FE608))

#define GPIO_PORTF_DATA_R (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_DEN_R (*((volatile unsigned long *)0x4002551C))

#define PF3 0x08 // Green LED

static void delay(volatile unsigned long ticks) {
    while (ticks--) { __asm__("nop"); }
}

int main(void)
{

    // Enable the GPIO port that is used for the onboard LED.
    SYSCTL_RCGCGPIO_R = 0x20;

    // Set the direction as output (PF3).
    GPIO_PORTF_DIR_R |= PF3;
    // Enable the GPIO pin for digital function (PF3).
    GPIO_PORTF_DEN_R |= PF3;

    while (1)
    {
        GPIO_PORTF_DATA_R ^= PF3; // Toggle the LED.
        delay(2000000); // Delay for a bit.
    }
} 