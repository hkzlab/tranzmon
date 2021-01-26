#include "pio.h"

#include <hardware/display.h>

#define PIO_BASE   0x00

#define DISPLAY_RESET 4
#define DISPLAY_DATA 5
#define DISPLAY_CLOCK 6

static __sfr __at (PIO_BASE+0x00) PIO_PortA;
static __sfr __at (PIO_BASE+0x01) PIO_CtrlA;
static __sfr __at (PIO_BASE+0x02) PIO_PortB;
static __sfr __at (PIO_BASE+0x03) PIO_CtrlB;


void disp_send_byte(uint8_t data);

void pio_init(void) {
    // Taken from Tranz 330 Library routines
    PIO_CtrlA = 0xCF; // Control each port bit individually
    PIO_CtrlA = 0x80; // Bit 7 is input, the others are outputs
    PIO_CtrlA = 0x10; // Interrupt vector 0x10
    PIO_CtrlA = 0x97; // Generate interrupt if any masked bit is low 
    PIO_CtrlA = 0x7F; // Mask bit 7 from the interrupt
    PIO_PortA = 0x3F; // Set the initial value for port A
}

void disp_init(void) {
    uint8_t ctrl_val = PIO_PortA & ~(1 << DISPLAY_RESET);
    uint8_t wait_counter = 0xFF;
    
    // Reset the display and wait a bit
    PIO_PortA = ctrl_val;
    
    while(wait_counter--) {
        __asm
            nop
        __endasm;
    }
    
    PIO_PortA = (ctrl_val | (1 << DISPLAY_RESET));

    disp_send_byte(DISP_DUTY_CYCLE | 0x0A); // Set duty cycle to set brightness
}

void disp_send_byte(uint8_t data) {
    uint8_t ctrl_val;
    uint8_t bits = 7;
    // Data will be sent MSB first. An MSB of 1 indicates a control byte, 0 indicates data to be displayed
    // The display controller data is in bit 5 of port A, display controller clock is in bit 6
    do {
        ctrl_val = PIO_PortA & ~((1 << DISPLAY_DATA) | (1 << DISPLAY_CLOCK)); // Read current port state and clean it
        ctrl_val |= (((data >> bits) & 0x01) << DISPLAY_DATA);
        PIO_PortA = ctrl_val;
        PIO_PortA = (ctrl_val | (1 << DISPLAY_CLOCK)); // Set clock output to high
        PIO_PortA = ctrl_val;
    } while(bits--);
}

void kp_selectColumn(uint8_t col) {
    uint8_t val = PIO_PortA & 0xF0;
    PIO_PortA = val | (~(0x01 << col) & 0x0F);
}

uint8_t kp_readRows(void) {
    return PIO_PortB & 0x0F;
}


