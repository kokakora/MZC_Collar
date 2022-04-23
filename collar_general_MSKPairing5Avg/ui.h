/*
 * ui.h
 *
 *  Created on: Mar 16, 2021
 *      Author: Jonathan Chen
 */

#ifndef UI_H_
#define UI_H_

#include "driverlib.h"
#include "Board.h"
#include "ui_pins.h"
#include "gpio_isr.h"
#include "global_vars.h"


/**
 * Handles up button being pressed
 */
void upPress();

/**
 * Handles down button being pressed
 */
void downPress();

/**
 * Handles manual button being pressed
 */
void manualPress();

/**
 * Handles homing button being pressed
 */
void homingPress();

/**
 * Handles boundary button being pressed
 */
void boundaryPress();

/**
 * Setup pins for user interface buttons on controller
 */
void configure_button_pins();

/**
 * Clear IFGs and enable button interrupts
 */
void init_button_pins();

#endif /* UI_H_ */
