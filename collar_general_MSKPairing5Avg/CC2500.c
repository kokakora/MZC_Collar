// Functions for interfacing with the CC2500 2.4GHz RF module

#include <cc_spi.h>
#include "cc2500.h"

static uint8_t gdo0_port;
static uint16_t gdo0_pin;
static uint8_t packet_length;
static uint8_t rx_buffer[256];
static void (*handler)(uint8_t byte_count, uint8_t bytes[]);

/**
 * Access a configuration register on the CC2500.
 * NOT used for multi-byte registers or command strobes.
 *
 * @param reg    Register to read or write
 * @param val    Value to write
 * @returns      Status byte for writes, or value for reads
 */

// This will be called after a falling edge on the GDO0 pin, indicating that a packet has been received
// *HOWEVER* that does not mean the packet was valid (CRC test passed)
// If the CRC check fails the RX FIFO will be automatically flushed by the CC2500.
static void receiver_handler(void) {

    // Only should be used on long-range transceiver
    uint8_t byte_count = spi_write_register(RXBYTES | READ_BYTE, 0x00);

    // check for RX FIFO overflow
    if(byte_count >> 7) {
        cc2500_flush_rx_fifo();
        cc2500_enter_rx_mode();
        return;
    }
    // if the CRC check fails, then the FIFO will be flushed anyways
    if (byte_count) {   // If CRC check passes, read the register and call handler
        // receive the bytes
        uint8_t i;
        for(i = 0; i < byte_count; i++)
            rx_buffer[i] = spi_write_register(FIFO | READ_BYTE, 0x00);

        // re-enter RX mode
        // cc2500_enter_rx_mode();

        // call the user's packet handler function, skipping past byte 0 (address byte)
        handler(packet_length, &rx_buffer[1]);
    }
    // else, do nothing
}

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
 * @param gdo0                  GDO0 interrupt pin
 * @param packet_size           Number of bytes in a packet
 * @param packet_handler        Your function that will be called after successfully receiving a packet
 */
void cc2500_setup(uint16_t baseAddress, uint8_t selectClockSource, uint32_t clockSourceFrequency, uint8_t csport, uint16_t cspin, uint8_t gdo0port, uint16_t gdo0pin, uint8_t packet_size, void (*packet_handler)(uint8_t byte_count, uint8_t bytes[])) {   // initialize the RF module
	
    packet_length = packet_size;
    handler = packet_handler;
    configure_ISR_functions_PORT3_6(&receiver_handler); // link ISR with receiver handler

    gdo0_port = gdo0port;
    gdo0_pin = gdo0pin;

	spi_setup(baseAddress, selectClockSource, clockSourceFrequency, 1000000, EUSCI_A_SPI_MSB_FIRST,
	          EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
	          EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW, EUSCI_A_SPI_3PIN, csport, cspin);

	// ensure cc2500 registers contain their reset values
	// Reset process recommended in datasheet
	GPIO_setOutputHighOnPin(csport, cspin); // Deactivate CSn
    __delay_cycles(480);
    GPIO_setOutputLowOnPin(csport, cspin);  // Activate CSn
    __delay_cycles(480);
    GPIO_setOutputHighOnPin(csport, cspin); // Deactivate CSn
    __delay_cycles(720);
	spi_send_strobe(SRES);

	// waste time for reset to complete
	volatile uint32_t i;
	for(i = 0; i < 999999; i++);

	// write to the registers that need different values
	spi_write_register(IOCFG0,	0x06);	// GDO0 as interrupt: asserts on sync, deasserts on end of packet
	spi_write_register(SYNC1,	0xD0);	// Sync word: 0xD09E
	spi_write_register(SYNC0,	0x9E);	// Sync word: 0xD09E
	spi_write_register(PKTLEN,	0x06);	// Packet length: 6 bytes
	spi_write_register(PKTCTRL1,	0x0D);	// Packet control: CRC autoflush, append status bytes, strict address check
	spi_write_register(PKTCTRL0,	0x44);	// Packet control: data whitening, CRC enabled, fixed packet length mode
	//spi_write_register(PKTCTRL0,    0x04);  // Packet control: no data whitening, CRC enabled, fixed packet length mode
	spi_write_register(ADDR,		0x43);	// Address: 0x43 -- Same for all Collars
	spi_write_register(CHANNR, 0x7F);  // Channel: 127
    spi_write_register(FSCTRL1,    0x0C);  //
    //spi_write_register(baseAddress, FSCTRL1, 0x08);  //
    //spi_write_register(baseAddress, FREQ2,    0x5C);  //
    spi_write_register(FREQ2,   0x5D);  //
    //spi_write_register(baseAddress, FREQ1,    0xF6);  //
    spi_write_register(FREQ1,   0x93);  //
    //spi_write_register(baseAddress, FREQ0,    0x27);  //
    spi_write_register(FREQ0,   0xB1);  //
    spi_write_register(MDMCFG4,    0x0E);  //
    //spi_write_register(baseAddress, MDMCFG4, 0x86);  //
    spi_write_register(MDMCFG3,    0x3B);  //
    //spi_write_register(baseAddress, MDMCFG3, 0x83);  //
    spi_write_register(MDMCFG2,    0x73);  // Modem configuration: MSK, 30/32 sync bits detected
    //spi_write_register(baseAddress, MDMCFG2, 0xF3);  // Modem configuration: MSK, 30/32 sync bits detected
    //spi_write_register(baseAddress, MDMCFG2, 0x13);  // Modem configuration: GFSK, 30/32 sync bits detected
    spi_write_register(MDMCFG1,    0xC2);  // Modem configuration: FEC, 8 byte minimum preamble
    spi_write_register(DEVIATN,    0x00);  //
    //spi_write_register(baseAddress, DEVIATN, 0x44);  //
	spi_write_register(MCSM1,	0x0F);	// Radio state machine: stay in RX mode after sending a packet, stay in RX mode after receiving
    spi_write_register(MCSM0,  0x18);  //
    spi_write_register(FOCCFG, 0x1D);  //
    //spi_write_register(baseAddress, FOCCFG,  0x16);  //
    spi_write_register(BSCFG,  0x1C);  //
    spi_write_register(AGCCTRL2,   0xC7);  //
    spi_write_register(AGCCTRL0,   0xB0);  //
    spi_write_register(FREND1, 0xB6);  //
    spi_write_register(FSCAL3, 0xEA);  //
    spi_write_register(FSCAL1, 0x00);  //
    spi_write_register(FSCAL0, 0x19);  //
    //spi_write_register(baseAddress, FSCAL0,  0x11);  //
	spi_write_register(PATABLE,	0xFF);	// Output power: +1dBm (the maximum possible)

	// Enable GDO0 interrupt ** must manually declare interrupt each pin
	GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN6);   // Clear flag
	GPIO_enableInterrupt(gdo0port, gdo0pin);

}

/**
 * Gets the status byte.
 *
 * @returns    Status
 */
uint8_t cc2500_get_status(void) {

    return spi_send_strobe(SNOP);

}

/**
 * Sets the current channel.
 *
 * @param channel    Channel number
 */
void cc2500_set_channel(uint8_t channel) {

    spi_write_register(CHANNR, channel);

}

/**
 * Gets the current channel.
 *
 * @returns    Channel number
 */
uint8_t cc2500_get_channel(void) {

    return spi_write_register(CHANNR | READ_BYTE, 0x00);

}

/**
 * Reads the RSSI value of the current channel.
 * The register contains the RSSI has a signed byte,
 * this function converts it to an unsigned integer in the 0 - 239 range. 0 = min power.
 *
 * @returns    RSSI
 */
uint8_t cc2500_get_rssi(void) {

    __disable_interrupt(); // Prevent interrupt vectors from being triggered while transmitting
    int8_t rawRssi = spi_write_register(RSSI, 0x00);
    uint8_t normalized = rawRssi + 108; // (-128 to 127) -> (0 to 255) -> -20 to remove some offset
    __enable_interrupt(); // Reenable interrupt vectors after transmission
    return normalized;

}

/**
 * Flushes the receiver FIFO.
 */
void cc2500_flush_rx_fifo(void) {

    spi_send_strobe(SFRX);

}

/**
 * Enters receiver mode.
 */
void cc2500_enter_rx_mode(void) {

    spi_send_strobe(SRX);

}

/**
 * Flushes the transmit FIFO.
 */
void cc2500_flush_tx_fifo(void) {

    spi_send_strobe(SFTX);

}

/**
 * Enters transmit mode.
 */
void cc2500_enter_tx_mode(void) {

    spi_send_strobe(STX);
}

/**
 * Transmits a packet. Automatically enters tx mode.
 *
 * @param bytes    The packet.
 */
void cc2500_transmit_packet(uint8_t bytes[]) {
    __disable_interrupt(); // Prevent interrupt vectors from being triggered while transmitting

    // wait for empty TX FIFO
    while(spi_write_register(TXBYTES | READ_BYTE, 0x00) != 0);

    // send the address
    spi_write_register(FIFO, controllerAddr); // Address of controller -- unique for each controller

    // send the payload
    uint8_t i;
    for(i = 0; i < packet_length; i++)
        spi_write_register(FIFO, bytes[i]);

    cc2500_enter_tx_mode(); // Change state to TX, initiating data transfer

    // check for RX FIFO underflow
    //uint8_t status = cc2500_get_status();
    //if((status >> 4) == 7) {
    //    cc2500_flush_tx_fifo();
    //    cc2500_enter_tx_mode();
    //}

    while (!GPIO_getInputPinValue(gdo0_port, gdo0_pin)); // Wait for GDO0 to go high -> sync TX'ed
    cc2500_get_status();

    while (!GPIO_getInterruptStatus(gdo0_port, gdo0_pin));
    GPIO_clearInterrupt(gdo0_port, gdo0_pin);
    __enable_interrupt(); // Reenable interrupt vectors after transmission

}
/*
#pragma vector = PORT3_VECTOR
__interrupt void P3_6_ISR(void)
{
    // Interrupt for indicating when radio has received a packet
    // Disable when transmitting over SPI
    if (GPIO_getInterruptStatus(GPIO_PORT_P3, GPIO_PIN6)) {
    receiver_handler();
        __delay_cycles(100);
    }
    GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN6);
}*/
