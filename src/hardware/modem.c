#include "modem.h"

#define MODEM_PORT_ADDR 0x40
#define MODEM_PORT_READ 0x50
#define MODEM_PORT_WRITE 0x60

// To read from a modem register, first write the register address into port 0x40
// Then read the status from port 0x50 or write the value into port 0x60

