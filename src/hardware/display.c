#include "display.h"

#include <hardware/pio.h>

void disp_clear(void) {
    uint8_t digit_counter = DISP_SIZE - 1; // 16 digits
    disp_send_byte(DISP_BUFFER_POINTER | 0x0F); // Set the cursor to the beginning of the line
    
    do {
        disp_send_byte(' '); // Send a space
    } while(digit_counter--);
}

void disp_print(char *str) {
    disp_send_byte(DISP_BUFFER_POINTER | 0x0F);
    for(uint8_t counter = 0; str[counter] && (counter < DISP_SIZE); counter++) {
        disp_send_byte((str[counter] >= 0x61 && str[counter] <= 0x7A) ? (str[counter] - 0x20) : str[counter]); // Make all letters uppercase!
    }
}

