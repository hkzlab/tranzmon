#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common_datatypes.h>
#include <hardware/pio.h>
#include <hardware/ctc.h>
#include <hardware/dart.h>
#include <hardware/rtc.h>
#include <hardware/display.h>
#include <hardware/modem.h>

#include <io/console.h>
#include <io/xmodem.h>
#include <io/keypad.h>

#include "utilities.h"

#define FREE_RAM_START 0x8800

#define CMD_BUF_SIZE 16

#define ANSI_CLRSCR "\x1b[2J\x1b[0m"

#define MONITOR_CMD_PROMPT "\r\n] "
#define MONITOR_ERR_MSG "\r\nERROR!\r\n"

extern char str_appname;
extern char str_clrscr;

static char cmd_buffer[CMD_BUF_SIZE];
static rtc_stat clk;

/******/
void pio_isr (void) __interrupt(0x10);

static void monitor_parse_command(char *cmd, uint8_t idx);

/**/
static void sys_init(void);
static void print_rtc(rtc_stat *clk);
static void monitor_read(uint16_t address, uint8_t blocks);

/** Here lies the code **/
static void sys_init(void) {
	pio_init();
	disp_init();
	disp_clear();
	rtc_init();
	clk_ser_init();
	tick_init();
	dart_init();
	modem_init();
	spkr_init();
	keypad_init();
	
	disp_print(&str_appname);
	
	spkr_beep(0x2F, 150); // Beep the speaker!
	
	// Enable the interrupts
	__asm ei __endasm;
}

void main(void) {
	uint8_t buf_idx = 0, cmd_read_loop = 1;
	char ch; // Char buffer for data read from console

	// Do basic system initialization
	sys_init();

	printf("%s\n\r%s", ANSI_CLRSCR, &str_appname);
	
	// Print the time
	rtc_get(&clk);
	print_rtc(&clk);
	
	delay_ms_ctc(1000);
	disp_clear();
	
	while(1) { // Endless loop
	    memset(cmd_buffer, 0, CMD_BUF_SIZE);
		console_printString(MONITOR_CMD_PROMPT);

		cmd_read_loop = 1;
		buf_idx = 0;
		while(cmd_read_loop) {
		    keypad_tick();
		
		    if(console_dataAvailable()) {
			    ch = getchar(); // Read a char
			    
			    // Turn the letter uppercase for parsing purposes
			    if (ch >= 0x61 && ch <= 0x7A) ch &= 0xDF;
			    
			    if((ch != 0x20) && (ch != 0x08) && (ch != 0x0D) && (ch < 0x30 || (ch > 0x39 && ch < 0x40) || (ch > 0x5A))) continue;
			    if(ch != 0x08) putchar(ch); // The backspace will be handled below

			    switch(ch) {
				    case 0x0D: // CR
					    monitor_parse_command(cmd_buffer, buf_idx);	
					    cmd_read_loop = 0;
					    break;
				    case 0x08: // Backspace
				        cmd_buffer[buf_idx] = 0;
				        if(buf_idx > 0) {
				            putchar(0x08); putchar(' '); putchar(0x08); // Clear the previous char
				            buf_idx--;
				        }
				        break;
				    default:
					    if(buf_idx >= CMD_BUF_SIZE) { // Command exceeded the maximum length, clearing!
						    cmd_read_loop = 0;
						    console_printString(MONITOR_ERR_MSG);
					    } else {
                            if(buf_idx == 0) {
                                if(ch == 0x20) { putchar(0x08); break; };
                                putchar(' ');
                                cmd_buffer[buf_idx++] = ch;
                                cmd_buffer[buf_idx++] = ' ';
                            } else cmd_buffer[buf_idx++] = ch;
					    }
					    break;
			    }

		    }
		}
	}
}

/***/

static void monitor_parse_command(char *cmd, uint8_t idx) {
	uint8_t val, port, qty;
	uint16_t address, length;

	if (!idx) return; // Nothing to execute

    if(cmd[1] != ' ') {
        console_printString(MONITOR_ERR_MSG);
        return;
    }

	switch(cmd[0]) {
	    case 'T':
	        if(idx > 16) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
	    
	        val = 1;
	        for(uint8_t i = 2; i < 16; i+=2) if(!monitor_strIsValidHex8(&cmd[i])) { val = 0; break; };
	        
	        if(val) { // Valid date given, setting the clock
	            // ddMMyyhhmmssdw
	            clk.d = monitor_parseU8(&cmd[2]);
	            clk.M = monitor_parseU8(&cmd[4]);
	            clk.y = monitor_parseU8(&cmd[6]);
	            clk.h = monitor_parseU8(&cmd[8]);
	            clk.m = monitor_parseU8(&cmd[10]);
	            clk.s = monitor_parseU8(&cmd[12]);	
	            clk.dow = monitor_parseU8(&cmd[14]);	                        
	            rtc_set(&clk);
	            
	            printf("\r\nRTC updated!");
	        }
	    
	        // Print the time
	        rtc_get(&clk);
        	print_rtc(&clk);
	        break;
	    case 'U': // XModem upload
	        if(idx > 11) {
	            console_printString(MONITOR_ERR_MSG);	            
	            return;
	        }
	        
	        if(!monitor_strIsValidHex8(&cmd[2]) || !monitor_strIsValidHex8(&cmd[4]) || !monitor_strIsValidHex8(&cmd[7]) || !monitor_strIsValidHex8(&cmd[9])) {
		        console_printString(MONITOR_ERR_MSG);
		        return;
		    }
		    
		    address = monitor_parseU16(&cmd[2]);
		    length = monitor_parseU16(&cmd[7]);
		    
		    disp_clear();
		    disp_print("XMODEM UPLOAD");
		    printf("\n\rXMODEM upload %04X bytes from %04X\n\r", length, address);

			if(!xmodem_upload((uint8_t*)address, length)) console_printString("\n\rUpload failed!\n\r");
			else console_printString("\n\rUpload completed.\n\r");
			
	        disp_clear();
	        
	        break;
		case 'X': // XModem download
		    if(idx > 6) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
		
		    if(!monitor_strIsValidHex8(&cmd[2]) || !monitor_strIsValidHex8(&cmd[4])) {
		        console_printString(MONITOR_ERR_MSG);
		        return;
		    }
		    
		    address = monitor_parseU16(&cmd[2]);
		    if(address < FREE_RAM_START) {
		        printf("\n\rDownloads are allowed only after %04X\n\r", FREE_RAM_START);
		        return;
		    }
		   
		   	disp_clear();
		    disp_print("XMODEM DOWNLOAD");
		    printf("\n\rXMODEM download @%04X\n\r", address);
		   
			if(!xmodem_receive((uint8_t*)address)) console_printString("\n\rDownload failed!\n\r");
			else console_printString("\n\rDownload completed.\n\r");
			
			disp_clear();
			
			break;
		case 'I': // IN
		    if(idx > 4) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
	        
		    if(!monitor_strIsValidHex8(&cmd[2])) {
		            console_printString(MONITOR_ERR_MSG);
		            return;		        
		    }
		    
		    port = monitor_parseU8(&cmd[2]);
			val = monitor_inp(port);
			
			printf("\n\rP:%02X -> %02X\n\r", port, val);
			break;
		case 'R': // READ
		    if(idx > 9) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
	        
		    if(!monitor_strIsValidHex8(&cmd[2]) || !monitor_strIsValidHex8(&cmd[4]) || !monitor_strIsValidHex8(&cmd[7]) || cmd[6] != ' ') {
		        console_printString(MONITOR_ERR_MSG);
		        return;
		    }
		    
		    address = monitor_parseU16(&cmd[2]);
		    qty = monitor_parseU8(&cmd[7]); // Number of blocks
		    
		    monitor_read(address, qty);

			break;
		case 'W': // WRITE
		    if(idx > 9) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
	        
		    if(!monitor_strIsValidHex8(&cmd[2]) || !monitor_strIsValidHex8(&cmd[4]) || !monitor_strIsValidHex8(&cmd[7]) || cmd[6] != ' ') {
		        console_printString(MONITOR_ERR_MSG);
		        return;
		    }
		    address = monitor_parseU16(&cmd[2]);
		    val = monitor_parseU8(&cmd[7]); // Value to write in RAM
		    		    
		   	printf("\n\rA:%04X <- %02X\n\r", address, val);
		    		    		
			*((uint8_t*)address) = val;
			break;
		case 'F': // FILL
		    if(idx > 12) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
	        
		    if(!monitor_strIsValidHex8(&cmd[2]) || !monitor_strIsValidHex8(&cmd[4]) || !monitor_strIsValidHex8(&cmd[7]) || !monitor_strIsValidHex8(&cmd[10]) || cmd[6] != ' ' || cmd[9] != ' ') {
		        console_printString(MONITOR_ERR_MSG);
		        return;
		    }
		    
		    address = monitor_parseU16(&cmd[2]);
		    qty = monitor_parseU8(&cmd[10]); // Number of bytes to fill
		    val = monitor_parseU8(&cmd[7]); // Value to write in RAM
		    
		    if(!qty) {
		        console_printString(MONITOR_ERR_MSG);
		        return;		        
		    }
		    
		    printf("\n\rA:%04X to %04X <- %02X\n\r", address, (address+qty)-1, val);
		    		    
		    while(qty--) *((uint8_t*)address++) = val;
		    		    
		    break;
		case 'J': // JP
		    if(idx > 6) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
	        
		    if(!monitor_strIsValidHex8(&cmd[2]) || !monitor_strIsValidHex8(&cmd[4])) {
		        console_printString(MONITOR_ERR_MSG);
		        return;
		    }
		    
		    disp_clear();
		    
		    address = monitor_parseU16(&cmd[2]);
			printf("\n\rJumping @%04X\n\r", address);
					    
			monitor_jmp((uint8_t*)address);
			break;
		case 'O': // OUT
		    if(idx > 7) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
		
	        if(!monitor_strIsValidHex8(&cmd[2]) || !monitor_strIsValidHex8(&cmd[5]) || cmd[4] != ' ') {
		        console_printString(MONITOR_ERR_MSG);
		        return;		        
		    }
		    
		    port = monitor_parseU8(&cmd[2]);
		    val = monitor_parseU8(&cmd[5]);
		    
		    printf("\n\rP:0x%02X <- 0x%02X\n\r", port, val);
		    
			monitor_outp(port, val);
			break;
	    case 'H': // Help
	        if(idx > 2) {
	            console_printString(MONITOR_ERR_MSG);
		        return;
	        }
	        
	        printf( "\n\rO xx yy          -> Output value yy to port xx" \
	                "\n\rI xx             -> Input from port xx" \
	                "\n\rJ xxxx           -> Jump @xxxx" \
	                "\n\rF xxxx yy zz     -> Fill zz bytes of RAM with yy starting @xxxx" \
	                "\n\rW xxxx yy        -> Write zz @xxxx" \
	                "\n\rR xxxx yy        -> Print yy 16b blocks of RAM starting @xxxx" \
	                "\n\rX xxxx           -> Download data via XMODEM @xxxx" \
	                "\n\rU xxxx yyyy      -> Upload yyyy bytes via XMODEM  from xxxx" \
	                "\n\rT ddMMyyhhmmssdw -> Show or set current date" \
	                "\n\r");
	        break;
		default:
			console_printString(MONITOR_ERR_MSG);
			break;
	}

	return;
}

/*** Monitor Commands ***/

static void print_rtc(rtc_stat *clk) {
    printf("\n\r%02X/%02X/%02X %02X:%02X:%02X (%s)\n\r", clk->d, clk->M, clk->y, clk->h, clk->m, clk->s, rtc_dowName(clk->dow));
}

static void monitor_read(uint16_t address, uint8_t blocks) {
    uint8_t *ptr = (uint8_t*)address;
    
    console_printString("\r\n");
    while(blocks--) {
        printf("0x%04X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
            (uint16_t)ptr, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8],
            ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);
        ptr += 0x10; // Move forward 16 bytes
    }
}

/***/

void pio_isr (void) __interrupt(0x10) {}

