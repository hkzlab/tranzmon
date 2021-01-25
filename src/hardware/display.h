#ifndef _DISPLAY_HEADER_
#define _DISPLAY_HEADER_

#include <common_datatypes.h>

#define DISP_SIZE 16

#define DISP_BUFFER_POINTER 0xA0 // The least-significant nibble sets the position of the character to be changed
#define DISP_DIGCNT_CTRL    0xC0 // The least-significant nibble sets how many characters to output
#define DISP_DUTY_CYCLE     0xE0 // The 5 least significant bits set the duty cycle of the display

void disp_clear(void);
void disp_print(char *str);

#endif /* _DISPLAY_HEADER_ */
