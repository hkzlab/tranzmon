#ifndef _CLOCK_HEADER_
#define _CLOCK_HEADER_

#include <common_datatypes.h>

typedef struct {
    uint8_t d;
    uint8_t M;
    uint8_t y;
    
    uint8_t h;
    uint8_t m;
    uint8_t s;
    
    uint8_t dow; // Weeks starts at 0 -> Sunday
} clock_stat;

void clock_init(void);
void clock_set(clock_stat *cs);
void clock_get(clock_stat *cs);
char *clock_dowName(uint8_t dow);

#endif /* _CLOCK_HEADER_ */
