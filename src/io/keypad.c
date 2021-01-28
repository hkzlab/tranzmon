#include "keypad.h"

#include <string.h>
#include <stdio.h>

#include <hardware/pio.h>
#include <hardware/ctc.h>
#include <hardware/rtc.h>
#include <hardware/display.h>

#include <utilities.h>

/***
 The keypad:

 1 2 3 A
 4 5 6 B
 7 8 9 C
 E 0 F D
***/

#define DEFAULT_STATE_TIMEOUT 10000

#define FORMAT_ARRAY_SIZE 7
#define STATE_VARARR_SIZE 5
#define KP_COLUMNS 4

#define KP_E(kp, kpb) ((kp[0] & 0x08) && !(kpb[0] & 0x08))
#define KP_7(kp, kpb) ((kp[0] & 0x04) && !(kpb[0] & 0x04))
#define KP_4(kp, kpb) ((kp[0] & 0x02) && !(kpb[0] & 0x02))
#define KP_1(kp, kpb) ((kp[0] & 0x01) && !(kpb[0] & 0x01))

#define KP_0(kp, kpb) ((kp[1] & 0x08) && !(kpb[1] & 0x08))
#define KP_8(kp, kpb) ((kp[1] & 0x04) && !(kpb[1] & 0x04))
#define KP_5(kp, kpb) ((kp[1] & 0x02) && !(kpb[1] & 0x02))
#define KP_2(kp, kpb) ((kp[1] & 0x01) && !(kpb[1] & 0x01))

#define KP_F(kp, kpb) ((kp[2] & 0x08) && !(kpb[2] & 0x08))
#define KP_9(kp, kpb) ((kp[2] & 0x04) && !(kpb[2] & 0x04))
#define KP_6(kp, kpb) ((kp[2] & 0x02) && !(kpb[2] & 0x02))
#define KP_3(kp, kpb) ((kp[2] & 0x01) && !(kpb[2] & 0x01))

#define KP_D(kp, kpb) ((kp[3] & 0x08) && !(kpb[3] & 0x08))
#define KP_C(kp, kpb) ((kp[3] & 0x04) && !(kpb[3] & 0x04))
#define KP_B(kp, kpb) ((kp[3] & 0x02) && !(kpb[3] & 0x02))
#define KP_A(kp, kpb) ((kp[3] & 0x01) && !(kpb[3] & 0x01))

typedef enum {
    KP_DEFAULT,
    KP_MREAD,
    KP_MWRITE,
    KP_JMP,
    KP_OPORT,
    KP_IPORT,
    KP_CLOCK_DMYW,
    KP_CLOCK_HMS
} keypad_sm_state;

static char disp_buffer[DISP_SIZE + 8]; // Leave some additional space for the null character and the '.', which the display condenses in the previous char
static char format_array[FORMAT_ARRAY_SIZE];

static uint8_t kp_stat[KP_COLUMNS], kp_stat_buf[KP_COLUMNS];
static rtc_stat clk;
static uint16_t keypad_tick_counter;
static keypad_sm_state sm_state;
static uint32_t sm_state_time;

static uint16_t s_val16[STATE_VARARR_SIZE];
static uint8_t s_val8[STATE_VARARR_SIZE];

static uint8_t read_btn(void);
static void format_rtc_short(rtc_stat *clk, char *buf);
static void refresh_state(void);
static void clear_vars(void);

static void state_mread(uint32_t now);
static void state_mwrite(uint32_t now);
static void state_clock(uint32_t now);
static void state_oport(uint32_t now);
static void state_iport(uint32_t now);
static void state_default(void);

void keypad_init(void) {
    // Clear keypad buffer
	memset(kp_stat, 0, KP_COLUMNS);
	memset(kp_stat_buf, 0, KP_COLUMNS);	
	
	keypad_tick_counter = 0;
	sm_state_time = 0;
	sm_state = KP_DEFAULT;
}

void keypad_tick(void) {
    uint8_t kp_rows = 0, kp_col = 0;
    
    // Check KB interaction
    kp_col = keypad_tick_counter & 0x03;
    kp_selectColumn(kp_col);
    kp_rows = kp_readRows();
	if(((kp_rows ^ ~kp_stat[kp_col]) & ~kp_rows) & 0x0F) spkr_beep(0x80, 1); // Beep if we detect a new key
	kp_stat[kp_col] = ~kp_rows & 0x0F;
	kp_selectColumn(5); // This will disable every column
		    
    // Time to refresh state and read the keypresses
    if(!kp_col) refresh_state();
		    
    keypad_tick_counter++;
}

static void refresh_state(void) {
    uint32_t now = get_tick();

    switch(sm_state) {
        case KP_IPORT:
            state_iport(now);
            if((now - sm_state_time) >= DEFAULT_STATE_TIMEOUT) { sm_state = KP_DEFAULT; spkr_beep(0x30, 20); };           
            break;
        case KP_OPORT:
            state_oport(now);
            if((now - sm_state_time) >= DEFAULT_STATE_TIMEOUT) { sm_state = KP_DEFAULT; spkr_beep(0x30, 20); };           
            break;
        case KP_CLOCK_DMYW:
        case KP_CLOCK_HMS:
            state_clock(now);
            if((now - sm_state_time) >= DEFAULT_STATE_TIMEOUT) { sm_state = KP_DEFAULT; spkr_beep(0x30, 20); };
            break;
        case KP_JMP:
            disp_clear();
            sprintf(disp_buffer, "JUMPING @%04X", s_val16[0]);
            disp_print(disp_buffer);
            spkr_beep(0x30, 100);
            delay_ms_ctc(2000);
            disp_clear();
            monitor_jmp((uint8_t*)s_val16[0]);
            break;
        case KP_MWRITE:
            state_mwrite(now);
            if((now - sm_state_time) >= DEFAULT_STATE_TIMEOUT) { sm_state = KP_MREAD; spkr_beep(0x30, 20); };
            break;
        case KP_MREAD:
            state_mread(now);
            break;
        case KP_DEFAULT:
        default:
            state_default();
            if(KP_1(kp_stat, kp_stat_buf)) { // Enter memory read mode
                sm_state = KP_MREAD;
                clear_vars();
                disp_clear();
                sm_state_time = now; 
            } else if(KP_2(kp_stat, kp_stat_buf)) { // Input port mode
                sm_state = KP_IPORT;
                clear_vars();
                disp_clear();
                sm_state_time = now; 
            } else if(KP_3(kp_stat, kp_stat_buf)) { // Output port mode
                sm_state = KP_OPORT;
                clear_vars();
                disp_clear();
                sm_state_time = now; 
            } else if(KP_4(kp_stat, kp_stat_buf)) { // Enter clock set mode
                clear_vars();
                sm_state = KP_CLOCK_DMYW;
                disp_clear();
                sm_state_time = now;             
            }
            break;
    }
    
    // Update the keypad buffer
    kp_stat_buf[0] = kp_stat[0];
    kp_stat_buf[1] = kp_stat[1];
    kp_stat_buf[2] = kp_stat[2];
    kp_stat_buf[3] = kp_stat[3];
}

static uint8_t read_btn(void) {
    if(KP_0(kp_stat, kp_stat_buf)) return 0x00;
    else if(KP_1(kp_stat, kp_stat_buf)) return 0x01;
    else if(KP_2(kp_stat, kp_stat_buf)) return 0x02;
    else if(KP_3(kp_stat, kp_stat_buf)) return 0x03;    
    else if(KP_4(kp_stat, kp_stat_buf)) return 0x04;
    else if(KP_5(kp_stat, kp_stat_buf)) return 0x05;
    else if(KP_6(kp_stat, kp_stat_buf)) return 0x06;
    else if(KP_7(kp_stat, kp_stat_buf)) return 0x07;
    else if(KP_8(kp_stat, kp_stat_buf)) return 0x08;
    else if(KP_9(kp_stat, kp_stat_buf)) return 0x09;
    else if(KP_A(kp_stat, kp_stat_buf)) return 0x0A;
    else if(KP_B(kp_stat, kp_stat_buf)) return 0x0B;
    else if(KP_C(kp_stat, kp_stat_buf)) return 0x0C;
    else if(KP_D(kp_stat, kp_stat_buf)) return 0x0D;
    else if(KP_E(kp_stat, kp_stat_buf)) return 0x0E;
    else if(KP_F(kp_stat, kp_stat_buf)) return 0x0F;
    
    return 0xFF;
}

static void format_rtc_short(rtc_stat *clk, char *buf) {
    char *dow = rtc_dowName(clk->dow);
    sprintf(buf, "%02X.%02X.%02X %02X.%02X %c%c%c", clk->d, clk->M, clk->y, clk->h, clk->m, dow[0], dow[1], dow[2]);
}

static void clear_vars(void) {
    memset(s_val8, 0, STATE_VARARR_SIZE);
    memset(s_val16, 0, STATE_VARARR_SIZE*sizeof(uint16_t));
}

/***/

static void state_clock(uint32_t now) {
    memset(format_array, '_', 7);
    uint8_t btn = read_btn();

    if(btn != 0xFF) sm_state_time = now;

    if(s_val8[0] > 0) format_array[0] = nibble_to_hex(s_val8[1] >> 4);
    if(s_val8[0] > 1) format_array[1] = nibble_to_hex(s_val8[1] >> 0);
    if(s_val8[0] > 2) format_array[2] = nibble_to_hex(s_val8[2] >> 4);
    if(s_val8[0] > 3) format_array[3] = nibble_to_hex(s_val8[2] >> 0);
    if(s_val8[0] > 4) format_array[4] = nibble_to_hex(s_val8[3] >> 4);
    if(s_val8[0] > 5) format_array[5] = nibble_to_hex(s_val8[3] >> 0);
    if(s_val8[0] > 6) format_array[6] = nibble_to_hex(s_val8[4] >> 4);
    
    if(sm_state == KP_CLOCK_DMYW) {
        sprintf(disp_buffer, "DATE %c%c/%c%c/%c%c %c", format_array[0], format_array[1], format_array[2], format_array[3], format_array[4], format_array[5], format_array[6]);
        disp_print(disp_buffer);
        
        if(s_val8[0] > 6) {
            clk.d = s_val8[1];
            clk.M = s_val8[2];
            clk.y = s_val8[3];
            clk.dow = s_val8[4] >> 4;
        
            memset(s_val8, 0, 5);
            sm_state = KP_CLOCK_HMS;
            
            delay_ms_ctc(200);
            spkr_beep(0x50, 50);
            return;
        }
        
        if(btn <= 9) { s_val8[1 + (s_val8[0]/2)] |= btn << (4 - (4 * (s_val8[0]%2)));  s_val8[0]++; }
    } else {
        sprintf(disp_buffer, "HOUR %c%c.%c%c.%c%c    ", format_array[0], format_array[1], format_array[2], format_array[3], format_array[4], format_array[5]);
        disp_print(disp_buffer);
        
        if(s_val8[0] > 5) {
            clk.h = s_val8[1];
            clk.m = s_val8[2];
            clk.s = s_val8[3];
        
        	rtc_set(&clk);
        
            sm_state = KP_DEFAULT;
            delay_ms_ctc(500);
            spkr_beep(0x30, 100);
            return;
        }
        
        if((btn <= 9) && (s_val8[0] <= 5)) { 
            s_val8[1 + (s_val8[0]/2)] |= btn << (4 - (4 * (s_val8[0]%2)));
            s_val8[0]++; 
        }
    }
    
    if(btn == 0x0D) { // Cancel and back
            sm_state = KP_DEFAULT;
            spkr_beep(0x30, 20);
    }
}

static void state_mwrite(uint32_t now) {
    format_array[0] = format_array[1] = '_';
    uint8_t btn;

    if(s_val8[0] > 0) format_array[0] = nibble_to_hex(s_val8[1] >> 4);
    if(s_val8[0] > 1) format_array[1] = nibble_to_hex(s_val8[1]);

    sprintf(disp_buffer, "MW @%04X  %c%c", s_val16[0], format_array[0], format_array[1]);
    disp_print(disp_buffer);    
    
    if(s_val8[0] > 1) {
        *((uint8_t*)s_val16[0]) = s_val8[1];
        sm_state = KP_MREAD;
        delay_ms_ctc(500);
        spkr_beep(0x30, 100);
        return;
    }

    btn = read_btn();
    if(btn <= 0x0F) {
        sm_state_time = now;
        s_val8[1] |= (btn << ((1-s_val8[0]) * 4));
        s_val8[0]++;
    }
    
}

static void state_mread(uint32_t now) {
    sprintf(disp_buffer, "MR @%04X  %02X", s_val16[0], *(uint8_t*)s_val16[0]);
    disp_print(disp_buffer);
    
    if(KP_E(kp_stat, kp_stat_buf)) s_val16[0] -= 0x01;
    else if(KP_F(kp_stat, kp_stat_buf)) s_val16[0] += 0x01;
    else if(KP_7(kp_stat, kp_stat_buf)) s_val16[0] -= 0x10;
    else if(KP_9(kp_stat, kp_stat_buf)) s_val16[0] += 0x10;
    else if(KP_4(kp_stat, kp_stat_buf)) s_val16[0] -= 0x100;
    else if(KP_6(kp_stat, kp_stat_buf)) s_val16[0] += 0x100;
    else if(KP_1(kp_stat, kp_stat_buf)) s_val16[0] -= 0x1000;
    else if(KP_3(kp_stat, kp_stat_buf)) s_val16[0] += 0x1000;  
    else if(KP_D(kp_stat, kp_stat_buf)) { sm_state = KP_DEFAULT; spkr_beep(0x30, 20); }
    else if(KP_A(kp_stat, kp_stat_buf)) sm_state = KP_JMP;
    else if(KP_B(kp_stat, kp_stat_buf)) { sm_state = KP_MWRITE; sm_state_time = now; disp_clear(); s_val8[0] = s_val8[1] = 0; }
}

static void state_default(void) {
    // Update clock on display
    if(!(keypad_tick_counter & 0x4FF)) {
        rtc_get(&clk);
        format_rtc_short(&clk, disp_buffer);
        disp_print(disp_buffer);
    }    
}

static void state_oport(uint32_t now) {
    uint8_t btn;
    memset(format_array, '_', 4);
    
    if(s_val8[0] > 0) format_array[0] = nibble_to_hex(s_val8[1] >> 4);
    if(s_val8[0] > 1) format_array[1] = nibble_to_hex(s_val8[1] >> 0);
    if(s_val8[0] > 2) format_array[2] = nibble_to_hex(s_val8[2] >> 4);
    if(s_val8[0] > 3) format_array[3] = nibble_to_hex(s_val8[2] >> 0);

    sprintf(disp_buffer, "OUT P.%c%c V.%c%c", format_array[0], format_array[1], format_array[2], format_array[3]);
    disp_print(disp_buffer);
    
    if(s_val8[0] > 3) {
        sm_state = KP_DEFAULT;
        delay_ms_ctc(1000);
        spkr_beep(0x30, 100);
        monitor_outp(s_val8[1], s_val8[2]);
        return;
    }
    
    btn = read_btn();
    if((btn != 0xFF) && (s_val8[0] <= 3)) { 
        sm_state_time = now; 
        s_val8[1 + (s_val8[0]/2)] |= (btn << (4 - (4 * (s_val8[0]%2))));
        s_val8[0]++;
    }
}

static void state_iport(uint32_t now) {
    uint8_t btn;
    format_array[0] = format_array[1] = '_';
    format_array[2] = format_array[3] = '*';
    
    if(s_val8[0] > 0) format_array[0] = nibble_to_hex(s_val8[1] >> 4);
    if(s_val8[0] > 1) format_array[1] = nibble_to_hex(s_val8[1] >> 0);
    if(s_val8[0] > 2) {
        format_array[2] = nibble_to_hex(s_val8[2] >> 4);
        format_array[3] = nibble_to_hex(s_val8[2] >> 0);
    }

    sprintf(disp_buffer, "IN  P.%c%c V.%c%c", format_array[0], format_array[1], format_array[2], format_array[3]);
    disp_print(disp_buffer);
    
    if(s_val8[0] == 2) {
        s_val8[2] = monitor_inp(s_val8[1]);
        s_val8[0]++;
    } else if(s_val8[0] > 2) {
        sm_state = KP_DEFAULT;
        delay_ms_ctc(2000);
        spkr_beep(0x30, 100);
        return;
    }
    
    btn = read_btn();
    if((btn != 0xFF) && (s_val8[0] <= 1)) {
        sm_state_time = now; 
        s_val8[1 + (s_val8[0]/2)] |= (btn << (4 - (4 * (s_val8[0]%2))));
        s_val8[0]++; 
    }
}


