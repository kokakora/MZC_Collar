/*
 * gpio_isr.h
 *
 *  Created on: Mar 16, 2021
 *      Author: Jonathan Chen
 */

#ifndef GPIO_ISR_H_
#define GPIO_ISR_H_

#include "driverlib.h"
#include "Board.h"
#include <stdio.h>
//#include "ui_pins.h"


/**
 * assigns class-specific functions to be called by the Port 2 ISR for pins 4, 5, and 6
 */
void configure_ISR_functions_PORT2_btns(void (*fun_ptr_p2_p4)(), void (*fun_ptr_p2_p5)(), void (*fun_ptr_p2_p6)());

/**
 * assigns class-specific functions to be called by the Port 3 ISR for pins 2 and 3
 */
void configure_ISR_functions_PORT3_btns(void (*fun_ptr_p3_p2)(), void (*fun_ptr_p3_p3)());

/**
 * assigns class-specific functions to be called by the Port 3 ISR for pin 6
 */
void configure_ISR_functions_PORT3_6(void (*fun_ptr_p3_p6)());

#endif /* GPIO_ISR_H_ */
