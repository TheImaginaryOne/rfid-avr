#include <avr/io.h>
#define F_CPU 16000000L
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

#include "rfid.h"
#include "spi.h"
#include "uart.h"
#include "lcd.h"

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
    // Enable PD0 - PD7
    DDRD |= 0b11111111;

    // interrupts ON
    uart_setup();
    spi_setup();
//    //sei();
//
//    // PORTD |= (1 << PD2);
//    // _delay_loop_2(60000);
//    // _delay_loop_2(60000);
//    // PORTD &= ~(1 << PD2);
    rfid_setup();

    Lcd lcd;
    lcd.rs_pin = PD6;
    lcd.enable_pin = PD7;
    lcd.data_pin_offset = PD2;
    lcd.rs_port = &PORTD;
    lcd.enable_port = &PORTD;
    lcd.data_port = &PORTD;
    lcd_init(&lcd);
    
    lcd_write_command(&lcd, 0x06);
    char input[] = "NotAT HOP Card";
    lcd_write_text(&lcd, input, 14);


    while (1) {
        while (!rfid_check_card_present());
        uint8_t uid[5];
        rfid_select_card(uid);

        uint8_t key_a[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        if (rfid_authenticate(4, key_a, uid)) {
            uint8_t resp[16];
            bool x = rfid_read(4, resp);
            if (x) {
                //uart_send(3, "b\r\n");
                char buf[2];
                for (int i = 0; i< 16; i++) {
                    sprintf(buf, "%02X", resp[i]);
                    uart_send(2, buf);
                }
                uart_send(2, "\r\n");

                uint8_t amount = resp[15];
                amount += 1;

                char amount_text[] = "Amount ";
                lcd_cursor_to(&lcd, 1, 0);
                lcd_write_text(&lcd, amount_text, 7);
                char text[4];
                sprintf(text, "%03u", amount);
                lcd_write_text(&lcd, text, 3);

                uint8_t input[16] = {};
                input[15] = amount;
                bool x = rfid_write(4, input);

                rfid_stop_crypto();
                rfid_halt();
            }
        }

        // PORTD |= (1 << PD2);
        // _delay_loop_2(60000);
        // _delay_loop_2(60000);
        // PORTD &= ~(1 << PD2);
    }
}
