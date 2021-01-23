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

static uint8_t isr_counter;
volatile uint16_t tick_counter = 0;

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

void clk_ser_init(void) {
    // CLOCK/2 = 1,78982Mhz   ---> We get this in input to the CTC
    // DART for port B is running in x1 prescaler mode
    // For 9600bps we have a counter of 1.789.820 / 9600 ~= 186

    tick_counter = 0;
    isr_counter = 0;

    // Program the interrupt vector
    CTC_Chan0 = 0x18;

    // Initialize clock for serial 1
    CTC_Chan0 = 0xC5; // Control word, interrupt, continued operation, followed by time constant, automatic trigger, falling edge, prescaler of 16, counter mode, no interrupts
    CTC_Chan0 = 0xBA; // time constant
    
    // The same for the second serial port
    CTC_Chan1 = 0x45;
    CTC_Chan1 = 0xBA; // time constant
}

void ctc_isr_0(void) __interrupt(0x18) {
    isr_counter++;
    
    if(!(isr_counter%3)) tick_counter++;
}

void ctc_isr_1(void) __interrupt(0x1A) { }
void ctc_isr_2(void) __interrupt(0x1C) { }
void ctc_isr_3(void) __interrupt(0x1E) { }

