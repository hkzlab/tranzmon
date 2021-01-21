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

void clk_ser_init(void) {
    // Initialize clock for serial 1
    CTC_Chan0 = 0x45; // Control word, continued operation, followed by time constant, automatic trigger, falling edge, prescaler of 16, counter mode, no interrupts
    CTC_Chan0 = 0x03; // Time constant of 3
    
    // The same for the second serial port
    CTC_Chan1 = 0x45;
    CTC_Chan1 = 0x03;
}

