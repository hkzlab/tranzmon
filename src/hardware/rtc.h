#ifndef _RTC_HEADER_
#define _RTC_HEADER_

#include <common_datatypes.h>

typedef struct {
    uint8_t d;
    uint8_t M;
    uint8_t y;
    
    uint8_t h;
    uint8_t m;
    uint8_t s;
    
    uint8_t dow; // Weeks starts at 0 -> Sunday
} rtc_stat;

void rtc_init(void);
void rtc_set(rtc_stat *cs);
void rtc_get(rtc_stat *cs);
char *rtc_dowName(uint8_t dow);

#endif /* _RTC_HEADER_ */
