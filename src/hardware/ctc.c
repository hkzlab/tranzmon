#include "ctc.h"
#include <utilities.h>
#include "dart.h"
#define CTC_BASE   0x10

static __sfr __at (CTC_BASE+0x00) CTC_Chan0;
static __sfr __at (CTC_BASE+0x01) CTC_Chan1;
static __sfr __at (CTC_BASE+0x02) CTC_Chan2;
static __sfr __at (CTC_BASE+0x03) CTC_Chan3;

void ctc_isr_0(void) __interrupt(0x18);
void ctc_isr_1(void) __interrupt(0x1A);
void ctc_isr_2(void) __interrupt(0x1C);
void ctc_isr_3(void) __interrupt(0x1E);

static volatile uint32_t tick_counter = 0;

void spkr_init(void) {
    // Taken from Tranz 330 Library routines
    CTC_Chan2 = 0x03; // Disable interrupts, timer mode, prescale=16, auto-trigger, no time constant
}

void spkr_beep(uint8_t time_const) {
    CTC_Chan2 = 0x07; // Control, software reset, time constant follows
    CTC_Chan2 = time_const;
    
    delay_ms(150);
    
    CTC_Chan2 = 0x03;
}

void tick_init(void) {
    tick_counter = 0;
    
    // 3.579.540 Hz, divided by 256
    
    CTC_Chan3 = 0xB7;
    CTC_Chan3 = 0x0E;
}

void clk_ser_init(void) {
    // CLOCK/2 = 1,78982Mhz   ---> We get this in input to the CTC, 
    // we have a prescaler of 16 on the clock of the dart -> 111.864 Hz

    // Program the interrupt vector
    CTC_Chan0 = 0x18;

    // Initialize clock for serial 1
    CTC_Chan0 = 0x45; // Control word, interrupt, continued operation, followed by time constant, automatic trigger, falling edge, counter mode, no interrupts
    CTC_Chan0 = 0x06; // time constant
    
    // The same for the second serial port
    CTC_Chan1 = 0x45;
    CTC_Chan1 = 0x06; // time constant
}

uint32_t get_tick(void) {
    uint32_t tick;
       
    __asm di __endasm;
        tick = tick_counter;
    __asm ei __endasm;
    
    return tick;
}

void ctc_isr_0(void) __interrupt(0x18) { }
void ctc_isr_1(void) __interrupt(0x1A) { }
void ctc_isr_2(void) __interrupt(0x1C) { }

void ctc_isr_3(void) __interrupt(0x1E) {
    tick_counter++;
}

