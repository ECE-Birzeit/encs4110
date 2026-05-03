---
outline: deep
lastUpdated: true
prev:
  text: "9. UART Serial Communication - Theory"
  link: /experiments/9-theory
next: false
---

# Universal Asynchronous Receiver-Transmitter (UART)

<Badge type="tip" text="TM4C123GH6PM" />



## Examples

::: code-group
<<< @/snippets/uart/main.c
<<< @/snippets/uart/uart.c
<<< @/snippets/uart/uart.h
:::


The above code demonstrates a simple UART communication setup. The `UART0_Init` function initializes the UART0 module, while `UART0_WriteChar` and `UART0_ReadChar` functions handle data transmission and reception, respectively. To be able to communicate with the microcontroller, you can use a terminal program on your PC [PuTTY](https://www.putty.org/) to send and receive data over the serial port. Make sure to set the baud rate and other parameters to match the configuration in your code. The following image shows how to set up the serial connection in PuTTY:

![alt text](/putty.png)

You should replace the `COM` port with the one assigned to your TM4C123G LaunchPad. You can find this in the Device Manager under Ports (COM & LPT). The baud rate should match the one set in your code (e.g., 115200).

