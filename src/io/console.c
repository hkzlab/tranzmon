#include "console.h"

// We need to implement this for stdio...
void putchar(char ch);
char getchar(void);

void console_printString(char *str) {
	while(*str) putchar(*(str++));
}

void putchar(char ch) {
#ifdef __SERIAL_CONSOLE__

#endif
}

char getchar(void) {
#ifdef __SERIAL_CONSOLE__

#endif
}

