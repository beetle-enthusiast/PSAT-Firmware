//******************************************************************************
//   MSP430FR235x Demo - eUSCI_B0, I2C Master multiple byte TX/RX
//
//   Description: I2C master communicates to I2C slave sending and receiving
//   3 different messages of different length. I2C master will enter LPM0 mode
//   while waiting for the messages to be sent/receiving using I2C interrupt.
//   ACLK = NA, MCLK = SMCLK = DCO 16MHz.
//
//                                     /|\ /|\
//                   MSP430FR2355      4.7k |
//                 -----------------    |  4.7k
//            /|\ |             P1.3|---+---|-- I2C Clock (UCB0SCL)
//             |  |                 |       |
//             ---|RST          P1.2|-------+-- I2C Data (UCB0SDA)
//                |                 |
//                |                 |
//                |                 |
//                |                 |
//                |                 |
//                |                 |
//
//   Xiaodong Li
//   Texas Instruments Inc.
//   May 2020
//   Built with CCS V9.2
//******************************************************************************

#include <msp430.h> 
#include <stdint.h>
#include <stdbool.h>
#include <i2c.h>

//******************************************************************************
// Pin Config ******************************************************************
//******************************************************************************

#define LED0_OUT    P1OUT
#define LED0_DIR    P1DIR
#define LED0_PIN    BIT0

#define LED1_OUT    P6OUT
#define LED1_DIR    P6DIR
#define LED1_PIN    BIT6

//******************************************************************************
// Example Commands ************************************************************
//******************************************************************************

#define SLAVE_ADDR  0x76

/* CMD_TYPE_X_SLAVE are example commands the master sends to the slave.
 * The slave will send example SlaveTypeX buffers in response.
 *
 * CMD_TYPE_X_MASTER are example commands the master sends to the slave.
 * The slave will initialize itself to receive MasterTypeX example buffers.
 * */

#define CMD_TYPE_0_SLAVE      0x00

#define CMD_TYPE_0_MASTER      3

#define TYPE_0_LENGTH   10

#define MAX_BUFFER_SIZE     20

/* MasterTypeX are example buffers initialized in the master, they will be
 * sent by the master to the slave.
 * SlaveTypeX are example buffers initialized in the slave, they will be
 * sent by the slave to the master.
 * */


uint8_t MasterType0 [TYPE_0_LENGTH] = { 11};

uint8_t SlaveType0 [TYPE_0_LENGTH] = {0};


//******************************************************************************
// Device Initialization *******************************************************
//******************************************************************************


void initGPIO()
{
    //LEDs
    LED0_OUT &= ~LED0_PIN;
    LED0_DIR |= LED0_PIN;

    LED1_OUT &= ~LED1_PIN;
    LED1_DIR |= LED1_PIN;

    // I2C pins
    P1SEL0 |= BIT2 | BIT3;
    P1SEL1 &= ~(BIT2 | BIT3);

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

void initClockTo16MHz()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    __bis_SR_register(SCG0);                           // disable FLL
    CSCTL3 |= SELREF__REFOCLK;                         // Set REFO as FLL reference source
    CSCTL0 = 0;                                        // clear DCO and MOD registers
    CSCTL1 &= ~(DCORSEL_7);                            // Clear DCO frequency select bits first
    CSCTL1 |= DCORSEL_5;                               // Set DCO = 16MHz
    CSCTL2 = FLLD_0 + 487;                             // DCOCLKDIV = 16MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                           // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));         // FLL locked
}


//******************************************************************************
// Main ************************************************************************
// Send and receive three messages containing the example commands *************
//******************************************************************************

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    initClockTo16MHz();
    initGPIO();
    initI2C();

    // // Configure BMP390 for pressure measurements in normal mode
    // uint8_t pwr_cfg = 0x31;
    // uint8_t osr = 0x01;

    // I2C_Master_WriteReg(SLAVE_ADDR, 0x1B, &pwr_cfg, 1);
    // I2C_Master_WriteReg(SLAVE_ADDR, 0x1C, &osr, 1);

    I2C_Master_ReadReg(SLAVE_ADDR, CMD_TYPE_0_SLAVE, TYPE_0_LENGTH);
    CopyArray(ReceiveBuffer, SlaveType0, TYPE_0_LENGTH);

    // __bis_SR_register(LPM0_bits + GIE);
	return 0;
}
