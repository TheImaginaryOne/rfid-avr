#ifndef LCD_H_
#define LCD_H_

#include <stdio.h>
#include <avr/io.h>

struct Lcd {
    volatile uint8_t* rs_port;
    uint8_t rs_pin;
    volatile uint8_t* enable_port;
    uint8_t enable_pin;
    volatile uint8_t* data_port;
    uint8_t data_pin_offset; // the first data pin which is set to D4
};

void lcd_write(Lcd* lcd, uint8_t b);
void lcd_write_data(Lcd* lcd, uint8_t data);
void lcd_write_command(Lcd* lcd, uint8_t command);
void lcd_init(Lcd* lcd);
void lcd_write_text(Lcd* lcd, char* buffer, uint8_t len);
void lcd_cursor_to(Lcd* lcd, uint8_t row, uint8_t column);

#endif
