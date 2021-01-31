#ifndef _CONSOLE_HEADER_
#define _CONSOLE_HEADER_

#include <common_datatypes.h>

void putchar(char ch);
char getchar(void);
void console_printString(char *str);
uint8_t console_dataAvailable(void);

#endif /* _CONSOLE_HEADER_ */
