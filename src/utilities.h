#ifndef _UTILITIES_HEADER_
#define _UTILITIES_HEADER_

#include <common_datatypes.h>

uint8_t monitor_parseU8(char *str);
uint16_t monitor_parseU16(char *str);
void monitor_printU8(uint8_t data, char *str);
uint8_t monitor_strIsValidHex8(char *str);

void delay_ms(uint16_t delay) __naked;
void delay_ms_ctc(uint16_t delay);

void monitor_outp(uint8_t port, uint8_t data);
uint8_t monitor_inp(uint8_t port);
void monitor_jmp(uint8_t *addr);

char nibble_to_hex(uint8_t val);

#endif /* _UTILITIES_HEADER_ */
