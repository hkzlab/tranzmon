#include "keypad.h"

#include <string.h>
#include <stdio.h>

#include <hardware/pio.h>
#include <hardware/ctc.h>
#include <hardware/rtc.h>
#include <hardware/display.h>

static char disp_buffer[DISP_SIZE + 1];
static uint8_t kb_stat[4];
static rtc_stat clk;

static uint16_t keypad_tick_counter;
static uint8_t kb_col;

static void format_rtc_short(rtc_stat *clk, char *buf, uint8_t blink);

void keypad_init(void) {
    // Clear keypad buffer
	memset(kb_stat, 0, 4);	
	
	keypad_tick_counter = 0;
	kb_col = 0;
}

void keypad_tick(void) {
    uint8_t kb_rows = 0;
    
    // Check KB interaction
    kb_col = (kb_col + 1)%4;
    kb_selectColumn(kb_col);
    kb_rows = kb_readRows();
	if((kb_rows ^ kb_stat[kb_col]) && (~kb_rows & kb_stat[kb_col])) spkr_beep(0x50, 2); // Beep if we detect a new key
	kb_stat[kb_col] = kb_rows;
		    
    // Update clock on display
    if(!(keypad_tick_counter & 0xFF)) {
        rtc_get(&clk);
        format_rtc_short(&clk, disp_buffer, (keypad_tick_counter >> 10) & 0x01);
        disp_print(disp_buffer);
    }
		    
    keypad_tick_counter++;
}

static void format_rtc_short(rtc_stat *clk, char *buf, uint8_t blink) {
    sprintf(buf, "%02X/%02X/%02X  %02X%c%02X ", clk->d, clk->M, clk->y, clk->h, (blink?':':' '), clk->m);
}

