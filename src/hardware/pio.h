#ifndef _PIO_HEADER_
#define _PIO_HEADER_

#include <common_datatypes.h>

void pio_init(void);
void disp_init(void);
void disp_send_byte(uint8_t data);

void kb_selectColumn(uint8_t col);
uint8_t kb_readRows(void);

#endif /* _PIO_HEADER_ */
