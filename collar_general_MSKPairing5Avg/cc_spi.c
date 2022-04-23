// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain
// Combined and ported MSP430 Interface to CC1100/2500 Code Library by Jonathan Chen, Feb 23, 2021

#include <cc_spi.h>
#include "stdarg.h"

static uint16_t baseAddress;
static uint8_t cs_port;
static uint16_t cs_pin;

/**
 * Configures and enables SPI for 8bit mode, 4wire mode, with a prescaler based on SMCLK on eUSCIA_0
 * Functions are written as if SPI are 3-wire, but the CSn pin is manually controlled by functions in CC2500.h
 *
 * Params are the same as for EUSCI_A_SPI_initMasterParam
 * Does not configure pins. Use GPIO_setAsPeripheralModuleFunction driver functions.
 * Also takes cs_port, cs_pin for the chip-select pin for 4-wire mode
 */
void spi_setup(uint16_t base_Address, uint8_t selectClockSource, uint32_t clockSourceFrequency,
               uint32_t desiredSpiClock, uint16_t msbFirst, uint16_t clockPhase, uint16_t clockPolarity, uint16_t spiMode, uint8_t csport, uint16_t cspin) {
	// reset and enable spi
    baseAddress = base_Address;
    cs_port = csport;
    cs_pin = cspin;
	if(baseAddress == EUSCI_A0_BASE) {
	    EUSCI_A_SPI_initMasterParam param = {0};
        param.selectClockSource = selectClockSource;
        param.clockSourceFrequency = clockSourceFrequency;
        param.desiredSpiClock = desiredSpiClock;
        param.msbFirst = msbFirst;
        param.clockPhase = clockPhase;
        param.clockPolarity = clockPolarity;
        param.spiMode = spiMode;
        EUSCI_A_SPI_initMaster(EUSCI_A0_BASE, &param);
        EUSCI_A_SPI_enable(EUSCI_A0_BASE);  //Enable SPI module
        //Clear receive interrupt flag
        EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE,
              EUSCI_A_SPI_RECEIVE_INTERRUPT
              );
	} else {
	    // Not defined for other SPI ports
	}
}

/**
 * Write to a configuration register on the CC2500.
 * NOT used for multi-byte registers or command strobes.
 *
 * @param reg           Register to read or write
 * @param val           Value to write
 * @returns             Status byte for writes, or value for reads
 */
uint8_t spi_write_register(uint8_t reg, uint8_t val) {

    uint8_t status = 0;

    GPIO_setOutputLowOnPin(cs_port, cs_pin);                // Enable CS
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                   EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;       // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, reg);             // send address
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                   EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;       // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, val);             // send data
    while(EUSCI_A_SPI_isBusy(baseAddress));              // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(baseAddress);          // read RX buffer
    GPIO_setOutputHighOnPin(cs_port, cs_pin);               // Disable CS

    return status;

}

/**
 * Writes values to multiple configuration registers, the first register being
 * at address "addr".  First data byte is at "buffer", and both addr and
 * buffer are incremented sequentially (within the CCxxxx and MSP430,
 * respectively) until "count" writes have been performed.
 *
 * @param reg           Starting register to write
 * @param buffer        Pointer to byte buffer
 * @param count         Number of bytes to write
 */
void spi_write_burst_register(uint8_t reg, uint8_t *buffer, uint8_t count)
{
    unsigned int i;

    GPIO_setOutputLowOnPin(cs_port, cs_pin);                    // Enable CS
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                 EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;             // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, reg | WRITE_BURST);   // Send address
    for (i = 0; i < count; i++) {

        while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                     EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;         // wait for empty TX buffer
        EUSCI_A_SPI_transmitData(baseAddress, buffer[i]);       // Send data

    }
    while(EUSCI_A_SPI_isBusy(baseAddress));                     // Wait until SPI is done transmitting
    GPIO_setOutputHighOnPin(cs_port, cs_pin);                   // Disable CS


}

/**
 * Read from a single configuration register on the CC2500
 *
 * @param reg           Register to read
 * @returns             Status byte for writes, or value for reads
 */
uint8_t spi_read_register(uint8_t reg)
{
    uint8_t status = 0;

    GPIO_setOutputLowOnPin(cs_port, cs_pin);                    // Enable CS
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                   EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;           // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, reg | READ_BYTE);     // send address with read
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                   EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;           // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, 0);                   // Dummy write so we can read data
    while(EUSCI_A_SPI_isBusy(baseAddress));                     // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(baseAddress);              // read RX buffer
    GPIO_setOutputHighOnPin(cs_port, cs_pin);                   // Disable CS

    return status;
}

/**
 * Reads multiple configuration registers, the first register being at address
 * "reg".  Values read are deposited sequentially starting at address
 * "buffer", until "count" registers have been read.
 *
 * @param reg           Starting register to read
 * @param buffer        Pointer to byte buffer
 * @param count         Number of bytes to read
 */
void spi_read_burst_register(uint8_t reg, uint8_t *buffer, uint8_t count) {

    char i;

    GPIO_setOutputLowOnPin(cs_port, cs_pin);                    // Enable CS
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                 EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;             // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, reg | READ_BURST);    // Send address
    while(EUSCI_A_SPI_isBusy(baseAddress));                     // Wait until SPI is done transmitting
    EUSCI_A_SPI_transmitData(baseAddress, 0);                   // Dummy write to read 1st data byte
    // Addr byte is now being TX'ed, with dummy byte to follow immediately after
    EUSCI_A_SPI_clearInterrupt(baseAddress, EUSCI_A_SPI_RECEIVE_INTERRUPT);                 // Clear flag
    while (!(EUSCI_A_SPI_getInterruptStatus(baseAddress, EUSCI_A_SPI_RECEIVE_INTERRUPT)));  // Wait for end of 1st data byte TX
    // First data byte now in RXBUF
    for (i = 0; i < (count-1); i++) {

        EUSCI_A_SPI_transmitData(baseAddress, 0);           //Initiate next data RX, meanwhile..
        buffer[i] = EUSCI_A_SPI_receiveData(baseAddress);   // Store data from last data RX
        while (!(EUSCI_A_SPI_getInterruptStatus(baseAddress, EUSCI_A_SPI_RECEIVE_INTERRUPT)));  // Wait for RX to finish

    }
    buffer[count-1] = EUSCI_A_SPI_receiveData(baseAddress); // Store last RX byte in buffer
    GPIO_setOutputHighOnPin(cs_port, cs_pin);               // Disable CS

}

/**
 *  Special read function for reading status registers.  Reads status register
 *  at register "reg" and returns the value read.
 *
 * @returns status byte
 */
uint8_t spi_read_status(uint8_t reg) {

    uint8_t status;

    GPIO_setOutputLowOnPin(cs_port, cs_pin);                    // Enable CS
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                   EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;           // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, reg | READ_BURST);    // send address with read
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                   EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;           // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, 0);                   // Dummy write so we can read data
    while(EUSCI_A_SPI_isBusy(baseAddress));                     // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(baseAddress);              // read RX buffer
    GPIO_setOutputHighOnPin(cs_port, cs_pin);                   // Disable CS

    return status;

}

/**
 * Send a command strobe (one-byte register access that initiates an action)
 *
 * @returns status byte
 */
uint8_t spi_send_strobe(uint8_t reg) {

    uint8_t status = 0;

    GPIO_setOutputLowOnPin(cs_port, cs_pin);                // Enable CS
    while (!EUSCI_A_SPI_getInterruptStatus(baseAddress,
                   EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;       // wait for empty TX buffer
    EUSCI_A_SPI_transmitData(baseAddress, reg);             // send strobe
    while(EUSCI_A_SPI_isBusy(baseAddress));                 // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(baseAddress);          // read RX buffer
    GPIO_setOutputHighOnPin(cs_port, cs_pin);               // Disable CS

    return status;

}

