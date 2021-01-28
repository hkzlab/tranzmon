#include "modem.h"

#define MODEM_PORT_ADDR 0x40
#define MODEM_PORT_READ 0x50
#define MODEM_PORT_WRITE 0x60

#define REG_CR0  0x00
#define REG_CR1  0x01
#define REG_DR   0x02
#define REG_TR   0x03
#define REG_ID   0x06 

// To read from a modem register, first write the register address into port 0x40
// Then read the status from port 0x50 or write the value into port 0x60

