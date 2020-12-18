#ifndef RFID_H_
#define RFID_H_

#include <stdio.h>
#include <stdbool.h>

enum Register: uint8_t {
    COMMAND_REG = 0x01,
    FIFO_DATA_REG = 0x09,
    FIFO_LEVEL_REG = 0x0a,
    BIT_FRAMING_REG = 0x0d,
    COM_IRQ_REG = 0x04,
    ERROR_REG = 0x06,
    TX_MODE_REG = 0x12,
    RX_MODE_REG = 0x13,
    TX_CONTROL_REG = 0x14,
    TX_ASK_REG = 0x15,
    COLL_REG = 0x0e, // anti-collision
    STATUS1_REG = 0x07, // contains CRC done bit
    STATUS2_REG = 0x08,
    CRC_RESULT_HIGH_REG = 0x21,
    CRC_RESULT_LOW_REG = 0x22,
    MODE_REG = 0x11,
};

enum Command: uint8_t {
    TRANSCEIVE = 0x0c,
    CALCULATE_CRC = 0x03,
    MF_AUTHENT = 0x0e,
};

enum PiccCommand: uint8_t {
    REQA = 0x26,
    WUPA = 0x52,
    ANTI_COLLISION = 0x93, // send anti-collision select
    AUTH_KEY_A = 0x60,
    READ_DATA = 0x30,
};


void rfid_setup(); 

uint8_t rfid_read_register(uint8_t address);

void rfid_write_register(uint8_t address, uint8_t data);

bool rfid_check_card_present();

bool rfid_select_card(uint8_t* uid);

bool rfid_calculate_crc(uint8_t* buffer, uint8_t len, uint8_t* out);

bool rfid_transceive(uint8_t bit_framing, uint8_t* buffer, uint8_t len, uint8_t* response, uint8_t response_len);

bool rfid_authenticate(uint8_t block_address, uint8_t* key, uint8_t* uid);

bool rfid_read(uint8_t block_address, uint8_t* output);

#endif
