/*
 * gpio_isr.c
 *
 *  Created on: Mar 16, 2021
 *      Author: Jonathan Chen
 *
 *  This file is needed to help assign multiple function calls from separate classes
 *  to the same ISR.
 */

#include "gpio_isr.h"

/**
 * Function Pointers
 */
void (*fun_port2_pin4)() = NULL;  //
void (*fun_port2_pin5)() = NULL;  //
void (*fun_port2_pin6)() = NULL;  //
void (*fun_port3_pin2)() = NULL;  //
void (*fun_port3_pin3)() = NULL;  //
void (*fun_port3_pin6)() = NULL;  //


/**
 * assigns class-specific functions to be called by the Port 2 ISR for pins 4, 5, and 6
 */
void configure_ISR_functions_PORT2_btns(void (*fun_ptr_p2_p4)(), void (*fun_ptr_p2_p5)(), void (*fun_ptr_p2_p6)()) {

    fun_port2_pin4 = fun_ptr_p2_p4;
    fun_port2_pin5 = fun_ptr_p2_p5;
    fun_port2_pin6 = fun_ptr_p2_p6;

}

/**
 * assigns class-specific functions to be called by the Port 3 ISR for pins 2 and 3
 */
void configure_ISR_functions_PORT3_btns(void (*fun_ptr_p3_p2)(), void (*fun_ptr_p3_p3)()) {

    fun_port3_pin2 = fun_ptr_p3_p2;
    fun_port3_pin3 = fun_ptr_p3_p3;

}

/**
 * assigns class-specific functions to be called by the Port 3 ISR for pin 6
 */
void configure_ISR_functions_PORT3_6(void (*fun_ptr_p3_p6)()) {

    fun_port3_pin6 = fun_ptr_p3_p6;

}

// Buttons on 2.4, 2.5, and 2.6
#pragma vector = PORT2_VECTOR
__interrupt void ISR_PORT2(void)
{

    if (GPIO_getInterruptStatus(GPIO_PORT_P2, GPIO_PIN4)) {

        fun_port2_pin4();   //call function()
        GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN4);

    }
    else if (GPIO_getInterruptStatus(GPIO_PORT_P2, GPIO_PIN5)) {

        fun_port2_pin5();   //call function()
        GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN5);

    }
    else if (GPIO_getInterruptStatus(GPIO_PORT_P2, GPIO_PIN6)) {

        fun_port2_pin6();   //call function()
        GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN6);

    }
    else {
    }

}

// Buttons on 3.2 and 3.3, transceiver gpo0 pin on 3.6
#pragma vector = PORT3_VECTOR
__interrupt void ISR_PORT3(void)
{
    // Interrupt for indicating when radio has received a packet
    // Disable when transmitting over SPI
    if (GPIO_getInterruptStatus(GPIO_PORT_P3, GPIO_PIN6)) {

        GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN6);
        GPIO_disableInterrupt(GPIO_PORT_P3, GPIO_PIN6); // disable interrupt
        Timer_B_disableCaptureCompareInterrupt(TB1_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_0);
        fun_port3_pin6();
        __disable_interrupt();  // Make sure interrupt is off because of audio
        GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN6);  // reenable interrupt
        Timer_B_enableCaptureCompareInterrupt(TB1_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_0);

    }
    else if (GPIO_getInterruptStatus(GPIO_PORT_P3, GPIO_PIN2)) {

        fun_port3_pin2();  //call function()
        GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN2);

    }
    else if (GPIO_getInterruptStatus(GPIO_PORT_P3, GPIO_PIN3)) {

        fun_port3_pin3();  //call function()
        GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN3);

    }
}
