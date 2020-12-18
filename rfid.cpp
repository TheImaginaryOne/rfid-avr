#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>

#include "rfid.h"
#include "spi.h"
#include "uart.h"


// "address" of a register
void rfid_write_register(uint8_t address, uint8_t data) {
    spi_slave_select(true);
    // 0 <address, 6b> 0
    spi_transmit((address & 0x3f) << 1);
    spi_transmit(data);
    spi_slave_select(false);
}

// "address" of a register
void rfid_write_register_multiple(uint8_t address, uint8_t* data, uint8_t len) {
    spi_slave_select(true);
    // 0 <address, 6b> 0
    spi_transmit((address & 0x3f) << 1);
    for (int i = 0; i < len; i++) {
        spi_transmit(data[i]);
    }
    spi_slave_select(false);
}

uint8_t rfid_read_register(uint8_t address) {
    spi_slave_select(true);
    // 1 <address, 6b> 0
    spi_transmit(((address & 0x3f) << 1) | 0x80);
    // 0x00 is a dummy transmission
    uint8_t result = spi_transmit(0x00);
    spi_slave_select(false);
    return result;
}

void rfid_setup() {
    rfid_write_register(TX_MODE_REG, 0x00);
    rfid_write_register(RX_MODE_REG, 0x00);
    rfid_write_register(TX_ASK_REG, 0x40);

    rfid_write_register(TMODE_REG, 0x80);
    rfid_write_register(TPRESCALER_REG, 0xA9);
    rfid_write_register(TRELOAD_HIGH_REG, 0x03);
    rfid_write_register(TRELOAD_LOW_REG, 0xE8);
}

bool rfid_check_card_present() {
    uint8_t command[] = {WUPA};
    bool result = rfid_transceive(0x80 | 0x07, command, 1, nullptr, 0, false);

    return result;
}

bool rfid_transceive(
        uint8_t bit_framing,
        uint8_t* buffer,
        uint8_t len,
        uint8_t* response,
        uint8_t response_len,
        bool do_check
        ) {
    return rfid_send(TRANSCEIVE, bit_framing, buffer, len, response, response_len, do_check);
}
bool rfid_send(
        uint8_t command,
        uint8_t bit_framing,
        uint8_t* buffer,
        uint8_t len,
        uint8_t* response,
        uint8_t response_len,
        bool do_check
        ) {
    rfid_write_register(COMMAND_REG, 0x00);
    rfid_write_register(TX_CONTROL_REG, 0x83);
        
    rfid_write_register(COM_IRQ_REG, 0x7f); // clear

    rfid_write_register(FIFO_LEVEL_REG, 0x80); // clear
    
    rfid_write_register_multiple(FIFO_DATA_REG, buffer, len);
    
    // start transceive
    rfid_write_register(COMMAND_REG, command);
    rfid_write_register(BIT_FRAMING_REG, bit_framing);
    uint8_t irq = 0;

    bool success = false;
    for (int i = 0; i < 500; i++) {
        // ComIrq
        irq = rfid_read_register(COM_IRQ_REG);
        // RxIrq -- receiving complete
        // and
        // IdleIrq
        if ((irq & (1 << 4)) | (irq & (1 << 5))) {
            success = true;
            break;
        }
    }
    if (!success) {
        return false;
    }
    uint8_t return_len = rfid_read_register(FIFO_LEVEL_REG);

    for (int i = 0; i < response_len; i++) {
        uint8_t out = rfid_read_register(FIFO_DATA_REG);
        response[i] = out;
    }
    if (do_check) {
        if ((return_len == 1) && (response[0] != 0x0a)) {
            return false;
        }
    }
    return true;
}

bool rfid_calculate_crc(uint8_t* buffer, uint8_t len, uint8_t* out) {
    // Clear command
    rfid_write_register(COMMAND_REG, 0);
    // CRC-16 initial value is 0x6363 (iso 14443-3)
    rfid_write_register(MODE_REG, 0x3d);
    // Clear interrupt bits
    rfid_write_register(STATUS1_REG, 0x00);
    // Reset FIFO
    rfid_write_register(FIFO_LEVEL_REG, 0x80);
    rfid_write_register(COMMAND_REG, CALCULATE_CRC);
    // Must send data after turning on CRC calculator
    rfid_write_register_multiple(FIFO_DATA_REG, buffer, len);

    for (int i = 0; i < 500; i++) {
        uint8_t result = rfid_read_register(STATUS1_REG);
        if (result & (1 << 5)) {
            // Stop the CRC calculation
            rfid_write_register(COMMAND_REG, 0);
            out[0] = rfid_read_register(CRC_RESULT_LOW_REG);
            out[1] = rfid_read_register(CRC_RESULT_HIGH_REG);
            return true;
        }
    }
    rfid_write_register(COMMAND_REG, 0);
    return false;
}

bool rfid_select_card(uint8_t* uid) {
    rfid_write_register(COLL_REG, 0x80);

    // An anticollision command, with 32 valid bits.
    // Ask the PICC to send the whole uid
    uint8_t commands[] = {ANTI_COLLISION, 32};

    // Buffer to be used for subsequent requests
    uint8_t request_data[9];

    bool x = rfid_transceive(0x80, commands, 2, &request_data[2], 5, false);
    if (!x) {
        return false;
    }
    // ComIrq
    uint8_t coll = rfid_read_register(COLL_REG);
    
    if (0 != (coll & (1 << 5))) {
        // Set up second select command
        request_data[0] = ANTI_COLLISION;
        request_data[1] = 7 * 16; // will transmit complete uid

        // no anticollision looop for now
        // Append crc
        bool success = rfid_calculate_crc(&request_data[0], 7, &request_data[7]);
        if (!success) {
            return false;
        }

        // Send request (select)
        uint8_t response;
        bool x = rfid_transceive(0x80, request_data, 9, &response, 1, false);

        if (x) {
            //// insert
            // Check if it is a SAK. NOTE! This is specific to Mifare.
            if (response & (1 << 3)) {
                // copy
                for (int i = 0; i < 4; i++) {
                    uid[i] = request_data[i + 2];
                }
                return true;
            }
        }
    }
    return false;
}

bool rfid_authenticate(uint8_t block_address, uint8_t* key, uint8_t* uid) {
    rfid_write_register(STATUS2_REG, 0);
    // TODO check
    // clear
    uint8_t request[12];

    // command
    request[0] = AUTH_KEY_A;
    request[1] = block_address;


    for (int i = 0; i < 6; i++) {
        request[2 + i] = key[i];
    }

    for (int i = 0; i < 4; i++) {
        // last 4 bits of uid
        request[8 + i] = uid[i];
    }

    uint8_t response[3];
    if (!rfid_send(MF_AUTHENT, 0x80, request, 12, response, 3, false)) {
        rfid_write_register(STATUS2_REG, 0);
        return false;
    }

    // Check if Crypto1 flag is on, meaning authentication success
    if (STATUS2_REG & (1 << 3)) {
        return true;
    }
    rfid_write_register(STATUS2_REG, 0);
    return false;
}

bool rfid_read(uint8_t block_address, uint8_t* output) {
    uint8_t request[4];
    request[0] = READ_DATA;
    request[1] = block_address;

    // append crc
    rfid_calculate_crc(request, 2, &request[2]);

    // TODO check crc of response
    return rfid_transceive(0x80, request, 4, output, 18, true);
}
