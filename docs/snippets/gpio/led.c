
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long *)0x400FE608))

#define GPIO_PORTF_DATA_R (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_DEN_R (*((volatile unsigned long *)0x4002551C))

#define DELAY 200000

int main(void)
{
    volatile unsigned long ulLoop;

    // Enable the GPIO port that is used for the onboard LED.

    SYSCTL_RCGCGPIO_R = 0x20;

    // Do a dummy read to insert a few cycles after enabling the peripheral.

    ulLoop = SYSCTL_RCGCGPIO_R;

    //_ Enable the GPIO pin for the LED (PF3). Set the direction as output and enable the GPIO pin for digital //function. _/

    GPIO_PORTF_DIR_R = 0x08;
    GPIO_PORTF_DEN_R = 0x08;

    // Loop for ever.

    while (1)
    {
        // Toggle  the LED.

        GPIO_PORTF_DATA_R ^= 0x08; // ^ means XOR in c

        // Delay for a bit.

        for (ulLoop = 0; ulLoop < DELAY; ulLoop++)
        {

            for (ulLoop = 0; ulLoop < DELAY; ulLoop++)
            {
            }
        }
    }
} 