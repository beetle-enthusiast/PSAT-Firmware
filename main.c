
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


#include <msp430.h> 
#include <stdint.h>
#include <i2c.h>

//******************************************************************************
// Pin Config ******************************************************************
//******************************************************************************

#define LED0_OUT    P1OUT
#define LED0_DIR    P1DIR
#define LED0_PIN    BIT0


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

#define REG_ADDR    0x00

#define PACKET_SIZE         12

#define MAX_BUFFER_SIZE     16

/* MasterTypeX are example buffers initialized in the master, they will be
 * sent by the master to the slave.
 * SlaveTypeX are example buffers initialized in the slave, they will be
 * sent by the slave to the master.
 * */

uint8_t tx_buffer[]
uint8_t rx_buffer[PACKET_SIZE] = {0};


/*  Device Initialisation  */

void gpio_init()
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

void cs_init()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    __bis_SR_register(SCG0);        // disable FLL
    CSCTL3 |= SELREF__REFOCLK;      // Set REFO as FLL reference source
    CSCTL0 = 0;                     // clear DCO and MOD registers
    CSCTL1 &= ~(DCORSEL_7);         // Clear DCO frequency select bits first
    CSCTL1 |= DCORSEL_5;            // Set DCO = 16MHz
    CSCTL2 = FLLD_0 + 487;          // DCOCLKDIV = 16MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                        // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));      // FLL locked
}



int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    cs_init();
    gpio_init();
    i2c_init();

    // // Configure BMP390 for pressure measurements in normal mode
    // uint8_t pwr_cfg = 0x31;
    // uint8_t osr = 0x01;

    // i2c_readReg(SLAVE_ADDR, 0x1B, &pwr_cfg, 1);
    // i2c_readReg(SLAVE_ADDR, 0x1C, &osr, 1);

    i2c_readReg(SLAVE_ADDR, REG_ADDR, PACKET_SIZE);
    copyArray(ReceiveBuffer, SlaveType0, PACKET_SIZE);

    __bis_SR_register(LPM0_bits + GIE);
	return 0;
}
