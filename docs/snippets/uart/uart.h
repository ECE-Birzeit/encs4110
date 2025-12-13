#ifndef UART_H
#define UART_H

#include "TM4C123.h"  // Or your MCU's main header

#define U0_TX ( 1 << 1 )
#define U0_RX ( 1 << 0 )

#define MAX_STR_LEN 50

void UART0_WriteChar(char c);
void UART0_WriteString(char *str);

char UART0_ReadChar();
void UART0_ReadString(char *buffer, int maxLen);

void UART0_Init();




#endif // UART_H
