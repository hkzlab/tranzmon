#include "dart.h"

#define DART_BASE   0x20

static __sfr __at (DART_BASE+0x00) DART_SerA;
static __sfr __at (DART_BASE+0x01) DART_CtrlA;
static __sfr __at (DART_BASE+0x02) DART_SerB;
static __sfr __at (DART_BASE+0x03) DART_CtrlB;


void dart_init(void) {
    DART_CtrlB = 0x18; // Channel reset
    DART_CtrlB = 0x01; DART_CtrlB = 0x18; // Interrupt on every received character
    DART_CtrlB = 0x02; DART_CtrlB = 0x12; // Set interrupt vector at 0x12
    DART_CtrlB = 0x03; DART_CtrlB = 0xC1; // WR3, RX enable, 8bits per char (RX)
    DART_CtrlB = 0x04; DART_CtrlB = 0x44; // 1 stop bit, X16 clock mode
    DART_CtrlB = 0x05; DART_CtrlB = 0x68; // TX enable, 8bits per char (TX)
}
