/*
 * rc2400.c
 *
 *  Created on: Apr 6, 2022
 *      Author: emtlo
 */
#include "gpio_isr.h"
#include "driverlib.h"
#include "Board.h"
#include "setup_common.h"
#include "rc2400.h"

//only ONE of these three defines should be uncommented.
//#define TRANSMITTER_CODE
#define COLLAR1
//#define COLLAR2

#define MODE 0x03
#define FREQ_2405 0x0B
#define BROADCAST_MSG 0xFF
#define ACK 0x01
#define RADIUS 0x01
#define HANDLE 0x05A
//config constants
const uint8_t rc_blank_data[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//const uint8_t rc_network_config[6] =  {0x03, CMD_TO_TRANS, CMD_CONFIG,  0x03, 0x01, 0x00}; //keeps last config and state
const uint8_t rc_network_config[6] =  {0x03, CMD_TO_TRANS, CMD_CONFIG,  0x03, 0x01, 0x03}; //resets last config and state
const uint8_t rc_endpoint[6]    =     {0x03, CMD_TO_TRANS, CMD_CONFIG,  CMD1_NV_LOGIC_TYPE, NV_LOGIC_LEN, ENDPOINT};
const uint8_t rc_router[6]      =     {0x03, CMD_TO_TRANS, CMD_CONFIG,  CMD1_NV_LOGIC_TYPE, NV_LOGIC_LEN, ROUTER};
const uint8_t rc_coordinator[6]    =  {0x03, CMD_TO_TRANS, CMD_CONFIG,  CMD1_NV_LOGIC_TYPE, NV_LOGIC_LEN, COORDINATOR};
const uint8_t rc_panid_config[7]  =   {0x04, CMD_TO_TRANS, CMD_CONFIG,  CMD1_PANID, PANID_LEN, PANID_DFLT0,PANID_DFLT1};
const uint8_t rc_chanlist_config[9]=  {0x06, CMD_TO_TRANS, CMD_CONFIG,  CMD1_CHANLIST,CHANLIST_LEN,CHANLIST0,CHANLIST1,CHANLIST2,CHANLIST3};

//const uint8_t rc_encryp_key[22] = {0x12, 0x26, 0x05, 0x62, 0x10,
//        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
//        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x43};
//app reg - len 0x26 0x0A AppEndPoint(same for all devices set from 1-240), AppProfileID(set same all devices, 2 bytes assigned by ZigBee Alliance)
//          DeviceId (2bytes, app spec ID, device type in pro id) DeviceVersion, Unused, InputCmdNum, InputCmdList, OutputCmdNum, OutputCmdList)
#ifdef TRANSMITTER_CODE
const uint8_t rc_app_reg[32]    = {0x1D, CMD_TO_TRANS, CMD_APP_REG,
                                   APPENDPNT, APPPROFID1, APPPROFID2, CODEVID, CODEVID1,
                                   DEVVER, UNUSED, INCMDNUM, OUTCMDNUM,
                                   CMD1, ZONE1PING, CMD1, ZONE2PING, CMD1, ZONE3PING, CMD1, ZONE4PING,
                                   CMD1, ZONE1SET,  CMD1, ZONE2SET,  CMD1, ZONE3SET,  CMD1, ZONE4SET,
                                   CMD1, ZONESOFF,  CMD1, CHIRPTEST};  //documentation at 4.3.1 pg 40 https://radiocrafts.com/uploads/CC2530ZNP%20Interface%20Specification.pdf
#endif
#ifdef COLLAR1
const uint8_t rc_app_reg[32]    = {0x1D, CMD_TO_TRANS, CMD_APP_REG,
                                   APPENDPNT, APPPROFID1, APPPROFID2, DEVID1, DEVID2,
                                   DEVVER, UNUSED, OUTCMDNUM, CMD1, ZONE1PING, CMD1, ZONE2PING, CMD1, ZONE3PING, CMD1, ZONE4PING,
                                   CMD1, ZONE1SET,  CMD1, ZONE2SET,  CMD1, ZONE3SET,  CMD1, ZONE4SET,
                                   CMD1, ZONESOFF,  CMD1, CHIRPTEST, INCMDNUM};
#endif
#ifdef COLLAR2
const uint8_t rc_app_reg[32]    = {0x1D, CMD_TO_TRANS, CMD_APP_REG,
                                   APPENDPNT, APPPROFID1, APPPROFID2, COLL2DEVID1, COLL2DEVID2,
                                   DEVVER, UNUSED, OUTCMDNUM, CMD1, ZONE1PING, CMD1, ZONE2PING, CMD1, ZONE3PING, CMD1, ZONE4PING,
                                   CMD1, ZONE1SET,  CMD1, ZONE2SET,  CMD1, ZONE3SET,  CMD1, ZONE4SET,
                                   CMD1, ZONESOFF,  CMD1, CHIRPTEST, INCMDNUM};
#endif
const uint8_t rc_start_rqst[3] = {0x00, 0x26, 0x00}; //this starts the network at the transmitter, or attempts to join at the collar

const uint8_t rc_allow_join[6] = {0x03, CMD_TO_TRANS, CMD_JOIN_ALWAYS, 0xFC, 0xFF, 0xFF}; //allow always join (either collar)
const uint8_t rc_allow_join2sec[6] = {0x03, CMD_TO_TRANS, CMD_JOIN_ALWAYS, 0xFC, 0xFF, 0x02}; // allow join for 2 seconds (either collar)
const uint8_t rc_allow_C1join[6] = {0x03, CMD_TO_TRANS, CMD_JOIN_ALWAYS, DEVID1, DEVID2, 0x02}; // allow collar 1 join for 2 seconds
const uint8_t rc_allow_C2join[6] = {0x03, CMD_TO_TRANS, CMD_JOIN_ALWAYS, COLL2DEVID1, COLL2DEVID2, 0x02}; // allow collar 2 join for 2 seconds

const uint8_t rc_device_ieeeaddr[4] = {0x01, CMD_TO_TRANS, CMD_DEV_ID, 0x01}; //request device ieee 8 byte data
const uint8_t rc_device_shortaddr[4] ={0x01, CMD_TO_TRANS, CMD_DEV_ID, 0x02}; //request device short address 2 byte data (srsp is always 9 bytes data)

const uint8_t rc_zone1_msg[14] = {0x0A, CMD_TO_TRANS, CMD_SEND_DATA, BROADCAST_MSG,BROADCAST_MSG,
                                  CMD1, ZONE1PING, HANDLE, ACK, RADIUS, 0x02, 0x55, 0x11}; //last 3, datalen, data, data
const uint8_t rc_zone2_msg[14] = {0x0A, CMD_TO_TRANS, CMD_SEND_DATA, BROADCAST_MSG,BROADCAST_MSG,
                                  CMD1, ZONE2PING, HANDLE, ACK, RADIUS, 0x02, 0x55, 0x22}; //last 3, datalen, data, data
const uint8_t rc_zone3_msg[14] = {0x0A, CMD_TO_TRANS, CMD_SEND_DATA, BROADCAST_MSG,BROADCAST_MSG,
                                  CMD1, ZONE3PING, HANDLE, ACK, RADIUS, 0x02, 0x55, 0x33}; //last 3, datalen, data, data
const uint8_t rc_zone4_msg[14] = {0x0A, CMD_TO_TRANS, CMD_SEND_DATA, BROADCAST_MSG,BROADCAST_MSG,
                                  CMD1, ZONE4PING, HANDLE, ACK, RADIUS, 0x02, 0x55, 0x44}; //last 3, datalen, data, data
const uint8_t rc_receive_data[] = {0x08, CMD_FROM_TRANS, RCV_DATA_PCK, 0x00, 0x00};


const uint8_t rc_test_mode[11] = {0x08, CMD_SETPNT, 0x09, 0x07, 0x0F, 0x00, 0x04, MODE, FREQ_2405,
                                dBm20,0};
const uint8_t rc_reset[4] = {0x01, 0x41, 0x00, 0x00};

const uint8_t network_created_response[] = {0x01, 0x45, 0xC0, 0x09};

volatile int zone1_col1_ack = 0;
volatile int zone2_col1_ack = 0;
volatile int zone3_col1_ack = 0;
volatile int zone4_col1_ack = 0;
volatile int zone1_col2_ack = 0;
volatile int zone2_col2_ack = 0;
volatile int zone3_col2_ack = 0;
volatile int zone4_col2_ack = 0;


void rc2400_send_one(uint8_t value)
{
    //assume CS already set
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
                       EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;       // wait for empty TX buffer
        EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, value);             // send value

}
void rc2400_send_mult(uint8_t len, uint8_t * array)
{
    int i;
    for(i = 0; i < len; i++)
    {
        rc2400_send_one(array[i]);
    }
    while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
}

void rc2400_send_blank(uint8_t len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        rc2400_send_one(0x00);
    }
    while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
}
void areq_handler(void)
{
    int values[128] = {0};
    uint8_t status = 1;
    int len = 0;
    int i = 0;
    __disable_interrupt();
    __delay_cycles(480);
    if((P3IN & GPIO_PIN6) != GPIO_PIN6)
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
        while((P3IN & GPIO_PIN6) != GPIO_PIN6) //wait for SLV Ready RX
        {
            rc2400_send_blank(1); //send blank data for mst side of it....
            while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
            len = (int)EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
            __delay_cycles(200);
        }
        rc2400_send_one(0x00);
        while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
        len = (int)EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
        values[i] = len;
        len = len +3;
        for(i = 1; i < len; i++)
        {
            rc2400_send_one(0x00);
            while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
            status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
            values[i] = (int)status;
        }
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
        if((values[2] == RCV_SEND_CONF)&&(values[4] == 0x00))//data send confirm
        {
        switch(values[3])
        {
        case COL1PING1:
            zone1_col1_ack = 1;
            break;
        case COL1PING2:
            zone2_col1_ack = 1;
            break;
        case COL1PING3:
            zone3_col1_ack = 1;
            break;
        case COL1PING4:
            zone4_col1_ack = 1;
            break;
        case COL2PING1:
            zone1_col2_ack = 1;
            break;
        case COL2PING2:
            zone2_col2_ack = 1;
            break;
        case COL2PING3:
            zone3_col2_ack = 1;
            break;
        case COL2PING4:
            zone4_col2_ack = 1;
            break;
        }
        }
        if((values[2] == RCV_DATA_PCK))//recieve data packet
        {
            play_2kHz_pulse(200);
        }
    }
    GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN6);
    __enable_interrupt();
}
uint8_t rc2400_readstatus_write_config(uint8_t len, int *values)
{
    int i;
    uint8_t status = 01;
    for(i = 0; i < len; i++)
    {
        rc2400_send_one(0x00);
        while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
        status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
        values[i] = (int)status;
    }
    return status;
}
uint8_t rc2400_read_deviceinfo(int *values)
{
    int i;
    uint8_t status = 01, len = 9+3;
    for(i = 0; i < len; i++)
    {
        rc2400_send_one(0x00);
        while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
        status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
        values[i] = (int)status;
    }
    return status;
}
/**
 * receive the system reset indicator.
 */
void rc2400_recieve_sys_reset(void)
{

    while((P3IN & GPIO_PIN6) == GPIO_PIN6); //wait for SLV Ready RX ----- THIS LINE ADDED... for reset reason transmit
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    rc2400_send_blank(8);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6); //wait for SLV Ready RX
    rc2400_send_blank(9);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
}
/**
 * set TX output power on the RC trans
 */
void rc2400_set_output_power(uint8_t power)
{
    uint8_t status;
    __disable_interrupt();
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6); //wait for SLV Ready RX ----- THIS LINE ADDED...
    rc2400_send_one(0x02);             // length
    rc2400_send_one(CMD_SETPNT);             // cmd0
    rc2400_send_one(CMD_TX_PWR);             // cmd1
    rc2400_send_one(0x00);             // send data
    rc2400_send_one( power);             // send data
    while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
    while((P3IN & GPIO_PIN6) != GPIO_PIN6); //wait for SLV Ready TX ----- THIS LINE ADDED...
    rc2400_send_one(0x00);                  //read the SRSP
    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
    rc2400_send_one(0x00);
    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
    rc2400_send_one(0x00);
    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
    rc2400_send_one(0x00);
    while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    if(status != 0) puts("power setting error");
    __enable_interrupt();
}
void rc2400_zone1()
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0); // RF SWitch 1 V2
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN5); // RF SWitch 1 V1
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0); // RF SWitch 2 V2
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1); // RF SWitch 1 V1
    rc2400_set_output_power(dBm11);
    __delay_cycles(1000000);
}
void rc2400_zone2()
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0); // RF SWitch 1 V2
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN5); // RF SWitch 1 V1
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0); // RF SWitch 2 V2
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1); // RF SWitch 1 V1
    rc2400_set_output_power(dBm11);
}
void rc2400_zone3()
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0); // RF SWitch 1 V2
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN5); // RF SWitch 1 V1
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0); // RF SWitch 2 V2
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1); // RF SWitch 1 V1
    rc2400_set_output_power(dBm3);
    __delay_cycles(1000000);
}
void rc2400_zone4()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0); // RF SWitch 1 V2
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN5); // RF SWitch 1 V1
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0); // RF SWitch 2 V2
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1); // RF SWitch 1 V1
    rc2400_set_output_power(dBm11);
    __delay_cycles(1000000);
}
void rc2400_sendData(uint8_t dest, uint8_t cmd, uint8_t handle, uint8_t datalen, uint8_t * data)
{
    uint8_t status = 1;
    __disable_interrupt();
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_one(datalen + 8);
    rc2400_send_one(CMD_TO_TRANS);
    rc2400_send_one(CMD_SEND_DATA);
    switch(dest)
    {
    case 1:
        rc2400_send_one(DEVID1);
        rc2400_send_one(DEVID2);
        break;
    case 2:
        rc2400_send_one(COLL2DEVID1);
        rc2400_send_one(COLL2DEVID2);
        break;
    default: //broadcast
        rc2400_send_one(BROADCAST_MSG);
        rc2400_send_one(BROADCAST_MSG);
        break;
    }
    rc2400_send_one(CMD1);
    rc2400_send_one(cmd);
    rc2400_send_one(handle);
    rc2400_send_one(ACK);
    rc2400_send_one(RADIUS);
    rc2400_send_one(datalen);
    if(datalen >0)
    {
        rc2400_send_mult(datalen, data);
    }
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    rc2400_send_one(0);
    while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
    rc2400_send_one(0);
    while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
    rc2400_send_one(0);
    while(EUSCI_A_SPI_isBusy(EUSCI_A0_BASE));              // Wait until SPI is done transmitting
    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    if(status == 0) puts("send issue");
    __enable_interrupt();


}
void rc2400_test1_mode_set()
{
    rc2400_send_mult(10, (uint8_t *)rc_test_mode);
    rc2400_send_mult(4, (uint8_t *)rc_reset);
}
void rc2400_config()
{
    uint8_t status = 1;
    int values[12] = {0};
    //set up interrupt for SLV RDY
    GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN6);   // Clear flag
    GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN6);
    configure_ISR_functions_PORT3_6(&areq_handler); // Only link ISR for main transceiver with receiver handler

    __delay_cycles(1000000);


    spi_setup(EUSCI_A0_BASE, EUSCI_A_SPI_CLOCKSOURCE_SMCLK, 2000000,  1000000, EUSCI_A_SPI_MSB_FIRST,
              EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
              EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW, EUSCI_A_SPI_3PIN,
              GPIO_PORT_P1, GPIO_PIN4);

    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Deactivate CSn
    __delay_cycles(1000000);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0); //reenable the radio
    __delay_cycles(1000000);

    rc2400_recieve_sys_reset();

    __disable_interrupt();
    __delay_cycles(1000000);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6); //wait for SLV Ready RX
    rc2400_send_mult(6, (uint8_t *)rc_network_config);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6); //wait for SLV Ready TX
    status = rc2400_readstatus_write_config(4, &values[0]); //recieve status
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    __delay_cycles(1000000);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
#ifdef TRANSMITTER_CODE
    rc2400_send_mult(6, (uint8_t *)rc_coordinator);
#else
    rc2400_send_mult(6, (uint8_t *)rc_router);
#endif
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_readstatus_write_config(4, &values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    __delay_cycles(1000000);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_mult(7, (uint8_t *)rc_panid_config);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_readstatus_write_config(4, &values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    __delay_cycles(1000000);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_mult(9, (uint8_t *)rc_chanlist_config);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_readstatus_write_config(4, &values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    __delay_cycles(1000000);

//    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
//    rc2400_send_one(CMD_START);             // start
//    rc2400_send_mult(22, (uint8_t *)rc_encryp_key);
//    status = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);          // read RX buffer
//    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
//    __delay_cycles(480);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_mult(sizeof(rc_app_reg), (uint8_t *)rc_app_reg);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_readstatus_write_config(4, &values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    __delay_cycles(480);
    status = 1;
#ifndef TRANSMITTER_CODE
    while(status < 6)
    {
#endif
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_mult(3, (uint8_t *)rc_start_rqst);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_readstatus_write_config(4, &values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    __delay_cycles(480);

    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX -- AREQ for network status...
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    rc2400_send_blank(8);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6); //wait for SLV Ready RX
    status = rc2400_readstatus_write_config(4,&values[0]);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS

#ifndef TRANSMITTER_CODE
    }
#endif

    __delay_cycles(480);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_mult(sizeof(rc_device_ieeeaddr),(uint8_t *)rc_device_ieeeaddr);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_read_deviceinfo(&values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS

    __delay_cycles(480);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_mult(sizeof(rc_device_shortaddr),(uint8_t *)rc_device_shortaddr);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_read_deviceinfo(&values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS


#ifdef TRANSMITTER_CODE
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);                // Enable CS
    while((P3IN & GPIO_PIN6) == GPIO_PIN6);                         //wait for SLV Ready RX
    rc2400_send_mult(6,(uint8_t *)rc_allow_join);
    while((P3IN & GPIO_PIN6) != GPIO_PIN6);                         //wait for SLV Ready TX
    status = rc2400_readstatus_write_config(4, &values[0]);          // read RX buffer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);               // Disable CS
    rc2400_zone1();
#else
    __delay_cycles(480);
    if(status == 0)
    {
        rc2400_set_output_power(dBm20);  //set output power high initially
    }
#endif

    __enable_interrupt();
}
