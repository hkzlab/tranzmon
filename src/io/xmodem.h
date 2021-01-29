#ifndef  _XMODEM_HEADER_
#define _XMODEM_HEADER_

#include <common_datatypes.h>

uint8_t xmodem_receive(uint8_t* dest);
uint8_t xmodem_upload(uint8_t* source, uint16_t len);

#endif /* _XMODEM_HEADER_ */
