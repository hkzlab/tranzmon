#include "dart.h"

#define DART_BASE   0x20

static __sfr __at (DART_BASE+0x00) DART_SerA;
static __sfr __at (DART_BASE+0x01) DART_CtrlA;
static __sfr __at (DART_BASE+0x02) DART_SerB;
static __sfr __at (DART_BASE+0x03) DART_CtrlB;


void dart_init(void) {
    DART_CtrlA = 
}
