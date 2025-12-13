#include <stdio.h>
#include "TM4C123.h"
#include "uart.h"

int main(void) {
    UART0_Init();
    UART0_WriteString("Hello World!\r\n");  // Send greeting with newline

    while (1) {
		char buff[MAX_STR_LEN];
		UART0_ReadString(buff, MAX_STR_LEN);
		
		UART0_WriteString("Received: ");
		UART0_WriteString(buff);
		UART0_WriteString("\r\n");
    }
}
