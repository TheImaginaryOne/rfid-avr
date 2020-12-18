#include <stdio.h>
#include <avr/io.h>

#include "spi.h"

void spi_setup() {
    uint8_t spcr = 0;
    // MSB
    spcr &= ~(1 << DORD);
    // master
    spcr |= (1 << MSTR);
    // prescale 16
    spcr |= (1 << SPR0);
    // enable
    spcr |= (1 << SPE);
    // IMPORTANT to set all bits at once
    SPCR = spcr;
}

void spi_slave_select(bool on) {
    if (on) {
        PORTB &= ~(1 << PB2);
    } else {
        PORTB |= (1 << PB2);
    }
}

uint8_t spi_transmit(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF))) {
    }
    SPSR &= ~(1 << SPIF);
    return SPDR;
}

