// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain
// Combined and ported MSP430 Interface to CC1100/2500 Code Library by Jonathan Chen, Feb 23, 2021

#ifndef __SPI__
#define __SPI__

#include "driverlib.h"
#include "CC2500_reg_defs.h"

/**
 * Configures and enables SPI for 8bit mode, 4wire mode, with a prescaler based on SMCLK on eUSCIA_0
 * Configures eUSCI to be 3-wire, but functions in this file manually control the CSn pin.
 *
 * Params are the same as for EUSCI_A_SPI_initMasterParam
 * Does not configure pins. Use GPIO_setAsPeripheralModuleFunction driver functions.
 * Also takes cs_port, cs_pin for the chip-select pin for 4-wire mode
 */
void spi_setup(uint16_t baseAddress, uint8_t selectClockSource, uint32_t clockSourceFrequency,
               uint32_t desiredSpiClock, uint16_t msbFirst, uint16_t clockPhase, uint16_t clockPolarity, uint16_t spiMode, uint8_t csport, uint16_t cspin);


/**
 * Write to a single configuration register on the CC2500.
 * NOT used for multi-byte registers or command strobes.
 *
 * @param reg           Register to read or write
 * @param val           Value to write
 * @returns             Status byte for writes, or value for reads
 */
uint8_t spi_write_register(uint8_t reg, uint8_t val);

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
void spi_write_burst_register(uint8_t reg, uint8_t *buffer, uint8_t count);

/**
 * Read from a single configuration register on the CC2500
 *
 * @param reg           Register to read
 * @returns             Status byte for writes, or value for reads
 */
uint8_t spi_read_register(uint8_t reg);

/**
 * Reads multiple configuration registers, the first register being at address
 * "reg".  Values read are deposited sequentially starting at address
 * "buffer", until "count" registers have been read.
 *
 * @param reg           Starting register to read
 * @param buffer        Pointer to byte buffer
 * @param count         Number of bytes to read
 */
void spi_read_burst_register(uint8_t reg, uint8_t *buffer, uint8_t count);

/**
 *  Special read function for reading status registers.  Reads status register
 *  at register "reg" and returns the value read.
 *
 * @param reg           Statis register
 * @returns             Status byte
 */
uint8_t spi_read_status(uint8_t reg);

/**
 * Send a command strobe (one-byte register access that initiates an action)
 *
 * @returns status byte
 */
uint8_t spi_send_strobe(uint8_t reg);
#endif
