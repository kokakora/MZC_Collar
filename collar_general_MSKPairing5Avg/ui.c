/*
 * ui.c
 *
 *  Created on: Mar 16, 2021
 *      Author: Jonathan Chen
 */
#include "ui.h"

// pointer variables to system state variables
//int *controller_state_ptr;
//int *zone_ptr;
//bool *automation_ptr;
//bool *homingON_ptr;



/**
 * Handles up button being pressed
 */
void upPress() {

    if (zone < 4) {

        zone = zone + 1;

    }
}

/**
 * Handles down button being pressed
 */
void downPress() {

    if (zone > 1) {

        zone = zone - 1;

    }
}

/**
 * Handles manual button being pressed
 */
void manualPress() {

    //TODO: send transmission to collar to play an audio tone

}

/**
 * Handles homing button being pressed
 */
void homingPress() {

    //TODO: send transmission to collar toggling the homing signal

}

/**
 * Handles boundary button being pressed
 */
void boundaryPress() {

    //TODO: sends transmission to collar toggling the boundary checking mode

}

/**
 * Setup pins for user interface buttons on controller
 */
void configure_button_pins() {

    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_UP_BUTTON, GPIO_PIN_UP_BUTTON);
    GPIO_selectInterruptEdge(GPIO_PORT_UP_BUTTON, GPIO_PIN_UP_BUTTON, GPIO_LOW_TO_HIGH_TRANSITION);

    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_DOWN_BUTTON, GPIO_PIN_DOWN_BUTTON);
    GPIO_selectInterruptEdge(GPIO_PORT_DOWN_BUTTON, GPIO_PIN_DOWN_BUTTON, GPIO_LOW_TO_HIGH_TRANSITION);

    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_MANUAL_BUTTON, GPIO_PIN_MANUAL_BUTTON);
    GPIO_selectInterruptEdge(GPIO_PORT_MANUAL_BUTTON, GPIO_PIN_MANUAL_BUTTON, GPIO_LOW_TO_HIGH_TRANSITION);

    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_HOMING_BUTTON, GPIO_PIN_HOMING_BUTTON);
    GPIO_selectInterruptEdge(GPIO_PORT_HOMING_BUTTON, GPIO_PIN_HOMING_BUTTON, GPIO_LOW_TO_HIGH_TRANSITION);

    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_BOUNDARY_BUTTON, GPIO_PIN_BOUNDARY_BUTTON);
    GPIO_selectInterruptEdge(GPIO_PORT_BOUNDARY_BUTTON, GPIO_PIN_BOUNDARY_BUTTON, GPIO_LOW_TO_HIGH_TRANSITION);

}

/**
 * Clear IFGs and enable button interrupts
 */
void init_button_pins() {

    // Clear interrupt flags
    GPIO_clearInterrupt(GPIO_PORT_UP_BUTTON, GPIO_PIN_UP_BUTTON);
    GPIO_clearInterrupt(GPIO_PORT_DOWN_BUTTON, GPIO_PIN_DOWN_BUTTON);
    GPIO_clearInterrupt(GPIO_PORT_MANUAL_BUTTON, GPIO_PIN_MANUAL_BUTTON);
    GPIO_clearInterrupt(GPIO_PORT_HOMING_BUTTON, GPIO_PIN_HOMING_BUTTON);
    GPIO_clearInterrupt(GPIO_PORT_BOUNDARY_BUTTON, GPIO_PIN_BOUNDARY_BUTTON);

    // Assign UI functions to interrupts
    // Port 2: Manual, Homing, Boundary to 2.4, 2.5, 2.6, Port 3: Up, Down to 3.3, 3.2
    configure_ISR_functions_PORT2_btns(&manualPress, &homingPress, &boundaryPress);
    configure_ISR_functions_PORT3_btns(&downPress, &upPress);

    // Enable interrupts
    GPIO_enableInterrupt(GPIO_PORT_UP_BUTTON, GPIO_PIN_UP_BUTTON);
    GPIO_enableInterrupt(GPIO_PORT_DOWN_BUTTON, GPIO_PIN_DOWN_BUTTON);
    GPIO_enableInterrupt(GPIO_PORT_MANUAL_BUTTON, GPIO_PIN_MANUAL_BUTTON);
    GPIO_enableInterrupt(GPIO_PORT_HOMING_BUTTON, GPIO_PIN_HOMING_BUTTON);
    GPIO_enableInterrupt(GPIO_PORT_BOUNDARY_BUTTON, GPIO_PIN_BOUNDARY_BUTTON);

}

/**
 * Button Press ISRs
 */
// Buttons on 2.4, 2.5, and 2.6
/*
#pragma vector = PORT2_VECTOR
__interrupt void ISR_PORT2(void)
{

    if (GPIO_getInterruptStatus(GPIO_PORT_UP_BUTTON, GPIO_PIN_UP_BUTTON)) {

        //call function()
        GPIO_clearInterrupt(GPIO_PORT_UP_BUTTON, GPIO_PIN_UP_BUTTON);

    }
    else if (GPIO_getInterruptStatus(GPIO_PORT_DOWN_BUTTON, GPIO_PIN_DOWN_BUTTON)) {

        //call function()
        GPIO_clearInterrupt(GPIO_PORT_DOWN_BUTTON, GPIO_PIN_DOWN_BUTTON);

    }
    else if (GPIO_getInterruptStatus(GPIO_PORT_MANUAL_BUTTON, GPIO_PIN_MANUAL_BUTTON)) {

        //call function()
        GPIO_clearInterrupt(GPIO_PORT_MANUAL_BUTTON, GPIO_PIN_MANUAL_BUTTON);

    }
    else {
    }

}
*/
// Buttons on 3.2 and 3.3
/*#pragma vector = PORT3_VECTOR
__interrupt void ISR_PORT3(void)
{
    if (GPIO_getInterruptStatus(GPIO_PORT_HOMING_BUTTON, GPIO_PIN_HOMING_BUTTON)) {

        //call function()
        GPIO_clearInterrupt(GPIO_PORT_HOMING_BUTTON, GPIO_PIN_HOMING_BUTTON);

    }
    else if (GPIO_getInterruptStatus(GPIO_PORT_BOUNDARY_BUTTON, GPIO_PIN_BOUNDARY_BUTTON)) {

        //call function()
        GPIO_clearInterrupt(GPIO_PORT_BOUNDARY_BUTTON, GPIO_PIN_BOUNDARY_BUTTON);

    }
}*/

