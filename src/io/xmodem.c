#include "xmodem.h"

#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "utilities.h"

#define SOH  0x01
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18

#define CTRLZ 0x1A
#define MAXERR 25


static void flush(void);
static uint8_t checksum(uint8_t *buf);
static uint8_t wait_getData(uint8_t *data, uint8_t secs);
static uint8_t xmodem_mngPkt(uint8_t *dest);

static uint8_t expected_pkt;
static uint8_t pkt_buf[131];


uint8_t xmodem_receive(uint8_t* dest) {
	uint8_t retries = MAXERR;
	uint8_t ch = 0;

	expected_pkt = 0x01; // Setting to the first packet
	
	//n8vem_serio_putch(NAK);
	while(retries--) {
		if(wait_getData(&ch, 10)) {
			switch(ch) {
				case SOH:
					switch(xmodem_mngPkt(dest)) { // Got SOH, time to parse the packet
						case 1: // Got new block
							dest+=128; // Next block
							expected_pkt++;
						case 2: // Re-Got the previous block
							//n8vem_serio_putch(ACK);
							retries = MAXERR;
							break;
						default:
							flush();
							//n8vem_serio_putch(NAK);
							break;
					}
					break;
				case EOT:
					//n8vem_serio_putch(ACK);
					//console_printString("\r\nEOT\r\n");
					return 0;
				default: // Reading garbage?
					flush();
					//n8vem_serio_putch(NAK);
					break;
			}
		} else {
			//n8vem_serio_putch(NAK);
		}
	}

	return 0xFF;
}

static void flush(void) {
	uint8_t val = 1;
	uint8_t del_loop;

	while(val) {
		del_loop = 0xFF;
		while(del_loop--) {
			__asm
				nop
			__endasm;
		}
		//n8vem_serio_getch_nb(&val, 1);
	}
}

static uint8_t checksum(uint8_t *buf) {
	uint8_t val = 0, idx;

	for (idx = 0; idx < 128; idx++)
		val += buf[idx];

	return val;
}

static uint8_t wait_getData(uint8_t *data, uint8_t secs) {
	uint8_t stat;
	//*data = n8vem_serio_getch_nb(&stat, secs);

	return stat;
}

static uint8_t xmodem_mngPkt(uint8_t *dest) {
	uint8_t idx;

	for (idx = 0; idx < 131; idx++) {
		if (!wait_getData(&pkt_buf[idx], 10)) {
				return 0;
		}
	}

	if(pkt_buf[1] != (0xFF - pkt_buf[0])) { // Verify we got the packet number correctly
		return 0;
	}
	
	if (pkt_buf[0] == (expected_pkt - 1)) { // Already got this...
		return 2;
	} else if (pkt_buf[0] == expected_pkt) {
		if(checksum(&pkt_buf[2]) == pkt_buf[130]) { // OK!
			memcpy(dest, pkt_buf+2, 128);

			return 1;
		}
	}
	
	return 0;
}
