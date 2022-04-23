// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#ifndef __CC2500__
#define __CC2500__

#include <cc_spi.h>
#include "CC2500_reg_defs.h"
#include "driverlib.h"
#include "gpio_isr.h"
#include "global_vars.h"

/**
 * Setup SPI, reset the CC2500, then write to registers that must be different from their reset values.
 * Register values were determined with the TI SmartRF program.
 *
 * The CC2500 supports a frequency range of 2400 - 2483.5 MHz, and it can be split up into 256 (or fewer) channels.
 * This function configures the CC2500 for 160 (0-159) channels.
 * Base frequency = 2411 MHz,  channel spacing = 405 kHz,  receiver filter bandwidth = 406 kHz.
 * Therefore: channel 0 = 2411 MHz ... channel 159 = 2475 MHz
 *
 * @param baseAddress   The base address of the EUSCI_A_SPI Master module.
 * @param selectClockSource Selects Clock source in driverlib
 *    Valid values are:
 *    - \b EUSCI_A_SPI_CLOCKSOURCE_UCLK
 *    - \b EUSCI_A_SPI_CLOCKSOURCE_ACLK
 *    - \b EUSCI_A_SPI_CLOCKSOURCE_MODCLK
 *    - \b EUSCI_A_SPI_CLOCKSOURCE_SMCLK
 * @param clockSourceFrequency  Current frequency of of the clock source in Hz
 * @param gdo0port              GDO0 interrupt port
 * @param gdo0pin               GDO0 interrupt pin
 * @param packet_size           Number of bytes in a packet
 * @param packet_handler        Your function that will be called after successfully receiving a packet
 */
void cc2500_setup(uint16_t baseAddress, uint8_t selectClockSource, uint32_t clockSourceFrequency, uint8_t csport, uint16_t cspin, uint8_t gdo0port, uint16_t gdo0pin, uint8_t packet_size, void (*packet_handler)(uint8_t byte_count, uint8_t bytes[]));

/**
 * Sets the current channel.
 *
 * @param channel	channel number
 */
void cc2500_set_channel(uint8_t channel);

/**
 * Gets the current channel.
 *
 * @returns 		channel number
 */
uint8_t cc2500_get_channel();

/**
 * Flushes the receiver FIFO
 */
void cc2500_flush_rx_fifo();

/**
 * Enters receiver mode
 */
void cc2500_enter_rx_mode();

/**
 * Flushes the transmit FIFO.
 */
void cc2500_flush_tx_fifo(void);

/**
 * Enters transmit mode.
 */
void cc2500_enter_tx_mode(void);

/**
 * Transmits a packet. Automatically enters tx mode.
 *
 * @param bytes    The packet.
 */
void cc2500_transmit_packet(uint8_t bytes[]);

/**
 * Gets the status byte
 *
 * @returns		status
 */
uint8_t cc2500_get_status();

/**
 * Reads the RSSI value of the current channel.
 * The register contains the RSSI has a signed byte,
 * this function converts it to an unsigned integer in the 0 - 239 range. 0 = min power.
 *
 * @returns			RSSI value
 */
uint8_t cc2500_get_rssi();

#endif
