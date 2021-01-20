#include "pio.h"

#define PIO_BASE   0x00

static __sfr __at (PIO_BASE+0x00) PIO_PortA;
static __sfr __at (PIO_BASE+0x01) PIO_CtrlA;
static __sfr __at (PIO_BASE+0x02) PIO_PortB;
static __sfr __at (PIO_BASE+0x03) PIO_CtrlB;

void pio_init(void) {
    // Taken from Tranz 330 Library routines
    PIO_CtrlA = 0xCF; // Control each port bit individually
    PIO_CtrlA = 0x80; // Bit 7 is input, the others are outputs
    PIO_CtrlA = 0x18; // Interupt vector 0x18
    PIO_CtrlA = 0x97; // Generate interrupt if any masked bit is low 
    PIO_CtrlA = 0x7F; // Mask bit 7 from the interrupt
    PIO_PortA = 0x00; // Set the initial value for port A
}