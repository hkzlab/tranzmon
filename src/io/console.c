#include "console.h"

#include <hardware/dart.h>

// We need to implement this for stdio...
void putchar(char ch);
char getchar(void);

void console_printString(char *str) {
	while(*str) putchar(*(str++));
}

void putchar(char ch) {
    return dart_write(PORT_B, ch);
}

char getchar(void) {
    dart_signalClearToSend_B();
    return dart_read(PORT_B);
}

