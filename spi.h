#ifndef SPI_H_
#define SPI_H_

#include <stdio.h>
#include <stdbool.h>

void spi_setup();

void spi_slave_select(bool on);

uint8_t spi_transmit(uint8_t data);

#endif
