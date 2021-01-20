#include "ctc.h"

#include <utilities.h>

#define CTC_BASE   0x10

static __sfr __at (CTC_BASE+0x00) CTC_Chan0;
static __sfr __at (CTC_BASE+0x01) CTC_Chan1;
static __sfr __at (CTC_BASE+0x02) CTC_Chan2;
static __sfr __at (CTC_BASE+0x03) CTC_Chan3;


void spkr_init(void) {
    // Taken from Tranz 330 Library routines
    CTC_Chan2 = 0x03; // Disable interrupts, timer mode, prescale=16, auto-trigger, no time constant
}

void spkr_beep(uint8_t time_const) {
    CTC_Chan2 = 0x07;
    CTC_Chan2 = time_const;
    
    delay_ms(150);
    
    CTC_Chan2 = 0x03;
}

