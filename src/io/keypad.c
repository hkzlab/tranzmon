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
#define KP_3(kp, kpb) ((kp[2] & 0x01) && !(kpb[2] & 0x02))

#define KP_D(kp, kpb) ((kp[3] & 0x08) && !(kpb[3] & 0x08))
#define KP_C(kp, kpb) ((kp[3] & 0x04) && !(kpb[3] & 0x04))
#define KP_B(kp, kpb) ((kp[3] & 0x02) && !(kpb[3] & 0x02))
#define KP_A(kp, kpb) ((kp[3] & 0x01) && !(kpb[3] & 0x01))

typedef enum {
    KP_DEFAULT,
    KP_MREAD
} keypad_sm_state;

static char disp_buffer[DISP_SIZE + 1];
static uint8_t kb_stat[4], kb_stat_buf[4];
static rtc_stat clk;
static uint16_t keypad_tick_counter;
static keypad_sm_state sm_state;

static void format_rtc_short(rtc_stat *clk, char *buf);
static void refresh_state(void);

static void state_default(void);

void keypad_init(void) {
    // Clear keypad buffer
	memset(kb_stat, 0, 4);
	memset(kb_stat_buf, 0, 4);	
	
	keypad_tick_counter = 0;
	sm_state = KP_DEFAULT;
}

void keypad_tick(void) {
    uint32_t now = get_tick();
    uint8_t kb_rows = 0, kb_col = 0;
    
    // Check KB interaction
    kb_col = keypad_tick_counter & 0x03;
    kb_selectColumn(kb_col);
    kb_rows = kb_readRows();
	if(((kb_rows ^ ~kb_stat[kb_col]) & ~kb_rows) & 0x0F) spkr_beep(0x50, 2); // Beep if we detect a new key
	kb_stat[kb_col] = ~kb_rows & 0x0F;
	kb_selectColumn(5); // This will disable every column
		    
    // Time to read the keypresses
    if(!kb_col) {
    
        //if(KP_A(kb_stat, kb_stat_buf)) spkr_beep(0x80, 20);
    
        // Update the buffer
        kb_stat_buf[0] = kb_stat[0];
        kb_stat_buf[1] = kb_stat[1];
        kb_stat_buf[2] = kb_stat[2];
        kb_stat_buf[3] = kb_stat[3];
    }
    
    refresh_state();
		    
    keypad_tick_counter++;
}

static void refresh_state(void) {
    switch(sm_state) {
        KP_DEFAULT:
        default:
            state_default();
            break;
    }
}

static void format_rtc_short(rtc_stat *clk, char *buf) {
    sprintf(buf, "%02X/%02X/%02X  %02X:%02X ", clk->d, clk->M, clk->y, clk->h, clk->m);
}

/***/

static void state_default(void) {
    // Update clock on display
    if(!(keypad_tick_counter & 0x3FF)) {
        rtc_get(&clk);
        format_rtc_short(&clk, disp_buffer);
        disp_print(disp_buffer);
    }    
}
