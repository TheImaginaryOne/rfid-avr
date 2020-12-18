#include "uart.h"
#include <avr/io.h>
#include <stdio.h>

void uart_setup() {
    // 16bit
    UBRR0 = BAUD_RATE_REGISTER;
    // Transmit enable
    UCSR0B |= (1 << TXEN0);
    // 8 bit 1 stop bit
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}
void uart_send(uint8_t len, char str[]) {
    for (int i = 0; i < len; i++) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = (uint8_t) str[i];
    }
}


