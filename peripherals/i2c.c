#include "i2c.h"
#include "def.h"
#include <driverlib.h>
#include <stdint.h>


volatile i2c_state_t i2c_state = I2C_IDLE;

volatile uint8_t *tx_buf;
volatile uint8_t tx_len;

volatile uint8_t *rx_buf;
volatile uint8_t rx_len;

volatile uint8_t slave_addr;
volatile uint8_t reg_addr;

volatile bool i2c_done = false;


void init_I2C() {
    UCB0CTLW0 |= UCSWRST;    // Put eUSCI_B in reset state
    UCB0CTLW0 |= UCMST + UCMODE_3;  // I2C master mode
    UCB0BRW = 20;   // Baud rate = SMCLK / 20 (400kHz Fast Mode)
    
    // REMOVE ONCE REG ADDRESSES HAVE BEEN WRITTEN
    // -------------------------------------------
    UCB0CTLW1 = UCASTP_2;   // Automatic STOP assertion
    UCB0TBCNT = 1;
    // -------------------------------------------
    
    UCB0CTLW0 &= ~UCSWRST;   // eUSCI_B in operational state
    // UCB0IE |= UCTXIE0 | UCRXIE0 | UCSTPIE | UCNACKIE; // Enable interrupts
}

// void I2C_switchDevice(uint8_t dev_addr) {
//     UCB0CTLW0 |= UCSWRST;    // Put eUSCI_B in reset state
//     UCB0I2CSA = dev_addr;         // Address slave
//     UCB0CTLW0 &= ~UCSWRST;   // eUSCI_B in operational state
// }

// Automatic stop bit configured
uint8_t I2C_transmit(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data) {
    while (UCB0STATW & UCBBUSY);
    UCB0CTLW0 |= UCTR;
    UCB0TXBUF = 0xEE;
    while (!UCB0IFG & UCTXIFG0);
    return 0;
}

// Manual stop bit configured
uint8_t I2C_writeReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint8_t len) {
    if (i2c_state != I2C_IDLE) {
        return 1;   // I2C bus busy
    }
    slave_addr = dev_addr;
    reg_addr = reg;

    tx_buf = data;
    tx_len = len;

    i2c_done = false;
    i2c_state = I2C_TX_REG_ADDR;

    // Address slave as transmitter
    UCB0I2CSA = slave_addr;
    UCB0CTLW0 |= UCTR | UCTXSTT;

    return 0;   // I2C write initialised successfully
}
