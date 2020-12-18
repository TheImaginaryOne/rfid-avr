#include <avr/io.h>
#define F_CPU 16000000L
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

#include "rfid.h"
#include "spi.h"
#include "uart.h"

volatile uint8_t adc_input = 2;
uint8_t bob = 0;

//void pwm_setup() {
//    DDRB |= (1 << PB1); // pin 9
//    // TOP value
//    // Period is 20ms
//    ICR1 = 20000; // not atomic
//
//    // non inverted
//    TCCR1A |= (1 << COM1A1);
//
//    // Phase correct, TOP = ISR1
//    TCCR1A |= (1 << WGM11);
//    TCCR1B |= (1 << WGM13);
//
//    // prescale 8 
//    TCCR1B |= (1 << CS11);
//}
//

int main(void) {
    // SCK, MOSI, SS
    DDRB |= (1 << PB5) | (1 << PB3) | (1 << PB2);
    // MISO
    DDRB &= ~(1 << PB4);
    // pin 2
    DDRD |= (1 << PD2);

    // interrupts ON
    uart_setup();
    spi_setup();
    //sei();

    PORTD |= (1 << PD2);
    _delay_loop_2(60000);
    _delay_loop_2(60000);
    PORTD &= ~(1 << PD2);
    rfid_setup();
    while (1) {
        while (!rfid_check_card_present());
        uint8_t uid[5];
        rfid_select_card(uid);

        uint8_t key_a[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        rfid_authenticate(4, key_a, uid);

        PORTD |= (1 << PD2);
        _delay_loop_2(60000);
        _delay_loop_2(60000);
        PORTD &= ~(1 << PD2);
    }
}
