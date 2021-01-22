#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common_datatypes.h>
#include <hardware/pio.h>
#include <hardware/ctc.h>
#include <hardware/dart.h>

#include "utilities.h"

#define CMD_BUF_SIZE 15

#define MONITOR_CMD_PROMPT "\r\n] "
#define MONITOR_ERR_MSG "\r\nE\r\n"

#define STR_BUFF_LEN 13
static char mon_buff[STR_BUFF_LEN];
static char cmd_buffer[CMD_BUF_SIZE];

/******/
void pio_isr (void) __interrupt(0x10);
void dart_isr(void) __interrupt(0x12);
void ctc_isr_0(void) __interrupt(0x18);
void ctc_isr_1(void) __interrupt(0x1A);
void ctc_isr_2(void) __interrupt(0x1C);
void ctc_isr_3(void) __interrupt(0x1E);

void monitor_parse_command(char *cmd, uint8_t idx);

/**/
void monitor_outp(uint8_t port, uint8_t data);
uint8_t monitor_inp(uint8_t port);
void monitor_jmp(uint8_t *addr);

extern char str_appname;

/** Here lies the code **/
void sys_init(void) {
	pio_init();
	disp_init();
	disp_clear();
	spkr_init();
	
    clk_ser_init();
    dart_init();
	
	disp_print(&str_appname);
	
	spkr_beep(0x2F); // Beep the speaker!
	
	// Enable the interrupts
	__asm
	    ei
	__endasm;
}

void main(void) {
	uint8_t buf_idx = 0, cmd_read_loop = 1;
	char ch; // Char buffer for data read from console

	// Do basic system initialization
	sys_init();
	
	while(1);
/*
	console_printString(MONITOR_HEAD);

	while(1) { // Endless loop
		console_printString(MONITOR_CMD_PROMPT);

		cmd_read_loop = 1;
		buf_idx = 0;
		while(cmd_read_loop) {
			ch = getchar(); // Read a char
			putchar(ch); // Print it
			
			// Turn the letter uppercase for parsing purposes
			if (ch >= 0x61) {
				ch &= 0xDF;
			}

			switch(ch) {
				case 0x0D: // CR
					monitor_parse_command(cmd_buffer, buf_idx);	
					cmd_read_loop = 0;
					break;
				default:
					if(buf_idx >= CMD_BUF_SIZE) {
						cmd_read_loop = 0;

						//console_printString(MONITOR_ERR_MSG);

					} else {
						cmd_buffer[buf_idx++] = ch;
					}
					break;
			}

		}
	}
	*/
}

/***/
/*
void monitor_parse_command(char *cmd, uint8_t idx) {
	uint8_t val;

	if (!idx) return;

	mon_buff[0] = '\r';
	mon_buff[1] = '\n';

	switch(cmd[0]) {
#ifdef __USE_N8VEM_SERIO__
		case 'X': // XModem transfer
			xmodem_receive((uint8_t*)monitor_parseU16(&cmd[1]));
			break;
#endif
		case 'I': // IN
			val = monitor_inp(monitor_parseU8(&cmd[1]));
			
			// Prepare the string to print
			monitor_printU8(val, &mon_buff[6]);
			mon_buff[2] = cmd[1];
			mon_buff[3] = cmd[2];
			mon_buff[4] = mon_buff[5] = ' ';
			mon_buff[8] = 0;

			console_printString(mon_buff);
			break;
		case 'R': // READ
			val = *((uint8_t*)monitor_parseU16(&cmd[1]));

			// Prepare the string to print
			memcpy(mon_buff + 2, &cmd[1], 4);
			monitor_printU8(val, &mon_buff[8]);
			mon_buff[6] = mon_buff[7] = ' ';
			mon_buff[10] = 0;

			console_printString(mon_buff);
			break;
		case 'W': // WRITE
			*((uint8_t*)monitor_parseU16(&cmd[1])) = monitor_parseU8(&cmd[6]);
			break;
		case 'J': // JP
			monitor_jmp((uint8_t*)monitor_parseU16(&cmd[1]));
			break;
		case 'O': // OUT
			monitor_outp(monitor_parseU8(&cmd[1]), monitor_parseU8(&cmd[4]));
			break;
		default:
			console_printString(MONITOR_ERR_MSG);
			break;
	}

	return;
}
*/
/*** Monitor Commands ***/
/*
void monitor_jmp(uint8_t *addr) __naked {
	addr;

	__asm
		pop bc
		pop hl
		jp (hl)
	__endasm;
}

void monitor_outp(uint8_t port, uint8_t data) __naked {
	port; data;

	__asm
		ld hl, #3
		add hl, sp
		ld a, (hl) // Load data from stack

		ld hl, #2
		add hl, sp

		push bc
		
		ld c, (hl) // Load port from stack
		out (c), a // Output to port

		pop bc

		ret
	__endasm;
}

uint8_t monitor_inp(uint8_t port) __naked {
	port;

	__asm
		ld hl, #2
		add hl, sp

		push bc

		ld c, (hl)
		in l,(c)

		pop bc

		ret
	__endasm;
}
*/
void pio_isr (void) __interrupt(0x10) {}
void dart_isr(void) __interrupt(0x12) {}
void ctc_isr_0(void) __interrupt(0x18) {}
void ctc_isr_1(void) __interrupt(0x1A) {}
void ctc_isr_2(void) __interrupt(0x1C) {}
void ctc_isr_3(void) __interrupt(0x1E) {}
