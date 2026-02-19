#ifndef I2C_H
#define I2C_H

#include <stdint.h>


// I2C State Machine
typedef enum i2c_state_enum {
    I2C_IDLE,
    I2C_TX_REG_ADDR,
    I2C_TX_DATA,
    I2C_SWITCH_TO_RX,
    I2C_RX_DATA,
    I2C_COMPLETE,
    I2C_ERROR
} i2c_state_t;


void init_I2C();
uint8_t I2C_transmitByte(uint8_t dev_addr, uint8_t *data);
uint8_t I2C_writeReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

#endif
