/*
 * setup_common.c
 *
 *  Created on: Mar 13, 2021
 *      Author: Jonathan Chen
 */
#include <setup_common.h>
char UARTTXString[] = "Initiated UART, SPI comms!\n\r";

/**
 * Helper method to send an opening message via UART
 */
void UARTTransmitStart(uint8_t** receivedPacket, uint8_t* packetLength, uint8_t* position) {
    *receivedPacket = (uint8_t*) UARTTXString;
    *packetLength = sizeof(UARTTXString);
    EUSCI_A_UART_transmitData(EUSCI_A1_BASE, *receivedPacket[*position]);
    *position = *position + 1;
    // Enable USCI_A1 TX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE,
         EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT);
}

/**
 * Setup pins for SPI to radio transceiver including interrupt pin
 */
void configure_SPI_pins_radio() {

    //--------------------Transceiver 1-----------------------//

    // Configure SPI Pins for, UCA0CLK, UCA0TXD/UCA0SIMO
    /*
    * Select Port 1
    * Set Pin 5 and Pin 7 to output Primary Module Function
    */
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN5 + GPIO_PIN7,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Configure SPI CSn pin as output UCA0STE
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN4);

    // Configure SPI Pins for UCA0RXD/UCA0SOMI
    /*
    * Select Port 1
    * Set Pin 6 to input Primary Module Function
    */
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1,
        GPIO_PIN6,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Configure SLV RDY
//    GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN6);


    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Deactivate CSn

    // Configure radio Reset as output
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0); //set the radio in reset

    // Configure rc32400 interrupt pin 3.6 SLV RDY
    GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN6);
    GPIO_selectInterruptEdge(GPIO_PORT_P3, GPIO_PIN6, GPIO_HIGH_TO_LOW_TRANSITION);   // GDO0 falls at end of packet

}


/**
 * Setup pins for clock outputs and UART to computer
 */
void configure_debug() {

    //Configure Pin 3.0 to monitor MCLK
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_MCLK,
            GPIO_PIN_MCLK,
            GPIO_FUNCTION_MCLK
    );

    //Configure Pin 3.4 to monitor SMCLK
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_SMCLK,
            GPIO_PIN_SMCLK,
            GPIO_FUNCTION_SMCLK
        );

    //Configure UART pins
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_UCA1TXD,
        GPIO_PIN_UCA1TXD,
        GPIO_FUNCTION_UCA1TXD
    );
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_UCA1RXD,
        GPIO_PIN_UCA1RXD,
        GPIO_FUNCTION_UCA1RXD
    );
}

/**
 * Initialize UART peripheral to computer
 */
void init_debug(uint8_t** receivedPacket, uint8_t* packetLength, uint8_t* position) {

    //Configure UART
    //SMCLK = 2 MHz, Baudrate = 115200
    //UCBRx = 16, UCBRFx = 0, UCBRSx = 0xD6, UCOS16 = 0
    EUSCI_A_UART_initParam paramUART = {0};
    paramUART.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    paramUART.clockPrescalar = 16;
    paramUART.firstModReg = 0;
    paramUART.secondModReg = 0xD6;
    paramUART.parity = EUSCI_A_UART_NO_PARITY;
    paramUART.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    paramUART.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    paramUART.uartMode = EUSCI_A_UART_MODE;
    paramUART.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

    if (STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A1_BASE, &paramUART)) {
        return;
    }

    // Take USCI_A1 out of software reset
    EUSCI_A_UART_enable(EUSCI_A1_BASE);

    // Clear USCI_A1 interrupt flags
    EUSCI_A_UART_clearInterrupt(EUSCI_A1_BASE,
        EUSCI_A_UART_RECEIVE_INTERRUPT);
    EUSCI_A_UART_clearInterrupt(EUSCI_A1_BASE,
        EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT);

    // Enable USCI_A1 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE,
        EUSCI_A_UART_RECEIVE_INTERRUPT);

    UARTTransmitStart(receivedPacket, packetLength, position);
    __delay_cycles(100);
    __enable_interrupt();
}

/**
 * Setup clocks for MCLK at 16 MHz, SMCLK at 2 MHz, ACLK at 32.768 kHz
 */
void init_CLKs() {

    // Set Ratio and Desired MCLK Frequency and initialize DCO to 2 MHz
    CS_initFLLSettle(
       16000,
       489
            );

    //Set ACLK = REFOCLK with clock divider of 1
    CS_initClockSignal(CS_ACLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    //Set SMCLK = DCO with frequency divider of 8
    CS_initClockSignal(CS_SMCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_8);
    //Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
}

/**
 * Setup TimerB using ACLK to trigger every ~0.1s, or 3277 cycles
 */
void init_timer() {

    //Configure and start timer
    Timer_B_initUpModeParam tParam = {0};
    tParam.clockSource = TIMER_B_CLOCKSOURCE_ACLK;
    tParam.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_1;
    //tParam.timerPeriod = 32770; // Currently set at every ~1s
    tParam.timerPeriod = 16385; // Currently set at every ~0.5s
    tParam.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_DISABLE;
    tParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_ENABLE;
    tParam.timerClear = TIMER_B_DO_CLEAR;
    tParam.startTimer = true;
    Timer_B_initUpMode(TB1_BASE, &tParam);

}

/**
 * Setup clock speed, pins, and spi
 * Clears LOCKLPM5 bit in the process
 */
void setup_controller(uint8_t** receivedPacket, uint8_t* packetLength, uint8_t* position) {
    // Outdated: Not the same function as in the controller program.
    init_CLKs();
    //configure_debug(); // *DEBUG*
    configure_SPI_pins_radio();
    configure_audio_pins();
    configure_button_pins();

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //init_debug(receivedPacket, packetLength, position); // *DEBUG*
    init_audio();
    init_button_pins();

}

/**
 * Setup clock speed, pins, and spi for collar
 * Clears LOCKLPM5 bit in the process
 * Does not use ui functions, which need to be either eliminated or unified with controller program
 */
void setup_collar(uint8_t** receivedPacket, uint8_t* packetLength, uint8_t* position) {

    init_CLKs();
    //configure_debug();  // *DEBUG*
    configure_SPI_pins_radio();
    configure_audio_pins();

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //init_debug(receivedPacket, packetLength, position); // *DEBUG*
    init_audio();

}
