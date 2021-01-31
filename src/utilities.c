#include "utilities.h"

#include <hardware/ctc.h>

uint8_t monitor_charIsValidHex4(char c) {
    if((c < 0x30) || (c > 0x39 && c < 0x41) ||
      (c > 0x46 && c < 0x61) || (c > 0x66)) return 0;
      
   return 1;
}

uint8_t monitor_strIsValidHex8(char *str) {
    if(monitor_charIsValidHex4(str[0]) && monitor_charIsValidHex4(str[1])) return 1;

    return 0;
}

uint8_t monitor_parseU4(char ch) {
    uint8_t val = 0;
    
    if ((ch >= 0x61) && (ch <= 0x66)) ch -= 0x20;

    if ((ch >= 0x41) && (ch <= 0x46)) val |= (ch - 55);
    else if ((ch >= 0x30) && (ch <= 0x39)) val |= (ch - 48);
    
    return val;
}

uint8_t monitor_parseU8(char *str) {
	uint8_t val;

    val = monitor_parseU4(str[0]) << 4;
    val |= monitor_parseU4(str[1]);

	return val;
}

uint16_t monitor_parseU16(char *str) {
	return (monitor_parseU8(str+0) << 8) | (monitor_parseU8(str+2) << 0);
}

void monitor_printU8(uint8_t data, char *str) {
	uint8_t idx, val;

	for (idx = 0; idx < 2; idx++) {
		val = (data >> (4 * (1 - idx)) & 0x0F);
		if (val <= 9)
			str[idx] = val + 0x30;
		else
			str[idx] = val + 0x37;
	}
}


// Lifted from the Tranz 330 library utilities
void delay_ms(uint16_t delay) __naked {
    delay;
    
    __asm
		ld hl, #2  ;; First parameter
		add hl, sp ;; Recover address of the parameter in the stack

        ;; Backup registers that get dirty
		push bc
		push af

        ;; Read delay from stack
		ld c,(hl) 
		inc hl
		ld b,(hl)
   
        ;; The actual delay
    delay_ms:
        push bc
        ld bc,#0x0086
    delay_loop:
        dec bc
        ld a,b
        or c
        jr nz,delay_loop
        pop bc
        dec bc
        ld a,b
        or c
        jr nz,delay_ms
    
        ;; Recover registers
        pop af
        pop bc
        ret      
    __endasm;
}

void delay_ms_ctc(uint16_t delay) {
    uint32_t now = get_tick();
    
    while((get_tick()-now) < delay) __asm nop __endasm;
}

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

char nibble_to_hex(uint8_t val) {
    uint8_t nib = val & 0x0F;

    if(nib <= 9) return nib + 0x30;
    else return nib + 0x37;
}

