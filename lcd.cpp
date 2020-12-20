#include "lcd.h"
#include <avr/io.h>
#define F_CPU 16000000L
#include <util/delay.h>
#include "uart.h"

void lcd_write(Lcd* lcd, uint8_t data) {
    uint8_t upper = data >> 4;
    uint8_t lower = data & 0x0f;
    char buf[2] = {};

    // Write upper
    *lcd->enable_port |= (1 << lcd->enable_pin);
    *lcd->data_port |= (upper << lcd->data_pin_offset);

    _delay_loop_2(80);
    // reset
    *lcd->enable_port &= ~(1 << lcd->enable_pin);
    *lcd->data_port &= ~(0x0f << lcd->data_pin_offset);

    _delay_loop_2(10);
    // Write lower
    *lcd->enable_port |= (1 << lcd->enable_pin);
    *lcd->data_port |= (lower << lcd->data_pin_offset);

    _delay_loop_2(80);
    // reset
    *lcd->enable_port &= ~(1 << lcd->enable_pin);
    *lcd->data_port &= ~(0x0f << lcd->data_pin_offset);

    _delay_loop_2(10);
}

void lcd_write_data(Lcd* lcd, uint8_t data) {
    *lcd->rs_port |= (1 << lcd->rs_pin); // data mode
    lcd_write(lcd, data);
}

void lcd_write_command(Lcd* lcd, uint8_t command) {
    *lcd->rs_port &= ~(1 << lcd->rs_pin); // instruction mode
    lcd_write(lcd, command);
}

void lcd_init(Lcd* lcd) {
    _delay_loop_2(30000);
    lcd_write_command(lcd, 0x2f); // 4 bit mode
    // display ON, cursor OFF
    lcd_write_command(lcd, 0x0c); // 00001100
    lcd_write_command(lcd, 0x01); // clear
    _delay_loop_2(65000);
}

void lcd_write_text(Lcd* lcd, char* input, uint8_t len) {
    for (int i = 0; i < len; i++) {
        lcd_write_command(lcd, 0x06);
        lcd_write_data(lcd, input[i]);
    }
}

void lcd_cursor_to(Lcd* lcd, uint8_t row, uint8_t column) {
    lcd_write_command(lcd, 0x80 + row * 0x40 + column);
}
