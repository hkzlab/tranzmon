#include "keypad.h"

#include <string.h>
#include <stdio.h>

#include <hardware/pio.h>
#include <hardware/ctc.h>
#include <hardware/rtc.h>
#include <hardware/display.h>

/***
 The keypad:

 1 2 3 A
 4 5 6 B
 7 8 9 C
 E 0 F D
***/

#define DEFAULT_STATE_TIMEOUT 15000

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
    KP_MREAD
} keypad_sm_state;

static char disp_buffer[DISP_SIZE + 1];

static uint8_t kp_stat[4], kp_stat_buf[4];
static rtc_stat clk;
static uint16_t keypad_tick_counter;
static keypad_sm_state sm_state;
static uint32_t sm_state_time;

static uint16_t s_val16[1];

static void format_rtc_short(rtc_stat *clk, char *buf);
static void refresh_state(void);

static void state_mread(uint32_t now);
static void state_default(void);

void keypad_init(void) {
    // Clear keypad buffer
	memset(kp_stat, 0, 4);
	memset(kp_stat_buf, 0, 4);	
	
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
        case KP_MREAD:
            state_mread(now);
            //if((now - sm_state_time) >= DEFAULT_STATE_TIMEOUT) { sm_state = KP_DEFAULT; spkr_beep(0x30, 20); };
            break;
        case KP_DEFAULT:
        default:
            state_default();
            if(KP_1(kp_stat, kp_stat_buf)) { // Enter memory read mode
                sm_state = KP_MREAD;
                s_val16[0] = 0;
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

static void format_rtc_short(rtc_stat *clk, char *buf) {
    sprintf(buf, "%02X/%02X/%02X  %02X:%02X ", clk->d, clk->M, clk->y, clk->h, clk->m);
}

/***/

static void state_mread(uint32_t now) {
    sprintf(disp_buffer, "MR @%04X  %02X", s_val16[0], *(uint8_t*)s_val16[0]);
    disp_print(disp_buffer);
    
    if(KP_E(kp_stat, kp_stat_buf)) { s_val16[0] -= 0x01; sm_state_time = now; }
    else if(KP_F(kp_stat, kp_stat_buf)) { s_val16[0] += 0x01; sm_state_time = now; }
    else if(KP_7(kp_stat, kp_stat_buf)) { s_val16[0] -= 0x10; sm_state_time = now; }
    else if(KP_9(kp_stat, kp_stat_buf)) { s_val16[0] += 0x10; sm_state_time = now; }
    else if(KP_4(kp_stat, kp_stat_buf)) { s_val16[0] -= 0x100; sm_state_time = now; }
    else if(KP_6(kp_stat, kp_stat_buf)) { s_val16[0] += 0x100; sm_state_time = now; }
    else if(KP_1(kp_stat, kp_stat_buf)) { s_val16[0] -= 0x1000; sm_state_time = now; }
    else if(KP_3(kp_stat, kp_stat_buf)) { s_val16[0] += 0x1000; sm_state_time = now; }  
    else if(KP_D(kp_stat, kp_stat_buf)) { sm_state = KP_DEFAULT; spkr_beep(0x30, 20); }
}

static void state_default(void) {
    // Update clock on display
    if(!(keypad_tick_counter & 0x3FF)) {
        rtc_get(&clk);
        format_rtc_short(&clk, disp_buffer);
        disp_print(disp_buffer);
    }    
}
