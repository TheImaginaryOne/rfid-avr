#ifndef UART_H_
#define UART_H_

#include <stdio.h>

// uart
#define F_CPU 16000000L
#define BAUD 9600
const uint16_t BAUD_RATE_REGISTER = (F_CPU / 16 / BAUD) - 1;

void uart_setup() ;
void uart_send(uint8_t len, const char* str);
void uart_send_hex(uint8_t* in, uint8_t len);
#endif
