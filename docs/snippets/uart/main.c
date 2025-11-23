#include <stdio.h>
#include "TM4C123.h"
#include "uart.h"

int main(void) {
    UART0_Init();
    UART0_WriteString("Hello World!\r\n");  // Send greeting with newline

    while (1) {
		char buff[16];
		UART0_ReadString(buff, 16);
		
		UART0_WriteString("Received: ");
		UART0_WriteString(buff);
		UART0_WriteString("\r\n");
    }
}
