/*
 * rc2400.h
 *
 *  Created on: Dec 8, 2021
 *      Author: emtlo
 */

#ifndef RC2400_H_
#define RC2400_H_
//----------------------------DEVICE Data-------------------------------//
#define CMD_TO_TRANS        0x26
#define CMD_START_RQST      0x00 //command to start the network, 00 data length
#define CMD_APP_REG         0x0A //command to set the app register.. ie what commands it cares about???
#define CMD_CONFIG          0x05 //command to write configurations.
#define CMD_JOIN_ALWAYS     0x08 //command to allow joining
#define CMD_BIND_DEV        0x01 //command to bind to a device (tx should bind to collars) //IEEE address or NULL for last 8 bytes
#define CMD_ALLOW_BIND      0x02 //command to allow a binding to happen (collars need to enable this)

#define CMD_DEV_ID          0x06 //command local address
#define CMD_FIND_DEV        0x07 //command find devices need to know IEEE address
#define CMD_SEND_DATA       0x03 //command to attempt to send data

#define CMD_RSP_TRANS       0x66
#define CMD_FROM_TRANS      0x46
#define RCV_SEND_CONF       0x83 //receive send data confirmation
#define RCV_DATA_PCK        0x87 //receive data on trans..
#define RCV_START_CONF      0x80 //receive network started.
#define RCV_BIND_CONF       0x81 //receive bind confirm
#define RCV_FIND_CONF       0x85 //receive found device returns the short address of the device

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
#define CHANLIST1      0x00
#define CHANLIST2      0x00
#define CHANLIST3      0x04

//status defines
#define SUCCESS     0x00
#define FAILURE     0x01
#define NOACK       0xE9

//power settings n denoted negative dbm
#define dBm3       0xED
#define dBm1       0xEE
#define dBmn1      0xEF
#define dBmn2      0xF0
#define dBmn4      0xF1
#define dBmn5       0xF2
#define dBmn6       0xF3
#define dBmn8       0xF5
#define dBmn10      0xF6
#define dBmn12      0xF9
#define dBmn14      0xFC
#define dBmn16        0xFE
#define dBmn18        0xFF

//-------------------------------USER SPECIFIC data--------------------//
//app reg defines
#define APPENDPNT   0x01
#define APPPROFID1  0xFF
#define APPPROFID2  0x1F
#define COLL2DEVID1 0x04
#define COLL2DEVID2 0x00
#define DEVID1      176
#define DEVID2      82
#define CODEVID     0x01
#define CODEVID1    0x00 //todo find collar 2 short address
#define DEVVER      0x01
#define UNUSED      0x00

#define INCMDNUM    0x00
#define OUTCMDNUM   0x0A
//COMMANDS
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
//HANDLES COLLAR 1
#define COL1PING1   0x11
#define COL1PING2   0x12
#define COL1PING3   0x13
#define COL1PING4   0x14
#define COL1SET1    0x51
#define COL1SET2    0x52
#define COL1SET3    0x53
#define COL1SET4    0x54
#define COL1OFF     0x1F
#define COL1CHIRP   0x1C
//HANDLES COLLAR 2
#define COL2PING1   0x21
#define COL2PING2   0x22
#define COL2PING3   0x23
#define COL2PING4   0x24
#define COL2SET1    0x61
#define COL2SET2    0x62
#define COL2SET3    0x63
#define COL2SET4    0x64
#define COL2OFF     0x2F
#define COL2CHIRP   0x2C
//HANDLE MASKS
#define COLL1HANDLEMASK 0x10
#define COLL2HANDLEMASK 0x20
#define SETZONEMASK     0x40
#define ZONE1MASK       0x01
#define ZONE2MASK       0x02
#define ZONE3MASK       0x03
#define ZONE4MASK       0x04
#define ZONEMASK        0x0F
#define ZONEOFFMASK     0x0F
#define MANUALMASK      0x0C

void rc2400_config();
void rc2400_set_output_power(uint8_t power);
void rc2400_zone4();
void rc2400_zone3();
void rc2400_zone2();
void rc2400_zone1();
void rc2400_sendData(uint8_t dest, uint8_t cmd, uint8_t handle, uint8_t datalen, uint8_t * data);


#endif /* RC2400_H_ */
