#include "clock.h"

#define CLOCK_BASE   0x30

static char dow_names[7][10] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

static __sfr __at (CLOCK_BASE+0x00) CLK_Reg_0;
static __sfr __at (CLOCK_BASE+0x01) CLK_Reg_1;
static __sfr __at (CLOCK_BASE+0x02) CLK_Reg_2;
static __sfr __at (CLOCK_BASE+0x03) CLK_Reg_3;
static __sfr __at (CLOCK_BASE+0x04) CLK_Reg_4;
static __sfr __at (CLOCK_BASE+0x05) CLK_Reg_5;
static __sfr __at (CLOCK_BASE+0x06) CLK_Reg_6;
static __sfr __at (CLOCK_BASE+0x07) CLK_Reg_7;
static __sfr __at (CLOCK_BASE+0x08) CLK_Reg_8;
static __sfr __at (CLOCK_BASE+0x09) CLK_Reg_9;
static __sfr __at (CLOCK_BASE+0x0A) CLK_Reg_A;
static __sfr __at (CLOCK_BASE+0x0B) CLK_Reg_B;
static __sfr __at (CLOCK_BASE+0x0C) CLK_Reg_C;
static __sfr __at (CLOCK_BASE+0x0D) CLK_Reg_D;
static __sfr __at (CLOCK_BASE+0x0E) CLK_Reg_E;
static __sfr __at (CLOCK_BASE+0x0F) CLK_Reg_F;

void clock_set(clock_stat *cs) {
}

void clock_get(clock_stat *cs) {
    // Seconds
    cs->s = CLK_Reg_0 & 0x0F;
    cs->s |= (CLK_Reg_1 & 0x07) << 4;
    
    // Minutes
    cs->m = CLK_Reg_2 & 0x0F;
    cs->m |= (CLK_Reg_3 & 0x07) << 4;
    
    // Hours
    cs->h = CLK_Reg_4 & 0x0F;
    cs->h |= (CLK_Reg_5 & 0x03) << 4;
    
    // Day
    cs->d = CLK_Reg_6 & 0x0F;
    cs->d |= (CLK_Reg_7 & 0x03) << 4;   
    
    // Month
    cs->M = CLK_Reg_8 & 0x0F;
    cs->M |= (CLK_Reg_9 & 0x01) << 4;
        
    // Year
    cs->y = CLK_Reg_A & 0x0F;
    cs->y |= (CLK_Reg_B & 0x0F) << 4;
        
    // DOW
    cs->dow = CLK_Reg_C ^ 0x07;
}

char *clock_dowName(uint8_t dow) {
    return dow_names[dow % 7];
}

