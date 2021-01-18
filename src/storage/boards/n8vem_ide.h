#ifndef _N8VEMIDE_HEADER_
#define _N8VEMIDE_HEADER_

#include <common_datatypes.h>

uint8_t n8vem_ide_init(void);
uint8_t n8vem_ide_read(uint8_t *dest, uint8_t sect, uint8_t head, uint8_t cyll, uint8_t cylh);
uint8_t n8vem_ide_reg_rd(uint8_t reg);
void n8vem_ide_reg_wr(uint8_t reg, uint8_t val);

#endif /* _N8VEMIDE_HEADER_ */
