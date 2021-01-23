#ifndef _DART_HEADER_
#define _DART_HEADER_

#include <common_datatypes.h>

typedef enum {
    PORT_A,
    PORT_B
} DART_Port;

void dart_init(void);
uint8_t dart_read(DART_Port port);
void dart_write(DART_Port port, uint8_t data);
uint8_t dart_dataAvailable(DART_Port port);
uint8_t dart_txBufferEmpty(DART_Port port);

#endif /* _DART_HEADER_ */
