#ifndef UART_H
#define UART_H

#include "TM4C123.h"

#define U0_RX        (1U << 0)          // PA0
#define U0_TX        (1U << 1)          // PA1

#define MAX_STR_LEN  50

void UART0_Init(void);
void UART0_WriteChar(char c);
void UART0_WriteString(char *str);
char UART0_ReadChar(void);
void UART0_ReadString(char *buffer, int maxLen);

#endif // UART_H
