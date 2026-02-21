#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define MAX_BUFFER_SIZE     20


extern uint8_t ReceiveBuffer[MAX_BUFFER_SIZE];
extern uint8_t TransmitBuffer[MAX_BUFFER_SIZE];


//******************************************************************************
// General I2C State Machine ***************************************************
//******************************************************************************

typedef enum I2C_ModeEnum{
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
    SWITCH_TO_TX_MODE,
    TIMEOUT_MODE
} I2C_Mode;


void initI2C();

/* I2C Write and Read Function Declarations */

/* For slave device with dev_addr, writes the data specified in *reg_data
 *
 * dev_addr: The slave device address.
 * reg_addr: The register or command to send to the slave.
 * *reg_data: The buffer to write
 * count: The length of *reg_data
 *  */
I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count);

/* For slave device with dev_addr, read the data specified in slaves reg_addr.
 * The received data is available in ReceiveBuffer
 *
 * dev_addr: The slave device address.
 * reg_addr: The register or command to send to the slave.
 * count: The length of data to read
 *  */
I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count);

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);


#endif
