#include "modem.h"

#define MODEM_PORT_ADDR 0x40
#define MODEM_PORT_READ 0x50
#define MODEM_PORT_WRITE 0x60

#define REG_CR0  0x00
#define REG_CR1  0x01
#define REG_DR   0x02
#define REG_TR   0x03
#define REG_ID   0x06 

static __sfr __at (MODEM_PORT_ADDR) MODEM_RegSel;
static __sfr __at (MODEM_PORT_READ) MODEM_RegRead;
static __sfr __at (MODEM_PORT_WRITE) MODEM_RegWrite;

static uint8_t modem_readReg(uint8_t reg);
static void modem_writeReg(uint8_t reg, uint8_t val);

void modem_init(void) {
    modem_writeReg(REG_CR0, 0x1A); // TX enabled, 10-bit/char (8-n-1 + start), 1200bps
    modem_writeReg(REG_CR1, 0x20); // Enable interrupt pin
    modem_writeReg(REG_TR,  0x00); // Disable all tones for now
}

// To read from a modem register, first write the register address into port 0x40
// Then read the status from port 0x50 or write the value into port 0x60

static uint8_t modem_readReg(uint8_t reg) {
    MODEM_RegSel = reg;
    return MODEM_RegRead;
}

static void modem_writeReg(uint8_t reg, uint8_t val) {
    MODEM_RegSel = reg;
    MODEM_RegWrite = val;
}

