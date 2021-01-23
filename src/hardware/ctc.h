#ifndef _CTC_HEADER_
#define _CTC_HEADER_

#include <common_datatypes.h>

void spkr_init(void);
void spkr_beep(uint8_t time_const);

void clk_ser_init(void);
void delay_ms_ctc(uint16_t delay);

#endif /* _CTC_HEADER_ */
