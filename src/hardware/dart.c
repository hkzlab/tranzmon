#include "dart.h"

#define DART_BASE   0x20

static __sfr __at (DART_BASE+0x00) DART_SerA;
static __sfr __at (DART_BASE+0x01) DART_CtrlA;
static __sfr __at (DART_BASE+0x02) DART_SerB;
static __sfr __at (DART_BASE+0x03) DART_CtrlB;

void dart_isr(void) __interrupt(0x12);

void dart_init(void) {
    // Initialize channel B
    DART_CtrlB = 0x18; // Channel reset
    // Keep the interrupts disabled for now
    //DART_CtrlB = 0x01; DART_CtrlB = 0x18; // Interrupt on every received character
    //DART_CtrlB = 0x02; DART_CtrlB = 0x12; // Set interrupt vector at 0x12
    DART_CtrlB = 0x03; DART_CtrlB = 0xC1; // WR3, RX enable, 8bits per char (RX)
    DART_CtrlB = 0x04; DART_CtrlB = 0x44; // 1 stop bit, X16 clock mode
    DART_CtrlB = 0x05; DART_CtrlB = 0x68; // TX enable, 8bits per char (TX)
}

uint8_t dart_read(DART_Port port) {
    switch(port) {
        case PORT_A:
            while(!(DART_CtrlA & 0x01)) __asm nop __endasm;
            return DART_SerA;
        default:
            while(!(DART_CtrlB & 0x01)) __asm nop __endasm;
            return DART_SerB;
    }
}

void dart_write(DART_Port port, uint8_t data) {
    switch(port) {
        case PORT_A:
            while(!(DART_CtrlA & 0x04)) __asm nop __endasm;
            DART_SerA = data;
            break;
        default:
            while(!(DART_CtrlB & 0x04)) __asm nop __endasm;
            DART_SerB = data;
            break;
    }
}

/***/

void dart_isr(void) __interrupt(0x12) {}

