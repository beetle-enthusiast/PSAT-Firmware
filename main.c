
#include <driverlib.h>
#include <i2c.h>

#include <stdint.h>
#include <stdlib.h>


/* Global Variables */
#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(mode)
#elif defined(__IAR_SYSTEMS_ICC__)
__persistent
#endif

uint8_t txBuffer[16] = {0xEE};

/* Function Declarations */
void init_GPIO();
void init_CS();
void init_UART();
void init_I2C();

/**
 * main.c
 */
int main() {
    /* Stop watchdog timer */
    WDT_A_hold(WDT_A_BASE);

    init_GPIO();
    init_CS();
    init_I2C();
    // init_UART();
    __bis_SR_register(GIE); // General interrupt enable

    uint8_t i = 0;    
    while(i < 16) {
        I2C_transmit(0x76, 0x00, txBuffer);
        __delay_cycles(800000UL * 1000);
        i++;
    }
}


void init_GPIO() {
    /* Initialise all GPIO to output low for minimal LPM power consumption */
    GPIO_setAsOutputPin(GPIO_PORT_PA, GPIO_PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PB, GPIO_PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PC, GPIO_PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PD, GPIO_PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PE, GPIO_PIN_ALL16);

    GPIO_setOutputLowOnPin(GPIO_PORT_PA, GPIO_PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PB, GPIO_PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PC, GPIO_PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PD, GPIO_PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PE, GPIO_PIN_ALL16);

    // Configure eUSCIB0 for I2C
    P1SEL0 |= (1<<2)|(1<<3);    // P1.2 -> SDA, P1.3 -> SCL
}


void init_CS() {
    // Configure two FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz before configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_2 ;

    __bis_SR_register(SCG0);                    // disable FLL
    CSCTL3 |= SELREF__REFOCLK;                  // Set REF0CLK as FLL reference source
    CSCTL0 = 0;                                 // clear DCO and MOD registers
    CSCTL1 = DCORSEL_3;                         // Set DCO = 24MHz
    CSCTL2 = FLLD_0 + 243;                      // DCOCLKDIV = 24MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                     // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));   // FLL locked

    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;   // set REF0CLK (~32768Hz) as ACLK source, ACLK = 32768Hz
                                                 // default DCOCLKDIV as MCLK and SMCLK source
}


// Initialise UART (Need to reconfigure clock timings)
void init_UART() {
    // Configure UCA1TXD and UCA1RXD
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    // Configure UART
    // ClockSource = SMCLK = 24MHz, Baudrate = 115200bps
    // http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 13;
    param.firstModReg = 0;
    param.secondModReg = 37;
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

    if(STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A1_BASE, &param)) {
        return;
    }

    EUSCI_A_UART_enable(EUSCI_A1_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A1_BASE,
                                EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE,
                                 EUSCI_A_UART_RECEIVE_INTERRUPT);      // Enable interrupt
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR()
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) PORT2_ISR ()
#else
#error Compiler not supported!
#endif
{
    __bic_SR_register_on_exit(LPM3_bits);
}
