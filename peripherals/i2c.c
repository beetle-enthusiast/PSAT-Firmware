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
    UCB0CTLW1 = UCASTP_0;   // Software STOP assertion    
    UCB0CTLW0 &= ~UCSWRST;   // eUSCI_B in operational state
    
    UCB0IE |= UCTXIE0 | UCRXIE0 | UCSTPIE | UCNACKIE; // Enable interrupts
}

// Automatic stop bit configured
uint8_t I2C_transmitByte(uint8_t dev_addr, uint8_t *data) {
    while (UCB0STATW & UCBBUSY);
    UCB0CTLW0 |= UCTR;
    UCB0I2CSA = dev_addr;
    UCB0TXBUF = 0xEE;
    while (!UCB0IFG & UCTXIFG0);
    UCB0CTLW0 |= UCTXSTP;
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



#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void)
{
    switch (__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG)) {
        
        case USCI_NONE:
            break;

        // NACK error handling
        case USCI_I2C_UCNACKIFG:
            UCB0CTLW0 |= UCTXSTP;
            i2c_state = I2C_ERROR;
            i2c_done = true;
            break;

        // Byte transmission
        case USCI_I2C_UCTXIFG0:
            switch (i2c_state) {
                
                case I2C_TX_REG_ADDR:
                    UCB0TXBUF = reg_addr;

                    if (rx_len > 0) {
                        i2c_state = I2C_SWITCH_TO_RX;
                        UCB0CTLW0 &= ~UCTR;      // RX mode
                        UCB0CTLW0 |= UCTXSTT;    // Send repeated START
                        i2c_state = I2C_RX_DATA;
                    } else {
                        i2c_state = I2C_TX_DATA;
                    }
                    break;

                case I2C_TX_DATA:
                    if (tx_len > 0) {
                        UCB0TXBUF = *tx_buf++;
                        tx_len--;
                    } else {
                        UCB0CTLW0 |= UCTXSTP;
                        i2c_state = I2C_COMPLETE;
                    }
                    break;
            }
            break;

        // Byte reception
        case USCI_I2C_UCRXIFG0:
            *rx_buf++ = UCB0RXBUF;
            rx_len--;

            if (rx_len == 1) {
                UCB0CTLW0 |= UCTXSTP;
            } else if (rx_len == 0) {
                i2c_state = I2C_COMPLETE;
            }
            break;

        // Release I2C bus
        case USCI_I2C_UCSTPIFG:
            i2c_done = true;
            i2c_state = I2C_IDLE;
            break;

        default:
            break;
    }

    // Handle repeated START for read
    if (i2c_state == I2C_SWITCH_TO_RX) {
        UCB0CTLW0 &= ~UCTR;      // RX mode
        UCB0CTLW0 |= UCTXSTT;    // Send repeated START
        i2c_state = I2C_RX_DATA;
    }
}
