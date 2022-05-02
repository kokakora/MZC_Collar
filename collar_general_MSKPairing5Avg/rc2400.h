/*
 * rc2400.h
 *
 *  Created on: Dec 8, 2021
 *      Author: emtlo
 */

#ifndef RC2400_H_
#define RC2400_H_

#define CMD_START           0xFE
#define CMD_TO_TRANS        0x26
#define CMD_START_RQST      0x00 //command to start the network, 00 data length
#define CMD_APP_REG         0x0A //command to set the app register.. ie what commands it cares about???
#define CMD_CONFIG          0x05 //command to write configurations.
#define CMD_JOIN_ALWAYS     0x08 //command to allow joining
#define CMD_DEV_ID          0x06 //command local address
#define CMD_FIND_DEV        0x07 //command find devices (?)
#define CMD_SEND_DATA       0x03 //command to attempt to send data

#define CMD_FROM_TRANS      0x46
#define RCV_SEND_CONF       0x83 //receive send data confirmation
#define RCV_DATA_PCK        0x87 //receive data on trans..

#define ACK_REQ             0x01
#define RADIUS              0x01

#define CMD_SETPNT          0x21
#define CMD_TX_PWR          0x0F

#define CMD0_NV_LOGIC_TYPE  0x00
#define CMD1_NV_LOGIC_TYPE  0x87
#define NV_LOGIC_LEN        0x01
#define ENDPOINT            0x02
#define ROUTER              0x01
#define COORDINATOR         0x00

#define CMD0_PANID        0x00
#define CMD1_PANID        0x83
#define PANID_LEN         0x02
#define PANID_DFLT0       0xAA //0xFF
#define PANID_DFLT1       0x55 //0xFF

#define CMD0_CHANLIST       0x00
#define CMD1_CHANLIST       0x84
#define CHANLIST_LEN        0x04
#define CHANLIST0      0x00
#define CHANLIST1      0x08
#define CHANLIST2      0x00
#define CHANLIST3      0x00


//power settings
#define dBm20       0xED
#define dBm19       0xEE
#define dBm18       0xEF
#define dBm17       0xF0
#define dBm15       0xF1
#define dBm14       0xF2
#define dBm13       0xF3
#define dBm11       0xF5
#define dBm9        0xF6
#define dBm7        0xF9
#define dBm5        0xFC
#define dBm3        0xFE
#define dBm1        0xFF

//app reg defines
#define APPENDPNT   0x01
#define APPPROFID1  0xFF
#define APPPROFID2  0x1F
#define COLL2DEVID1 0x04
#define COLL2DEVID2 0x00
#define DEVID1      0x02
#define DEVID2      0x00
#define CODEVID     0x01
#define CODEVID1    0x00
#define DEVVER      0x01
#define UNUSED      0x00

#define INCMDNUM    0x00
#define OUTCMDNUM   0x0A
#define CMD1        0x00
#define ZONE1PING   0x01
#define ZONE2PING   0x02
#define ZONE3PING   0x03
#define ZONE4PING   0x04
#define ZONE1SET    0x11
#define ZONE2SET    0x22
#define ZONE3SET    0x33
#define ZONE4SET    0x44
#define ZONESOFF    0xFF
#define CHIRPTEST   0xCC
#define COL1PING1   0x11
#define COL1PING2   0x12
#define COL1PING3   0x13
#define COL1PING4   0x14
#define COL2PING1   0x21
#define COL2PING2   0x22
#define COL2PING3   0x23
#define COL2PING4   0x24

void rc2400_config();
void rc2400_set_output_power(uint8_t power);
void rc2400_zone4();
void rc2400_zone3();
void rc2400_zone2();
void rc2400_zone1();
void rc2400_sendData(uint8_t dest, uint8_t cmd, uint8_t handle, uint8_t datalen, uint8_t * data);


#endif /* RC2400_H_ */
