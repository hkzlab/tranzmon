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

static uint8_t raw_rreg(uint8_t reg);
static void raw_wreg(uint8_t reg, uint8_t val);
static uint8_t read_clock_reg(uint8_t reg);
static void write_clock_reg(uint8_t reg, uint8_t val);

void clock_init(void) {
    CLK_Reg_F = 0x06;
    __asm nop __endasm;
    CLK_Reg_D = 0x04; // Hold bit at 0
    __asm nop __endasm;
    CLK_Reg_F = 0x04;
}

static uint8_t raw_rreg(uint8_t reg) {
     switch(reg) {
        case 0x00: return CLK_Reg_0;
        case 0x01: return CLK_Reg_1;
        case 0x02: return CLK_Reg_2;
        case 0x03: return CLK_Reg_3;
        case 0x04: return CLK_Reg_4;
        case 0x05: return CLK_Reg_5;
        case 0x06: return CLK_Reg_6;
        case 0x07: return CLK_Reg_7;
        case 0x08: return CLK_Reg_8;
        case 0x09: return CLK_Reg_9;
        case 0x0A: return CLK_Reg_A;
        case 0x0B: return CLK_Reg_B;
        case 0x0C: return CLK_Reg_C;
        case 0x0D: return CLK_Reg_D;
        case 0x0E: return CLK_Reg_E;
        default:
        case 0x0F: return CLK_Reg_F;
    }   
}

static void raw_wreg(uint8_t reg, uint8_t val) {
     switch(reg) {
        case 0x00: CLK_Reg_0 = val; break;
        case 0x01: CLK_Reg_1 = val; break;
        case 0x02: CLK_Reg_2 = val; break;
        case 0x03: CLK_Reg_3 = val; break;
        case 0x04: CLK_Reg_4 = val; break;
        case 0x05: CLK_Reg_5 = val; break;
        case 0x06: CLK_Reg_6 = val; break;
        case 0x07: CLK_Reg_7 = val; break;
        case 0x08: CLK_Reg_8 = val; break;
        case 0x09: CLK_Reg_9 = val; break;
        case 0x0A: CLK_Reg_A = val; break;
        case 0x0B: CLK_Reg_B = val; break;
        case 0x0C: CLK_Reg_C = val; break;
        case 0x0D: CLK_Reg_D = val; break;
        case 0x0E: CLK_Reg_E = val; break;
        default:
        case 0x0F: CLK_Reg_F = val; break;
    }   
}

static uint8_t read_clock_reg(uint8_t reg) {
    uint8_t temp_val;

    while(1) {
        CLK_Reg_D = 0x05; // HOLD bit at 1
        if(CLK_Reg_D & 0x02) {
            CLK_Reg_D = 0x04; // HOLD at 0
            __asm
                nop
                nop
                nop
            __endasm;
        } else break;
    }
    
    temp_val = raw_rreg(reg);
    CLK_Reg_D = 0x04; // HOLD at 0
    
    return temp_val;
}

static void write_clock_reg(uint8_t reg, uint8_t val) {
    while(1) {
        CLK_Reg_D = 0x05; // HOLD bit at 1
        if(CLK_Reg_D & 0x02) {
            CLK_Reg_D = 0x04; // HOLD at 0
            __asm
                nop
                nop
                nop
            __endasm;
        } else break;
    }
    
    raw_wreg(reg, val);
    CLK_Reg_D = 0x04; // HOLD at 0
}

void clock_get(clock_stat *cs) {
    // Seconds
    cs->s = read_clock_reg(0x00) & 0x0F;
    cs->s |= (read_clock_reg(0x01) & 0x07) << 4;
    
    // Minutes
    cs->m = read_clock_reg(0x02) & 0x0F;
    cs->m |= (read_clock_reg(0x03) & 0x07) << 4;
    
    // Hours
    cs->h = read_clock_reg(0x04) & 0x0F;
    cs->h |= (read_clock_reg(0x05) & 0x03) << 4;
    
    // Day
    cs->d = read_clock_reg(0x06) & 0x0F;
    cs->d |= (read_clock_reg(0x07) & 0x03) << 4;   
    
    // Month
    cs->M = read_clock_reg(0x08) & 0x0F;
    cs->M |= (read_clock_reg(0x09) & 0x01) << 4;
        
    // Year
    cs->y = read_clock_reg(0x0A) & 0x0F;
    cs->y |= (read_clock_reg(0x0B) & 0x0F) << 4;
  
    // DOW
    cs->dow = read_clock_reg(0x0C) & 0x07;
}

void clock_set(clock_stat *cs) {
    // Seconds
    write_clock_reg(0x00, cs->s & 0x0F);
    write_clock_reg(0x01, (cs->s >> 4) & 0x07);
    
    // Minutes
    write_clock_reg(0x02, cs->m & 0x0F);
    write_clock_reg(0x03, (cs->m >> 4) & 0x07);
    
    // Hours
    write_clock_reg(0x04, cs->h & 0x0F);
    write_clock_reg(0x05, (cs->h >> 4) & 0x03);
    
    // Day
    write_clock_reg(0x06, cs->d & 0x0F);
    write_clock_reg(0x07, (cs->d >> 4) & 0x03);
    
    // Month
    write_clock_reg(0x08, cs->M & 0x0F);
    write_clock_reg(0x09, (cs->M >> 4) & 0x01);
    
    // Year
    write_clock_reg(0x0A, cs->y & 0x0F);
    write_clock_reg(0x0B, (cs->y >> 4) & 0x0F);
    
    // DOW
    write_clock_reg(0x0C, cs->dow & 0x07);
}

char *clock_dowName(uint8_t dow) {
    return dow_names[dow % 7];
}

