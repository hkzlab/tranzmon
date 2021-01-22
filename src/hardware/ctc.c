#include "ctc.h"

#include <utilities.h>

#define CTC_BASE   0x10

static __sfr __at (CTC_BASE+0x00) CTC_Chan0;
static __sfr __at (CTC_BASE+0x01) CTC_Chan1;
static __sfr __at (CTC_BASE+0x02) CTC_Chan2;
static __sfr __at (CTC_BASE+0x03) CTC_Chan3;

void ctc_isr_0(void) __interrupt(0x18);
void ctc_isr_1(void) __interrupt(0x1A);
void ctc_isr_2(void) __interrupt(0x1C);
void ctc_isr_3(void) __interrupt(0x1E);

static volatile uint16_t isr_time_counter;
volatile uint16_t msec_counter = 0;

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
    //CLOCK = 3.57964Mhz
    // CLOCK/2 = 1,78982Mhz 
    // 38400bps -> we need at least x16 --> 614400Hz
    // 1789820 / 614400 = 2.91 = ~3


    isr_time_counter = 0;

    // Program the interrupt vector
    CTC_Chan0 = 0x18;

    // Initialize clock for serial 1
    CTC_Chan0 = 0xC5; // Control word, interrupt, continued operation, followed by time constant, automatic trigger, falling edge, prescaler of 16, counter mode, no interrupts
    CTC_Chan0 = 0x03; // Time constant of 3
    
    // The same for the second serial port
    CTC_Chan1 = 0x45;
    CTC_Chan1 = 0x03;
}

void ctc_isr_0(void) __interrupt(0x18) {
    isr_time_counter++;
    
    if(isr_time_counter == 614) { // We're doing 614.400 impulses per second
        isr_time_counter = 0;
        msec_counter++;
    }
}

void ctc_isr_1(void) __interrupt(0x1A) {}
void ctc_isr_2(void) __interrupt(0x1C) {}
void ctc_isr_3(void) __interrupt(0x1E) {}

