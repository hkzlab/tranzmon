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

#define KP_E() (kb_stat[0] & 0x08)
#define KP_7() (kb_stat[0] & 0x04)
#define KP_4() (kb_stat[0] & 0x02)
#define KP_1() (kb_stat[0] & 0x01)

#define KP_0() (kb_stat[1] & 0x08)
#define KP_8() (kb_stat[1] & 0x04)
#define KP_5() (kb_stat[1] & 0x02)
#define KP_2() (kb_stat[1] & 0x01)

#define KP_F() (kb_stat[2] & 0x08)
#define KP_9() (kb_stat[2] & 0x04)
#define KP_6() (kb_stat[2] & 0x02)
#define KP_3() (kb_stat[2] & 0x01)

#define KP_D() (kb_stat[3] & 0x08)
#define KP_C() (kb_stat[3] & 0x04)
#define KP_B() (kb_stat[3] & 0x02)
#define KP_A() (kb_stat[3] & 0x01)

typedef enum {
    KP_DEFAULT,
    KP_MREAD
} keypad_sm_state;

static char disp_buffer[DISP_SIZE + 1];
static uint8_t kb_stat[4];
static rtc_stat clk;
static uint16_t keypad_tick_counter;
static keypad_sm_state sm_state;

static void format_rtc_short(rtc_stat *clk, char *buf);
static void refresh_state(void);

static void state_default(void);

void keypad_init(void) {
    // Clear keypad buffer
	memset(kb_stat, 0, 4);	
	
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
	//if((kb_rows ^ kb_stat[kb_col]) && (~kb_rows & kb_stat[kb_col])) spkr_beep(0x50, 2); // Beep if we detect a new key
	kb_stat[kb_col] = ~kb_rows & 0x0F;
	kb_selectColumn(5); // This will disable every column
		    
    // Time to read the keypresses
    if(!kb_col) {
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
