// Access modes for the configuration and multi-byte registers. Add/or them to a register name to set the appropriate bits.
#define WRITE_BYTE      0x00
#define WRITE_BURST     0x40
#define READ_BYTE       0x80
#define READ_BURST      0xC0

// Configuration registers.
#define IOCFG2          0x00    /* pg61,53  GDO2 output pin config */
#define IOCFG1          0x01    /* pg61,53  GDO1 output pin config */
#define IOCFG0          0x02    /* pg61,53  GDO0 output pin config */
#define FIFOTHR         0x03    /* pg62     RX FIFO and TX FIFO thresholds */
#define SYNC1           0x04    /* pg62     sync word, high byte */
#define SYNC0           0x05    /* pg62     sync word, low byte */
#define PKTLEN          0x06    /* pg62     packet length */
#define PKTCTRL1        0x07    /* pg63     packet automation control */
#define PKTCTRL0        0x08    /* pg64     packet automation control */
#define ADDR            0x09    /* pg64     device address */
#define CHANNR          0x0A    /* pg64     channel number */
#define FSCTRL1         0x0B    /* pg65     frequency synthesizer control */
#define FSCTRL0         0x0C    /* pg65     frequency synthesizer control */
#define FREQ2           0x0D    /* pg65     frequency control word, high byte */
#define FREQ1           0x0E    /* pg65     frequency control word, middle byte */
#define FREQ0           0x0F    /* pg65     frequency control word, low byte */
#define MDMCFG4         0x10    /* pg66     modem configuration */
#define MDMCFG3         0x11    /* pg66     modem configuration */
#define MDMCFG2         0x12    /* pg67     modem configuration */
#define MDMCFG1         0x13    /* pg68     modem configuration */
#define MDMCFG0         0x14    /* pg68     modem configuration */
#define DEVIATN         0x15    /* pg69     modem deviation setting */
#define MCSM2           0x16    /* pg70     main radio control state machine config */
#define MCSM1           0x17    /* pg71     main radio control state machine config */
#define MCSM0           0x18    /* pg72     main radio control state machine config */
#define FOCCFG          0x19    /* pg73     frequency offset compensation config */
#define BSCFG           0x1A    /* pg74     bit synchronization config */
#define AGCCTRL2        0x1B    /* pg75     agc control */
#define AGCCTRL1        0x1C    /* pg76     agc control */
#define AGCCTRL0        0x1D    /* pg77     agc control */
#define WOREVT1         0x1E    /* pg77     event0 timeout, high byte */
#define WOREVT0         0x1F    /* pg78     event0 timeout, low byte */
#define WORCTRL         0x20    /* pg78     wake on radio control */
#define FREND1          0x21    /* pg78     front end rx config */
#define FREND0          0x22    /* pg79     front end tx config */
#define FSCAL3          0x23    /* pg79     frequency synthesizer calibration */
#define FSCAL2          0x24    /* pg79     frequency synthesizer calibration */
#define FSCAL1          0x25    /* pg80     frequency synthesizer calibration */
#define FSCAL0          0x26    /* pg80     frequency synthesizer calibration */
#define RCCTRL1         0x27    /* pg80     rc oscillator config */
#define RCCTRL0         0x28    /* pg80     rc oscillator config */
#define FSTEST          0x29    /* pg80     frequency synthesizer calibration control */
#define PTEST           0x2A    /* pg80     production test */
#define AGCTEST         0x2B    /* pg81     agc test */
#define TEST2           0x2C    /* pg82     various test settings */
#define TEST1           0x2D    /* pg82     various test settings */
#define TEST0           0x2E    /* pg82     various test settings */

// Multi-byte registers.
#define PATABLE         0x3E    /* pg46     output power setting */
#define FIFO            0x3F    /* pg43     read the RX FIFO or write the TX FIFO one byte at a time */

// Command strobes. Will be written as individual bytes (not bursts)
#define SRES            (0x30 + WRITE_BYTE)     /* pg57     reset chip */
#define SFSTXON         (0x31 + WRITE_BYTE)     /* pg57     if MCSM0.FS_AUTOCAL=1: enable and calibrate frequency synthesizer
                                                            if RX with CCA: go to wait state (only synth. running) for quick TX/RX turnaround */
#define SXOFF           (0x32 + WRITE_BYTE)     /* pg57     turn off crystal oscillator */
#define SCAL            (0x33 + WRITE_BYTE)     /* pg57     calibrate frequency synthesizer and turn it off */
#define SRX             (0x34 + WRITE_BYTE)     /* pg57     enable RX. will also perform calibration if coming from idle and MCSM0.FS_AUTOCAL=1 */
#define STX             (0x35 + WRITE_BYTE)     /* pg57     enable TX. will also perform calibration if coming from idle and MCSM0.FS_AUTOCAL=1
                                                            if RX with CCA: only go to TX mode if channel clear */
#define SIDLE           (0x36 + WRITE_BYTE)     /* pg57     exit TX/RX mode. turns off frequency synthesizer and exits WOR mode if applicable. */
#define SWOR            (0x38 + WRITE_BYTE)     /* pg57     start automatic RX polling if WORCTRL.RC_PD=1 */
#define SPWD            (0x39 + WRITE_BYTE)     /* pg57     enter power down mode when CS goes high */
#define SFRX            (0x3A + WRITE_BYTE)     /* pg57     flush the RX FIFO. only use when in IDLE or RXFIFO_OVERFLOW states */
#define SFTX            (0x3B + WRITE_BYTE)     /* pg57     flush the TX FIFO. only use when in IDLE or TXFIFO_UNDERFLOW states */
#define SWORRST         (0x3C + WRITE_BYTE)     /* pg57     reset RTC to Event1 value */
#define SNOP            (0x3D + WRITE_BYTE)     /* pg57     no operation */

// Status registers. Will be read as individual bytes (NOT actually a burst as listed below.)
#define PARTNUM         (0x30 + READ_BURST)     /* pg81     chip part number, = 0x80 */
#define VERSION         (0x31 + READ_BURST)     /* pg81     chip version number, = 0x03 */
#define FREQEST         (0x32 + READ_BURST)     /* pg81     frequency offset estimate from demodulator */
#define LQI             (0x33 + READ_BURST)     /* pg82     link quality estimate from demodulator */
#define RSSI            (0x34 + READ_BURST)     /* pg82     received signal strength indication */
#define MARCSTATE       (0x35 + READ_BURST)     /* pg82     main radio control state machine state */
#define WORTIME1        (0x36 + READ_BURST)     /* pg83     WOR time, high byte */
#define WORTIME0        (0x37 + READ_BURST)     /* pg83     WOR time, low byte */
#define PKTSTATUS       (0x38 + READ_BURST)     /* pg83     current GDOx status and packet status */
#define VCO_VC_DAC      (0x39 + READ_BURST)     /* pg83     current setting from PLL calibration module */
#define TXBYTES         (0x3A + READ_BURST)     /* pg83     underflow and number of bytes in TX FIFO */
#define RXBYTES         (0x3B + READ_BURST)     /* pg84     overflow and number of bytes in RX FIFO */
#define RCCTRL1_STATUS  (0x3C + READ_BURST)     /* pg85     last rc oscillator calibration result */
#define RCCTRL0_STATUS  (0x3D + READ_BURST)     /* pg85     last rc oscillator calibration result */

// PA Table values
#define PA_1DBM         0xFF                                // 1 dBm
#define PA_0DBM         0xFE                                // 0 dBm
#define PA_MINUS2DBM    0xBB                                // -2 dBm
#define PA_MINUS4DBM    0xA9                                // -4 dBm
#define PA_MINUS6DBM    0x7F                                // -6 dBm
#define PA_MINUS8DBM    0x6E                                // -8 dBm
#define PA_MINUS10DBM   0x97                                // -10 dBm
#define PA_MINUS12DBM   0xC6                                // -12 dBm
#define PA_MINUS14DBM   0x8D                                // -14 dBm
#define PA_MINUS16DBM   0x55                                // -16 dBm
#define PA_MINUS18DBM   0x93                                // -18 dBm
#define PA_MINUS20DBM   0x46                                // -20 dBm
#define PA_MINUS22DBM   0x81                                // -22 dBm
#define PA_MINUS24DBM   0x84                                // -24 dBm
#define PA_MINUS26DBM   0xC0                                // -26 dBm
#define PA_MINUS28DBM   0x44                                // -28 dBm
#define PA_MINUS30DBM   0x50                                // -30 dBm
#define PA_MINUS55DBM   0x00                                // -55 dBm
