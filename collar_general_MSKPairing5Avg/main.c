/*
 * main.c
 *
 *  Created on: Feb 24, 2021
 *      Author: Jonathan Chen
 *
 *      Collar main.c
 */

#include <cc_spi.h>
#include <setup_common.h>
#include "driverlib.h"
#include "Board.h"
#include "cc2500.h"
#include "rc2400.h"

uint8_t* receivedPacket;    // Points to a received packet
uint8_t packetLength;
uint8_t position = 0;

// Zone Signals from First collar (if set to primary collar)
uint8_t* packetInZone1P = "pr1\n\r";
uint8_t* packetInZone2P = "pr2\n\r";
uint8_t* packetInZone3P = "pr3\n\r";
uint8_t* packetInZone4P = "pr4\n\r";
uint8_t* packetInZone5P = "pr5\n\r";

// Zone signals from Second collar (if set to secondary collar)
uint8_t* packetInZone1S = "se1\n\r";
uint8_t* packetInZone2S = "se2\n\r";
uint8_t* packetInZone3S = "se3\n\r";
uint8_t* packetInZone4S = "se4\n\r";
uint8_t* packetInZone5S = "se5\n\r";

/*
 * Packet definitions (Commands and signals)
 */

// Zone packets
uint8_t* packetZone1 = "zz1\n\r";
uint8_t* packetZone2 = "zz2\n\r";
uint8_t* packetZone3 = "zz3\n\r";
uint8_t* packetZone4 = "zz4\n\r";
uint8_t* packetZone5 = "zz5\n\r";

// Primary Collar Zone Reports
uint8_t* packetSetZone1P = "sp1\n\r";
uint8_t* packetSetZone2P = "sp2\n\r";
uint8_t* packetSetZone3P = "sp3\n\r";
uint8_t* packetSetZone4P = "sp4\n\r";

// Primary Collar Commands
uint8_t* packetBoundaryOnP = "bpn\n\r";
uint8_t* packetBoundaryOffP = "bpf\n\r";
uint8_t* packetHomingOnP = "hpn\n\r";
uint8_t* packetHomingOffP = "hpf\n\r";
uint8_t* packetManualToneP = "mpn\n\r";

// Secondary Collar Zone Reports
uint8_t* packetSetZone1 = "ss1\n\r";
uint8_t* packetSetZone2 = "ss2\n\r";
uint8_t* packetSetZone3 = "ss3\n\r";
uint8_t* packetSetZone4 = "ss4\n\r";

// Secondary Collar Commands
uint8_t* packetBoundaryOn = "bsn\n\r";
uint8_t* packetBoundaryOff = "bsf\n\r";
uint8_t* packetHomingOn = "hsn\n\r";
uint8_t* packetHomingOff = "hsf\n\r";
uint8_t* packetManualTone = "msn\n\r";

extern volatile int zone1_col1_ack;
extern volatile int zone2_col1_ack;
extern volatile int zone3_col1_ack;
extern volatile int zone4_col1_ack;
extern volatile int zone1_col2_ack;
extern volatile int zone2_col2_ack;
extern volatile int zone3_col2_ack;
extern volatile int zone4_col2_ack;

uint8_t controllerAddr = 0; // Address of paired controller. Automatically sets after powering on and receiving controller broadcast
int pairedStatus = 0;       // 0 = not paired; 1 = paired primary; 2 = paired secondary

// State variables
int zone = 1;               // which zone the dog should stay within
bool automation = false;    // whether the boundary tone is on
bool homingON = false;      // whether the homing signal is on
int collar_zone = 5;                    // Which zone the collar is currently within
int last_collar_zone1 = 1;               // The last collar zone detected # before current
int last_collar_zone2 = 1;
int last_collar_zone3 = 1;
int last_collar_zone4 = 1;
int cycles_since_last_transmission = 0; // Number of timer triggers since the last transmission was received

bool playedHomingLast = false;  // helper variable to offset homing signal to every 2 cycles


/**
 * Packet handler that triggers when a packet is received to be interpreted
 * ----Needs to be updated for each radio module----
 */
void process_new_packet(uint8_t byte_count, uint8_t bytes[]) {

    // put bytes into receivedString to be sent to computer via ISR
    cycles_since_last_transmission = 0; // Reset cycles timer
    receivedPacket = bytes;
    packetLength = byte_count;

    // Interpret packet
    switch(bytes[0]) // check first byte in packet
    {
        case 'z':   // zone signal
            switch(bytes[2])    // check third byte in packet
            {
                case '1':
                    collar_zone = 4;
                    break;
                case '2':
                    collar_zone = 3;
                    break;
                case '3':
                    collar_zone = 2;
                    break;
                case '4':
                    collar_zone = 1;
                    break;
                case '5':
                    if (automation) {
                        if ((collar_zone > zone) && (last_collar_zone1 > zone) && (last_collar_zone2 > zone) && (last_collar_zone3 > zone) && (last_collar_zone4 > zone)) {
                            play_2kHz_pulse(200);   // if outside of zone, play corrective tone
                        }
                    }

                    // Pair collar to one of two roles if not paired
                    if (pairedStatus == 0) {
                        if (bytes[3] == 0) {    // 0 collars paired to controller so far
                            pairedStatus = 1;           // paired as primary collar
                            controllerAddr = bytes[4];  // store controller address
                            play_1kHz_pulse(100);       // Indicate pairing
                        }
                        else if (bytes[3] == 1) {    // 1 collar paired to controller so far
                            pairedStatus = 2;           // paired as secondary collar
                            controllerAddr = bytes[4];  // store controller address
                            play_2kHz_pulse(200);       // Indicate pairing
                        }
                    }

                    int sendZone = 5;
                    if (collar_zone < sendZone) {
                        sendZone = collar_zone;
                    }
                    if (last_collar_zone1 < sendZone) {
                        sendZone = last_collar_zone1;
                    }
                    if (last_collar_zone2 < sendZone) {
                        sendZone = last_collar_zone2;
                    }
                    if (last_collar_zone3 < sendZone) {
                        sendZone = last_collar_zone3;
                    }
                    if (last_collar_zone4 < sendZone) {
                        sendZone = last_collar_zone4;
                    }

                    // Transmit calculated zone location of collar
                    if (pairedStatus == 1) {  // Primary Collar
                        __delay_cycles(1600000);   // Delay ~100 ms to space out transmissions so controller can receive 2nd collar
                        switch(sendZone)
                            {
                                case 1:
                                    cc2500_transmit_packet(packetInZone1P);
                                    break;
                                case 2:
                                    cc2500_transmit_packet(packetInZone2P);
                                    break;
                                case 3:
                                    cc2500_transmit_packet(packetInZone3P);
                                    break;
                                case 4:
                                    cc2500_transmit_packet(packetInZone4P);
                                    break;
                                case 5:
                                    cc2500_transmit_packet(packetInZone5P);
                                    break;
                            }
                    }
                    else if (pairedStatus == 2) {  // Primary Collar
                        switch(sendZone)
                            {
                                case 1:
                                    cc2500_transmit_packet(packetInZone1S);
                                    break;
                                case 2:
                                    cc2500_transmit_packet(packetInZone2S);
                                    break;
                                case 3:
                                    cc2500_transmit_packet(packetInZone3S);
                                    break;
                                case 4:
                                    cc2500_transmit_packet(packetInZone4S);
                                    break;
                                case 5:
                                    cc2500_transmit_packet(packetInZone5S);
                                    break;
                            }
                    }

                    // Reset collar zones, store last zone
                    last_collar_zone4 = last_collar_zone3;
                    last_collar_zone3 = last_collar_zone2;
                    last_collar_zone2 = last_collar_zone1;
                    last_collar_zone1 = collar_zone;
                    collar_zone = 5;
                    break;
            }
            break;

        case 's':   // set zone
            if (pairedStatus == 1) {  // Primary Collar
                if (bytes[1] == 'p') {  // check that it goes to the first collar
                    switch(bytes[2])
                    {
                        case '1':   // Set zone to 1
                            zone = 1;
                            break;
                        case '2':   // Set zone to 2
                            zone = 2;
                            break;
                        case '3':   // Set zone to 3
                            zone = 3;
                            break;
                        case '4':   // Set zone to 4
                            zone = 4;
                            break;
                    }
                }
            }
            else if (pairedStatus == 2) {  // Secondary Collar
                if (bytes[1] == 's') {  // check that it goes to the second collar
                    switch(bytes[2])
                    {
                        case '1':   // Set zone to 1
                            zone = 1;
                            break;
                        case '2':   // Set zone to 2
                            zone = 2;
                            break;
                        case '3':   // Set zone to 3
                            zone = 3;
                            break;
                        case '4':   // Set zone to 4
                            zone = 4;
                            break;
                    }
                }
            }
            break;
        case 'm':   // play manual tone
            if (pairedStatus == 1) {  // Primary Collar
                if (bytes[1] == 'p') {  // check that it goes to the first collar
                    play_2kHz_pulse(400);
                }
                break;
            }
            else if (pairedStatus == 2) {  // Second Collar
                if (bytes[1] == 's') {  // check that it goes to the first collar
                    play_2kHz_pulse(400);
                }
                break;
            }
        case 'b':   // boundary on/off
            if (pairedStatus == 1) {  // Primary Collar
                if (bytes[1] == 'p') {  // check that it goes to the first collar
                    if (bytes[2]=='n') {
                        // boundary on
                        automation = true;
                        play_2kHz_pulse(400);   // Play tone to indicate signal reception
                    }
                    else {
                        // boundary off
                        automation = false;
                        play_1kHz_pulse(200);   // Play tone to indicate signal reception
                    }
                }
                break;
            }
            else if (pairedStatus == 2) {  // Second Collar
                if (bytes[1] == 's') {  // check that it goes to the first collar
                    if (bytes[2]=='n') {
                        // boundary on
                        automation = true;
                        play_2kHz_pulse(400);   // Play tone to indicate signal reception
                    }
                    else {
                        // boundary off
                        automation = false;
                        play_1kHz_pulse(200);   // Play tone to indicate signal reception
                    }
                }
                break;
            }
        case 'h':   // homing on/off
            if (pairedStatus == 1) {  // Primary Collar
                if (bytes[1] == 'p') {  // check that it goes to the first collar
                    if (bytes[2]=='n') {
                        // homing on
                        homingON = true;
                        play_2kHz_pulse(400);   // Play tone to indicate signal reception
                    }
                    else {
                        // homing off
                        homingON = false;
                        play_1kHz_pulse(200);   // Play tone to indicate signal reception
                    }
                }
                break;
            }
            else if (pairedStatus == 2) {  // Second Collar
                if (bytes[1] == 's') {  // check that it goes to the first collar
                    if (bytes[2]=='n') {
                        // homing on
                        homingON = true;
                        play_2kHz_pulse(400);   // Play tone to indicate signal reception
                    }
                    else {
                        // homing off
                        homingON = false;
                        play_1kHz_pulse(200);   // Play tone to indicate signal reception
                    }
                }
                break;
            }
    }
}

void main(void)
{
    volatile uint16_t i;

    //Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    setup_collar(&receivedPacket, &packetLength, &position);

    __delay_cycles(10000000);
    // Initialize CC2500 module on SPI
    //cc2500_setup(EUSCI_A0_BASE, EUSCI_A_SPI_CLOCKSOURCE_SMCLK, 2000000, GPIO_PORT_P1, GPIO_PIN4, GPIO_PORT_P3, GPIO_PIN6, 5, &process_new_packet);
    rc2400_config();

//    cc2500_enter_rx_mode();
    __delay_cycles(20000);       // Wait for calibration to complete

    init_timer();   // start timer to trigger timeout or homing signal
    while(1);
    __bis_SR_register(LPM0_bits + GIE);      // CPU off, enable interrupts
    __no_operation();                       // Remain in LPM0
}

//******************************************************************************
//
//This is the USCI_A1 interrupt vector service routine. It can only be used when
//connected to a debugger to the computer. Not used in embedded program
//
//******************************************************************************
#pragma vector = EUSCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        /*case USCI_UART_UCRXIFG:
            RXData = EUSCI_A_UART_receiveData(EUSCI_A1_BASE);
            // Check value
            if(RXData == TXData)
            {
                TXData = TXData+1;
                // Load data onto buffer
                EUSCI_A_UART_transmitData(EUSCI_A1_BASE, TXData);
                position = 0;
                // Enable USCI_A1 TX interrupt
                EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE,
                     EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT);
            }
            break;*/
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG:

            if(position >= packetLength) {
                EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE,
                      EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT);       // if all characters have been sent, disable IRQ
            }
            else {
                EUSCI_A_UART_transmitData(EUSCI_A1_BASE, receivedPacket[position]);  // inc index and send next character in buffer
                position++;
            }
            //UCA1IFG &= ~UCTXCPTIFG;         // clear flag
            break;
    }
}

//-- Timer ISR to for timeout or triggering homing signal on collar - should be unique to controller or collar ---------------------------
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void)
{
/*    if (homingON) {
        if (playedHomingLast) {
            playedHomingLast = false;   // alternate cycles, so homing signal plays every second
        }
        else {
            playedHomingLast = true;
            play_1kHz_pulse(100);
        }
    }
    cycles_since_last_transmission++;
    if (automation) {
        if (cycles_since_last_transmission > 4) { // Timeout 2 seconds
            play_2kHz_pulse(200);
        }
    }*/
}
