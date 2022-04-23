/*
 * setup_common.h
 *
 *  Created on: Mar 13, 2021
 *      Author: Jonathan Chen
 */

#ifndef SETUP_COMMON_H_
#define SETUP_COMMON_H_

#include <cc_spi.h>
#include "audio.h"
#include "ui.h"
#include "gpio_isr.h"
#include "driverlib.h"
#include "Board.h"


/**
 * Setup pins for SPI to radio transceiver including interrupt pin
 */
void configure_SPI_pins_radio();

/**
 * Setup pins for clock outputs and UART to computer
 */
void configure_debug();

/**
 * Initialize UART peripheral to computer
 */
void init_debug(uint8_t** receivedPacket, uint8_t* packetLength, uint8_t* position);

/**
 * Setup clocks for MCLK at 16 MHz, SMCLK at 2 MHz, ACLK at 32.768 kHz
 */
void init_CLKs();

/**
 * Setup TimerB using ACLK to trigger every ~0.1s, or 3277 cycles
 */
void init_timer();

/**
 * Setup clock speed, pins, and spi for controller
 * Clears LOCKLPM5 bit in the process
 */
void setup_controller(uint8_t** receivedPacket, uint8_t* packetLength, uint8_t* position);

/**
 * Setup clock speed, pins, and spi for collar
 * Clears LOCKLPM5 bit in the process
 */
void setup_collar(uint8_t** receivedPacket, uint8_t* packetLength, uint8_t* position);

#endif /* SETUP_COMMON_H_ */
