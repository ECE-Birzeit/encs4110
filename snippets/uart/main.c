#include "TM4C123.h"
#include "uart.h"

int main(void)
{
	char buff[MAX_STR_LEN];

	UART0_Init();
	UART0_WriteString("Hello World!\r\n");          // A greeting, then a new line

	while (1)
	{
		UART0_ReadString(buff, MAX_STR_LEN);        // Wait for a line

		UART0_WriteString("Received: ");            // Send it back
		UART0_WriteString(buff);
		UART0_WriteString("\r\n");
	}
}
