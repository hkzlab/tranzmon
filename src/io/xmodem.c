#include "xmodem.h"

#include <stdlib.h>
#include <string.h>

#include <hardware/dart.h>
#include <hardware/ctc.h>

#include "console.h"
#include "utilities.h"

#define SOH     0x01
#define EOT     0x04
#define ACK     0x06
#define NACK    0x15
#define SYNC    0x43

#define XMODEM_DATA_OFFSET 3
#define XMODEM_DATA_SIZE 128
#define XMODEM_PKT_SIZE (XMODEM_DATA_SIZE+5)

#define XMODEM_DEFAULT_TRIES 20
#define XMODEM_XFER_TIMEOUT 5000 // 5 secs without transferred bytes
#define XMODEM_BYTE_TIMEOUT 1000 // 1 sec without transferred bytes

static uint8_t packet_buf[XMODEM_PKT_SIZE];

static uint8_t xmodem_sync(uint8_t tries);
static uint8_t xmodem_recv_pkt(void);
static uint8_t xmodem_check_packet(void);
static void xmodem_upload_packet(uint8_t *ptr);
static uint16_t crc_calc(uint8_t *ptr, int16_t count);

uint8_t xmodem_receive(uint8_t* dest) {
    uint8_t *pkt_dest = dest;

    uint8_t nack_retries = 0xFF;
    uint32_t last_packet = get_tick();
    uint32_t now = 0;
    uint8_t last_pkt_num = 0xFF; // As we start from 0, this should be different from the first we get

    if(!xmodem_sync(XMODEM_DEFAULT_TRIES)) return 0; // No SYNC, time to exit

    while(nack_retries) {
        if(xmodem_recv_pkt()) {
            last_packet = get_tick();

            switch(packet_buf[0]) {
                case SOH:
                    if(!xmodem_check_packet()) { 
                        nack_retries--;
                        dart_write(PORT_B, NACK);
                    } else {
                        nack_retries = 0xFF;
                        if(last_pkt_num != packet_buf[1]) { // Upload this only if it is not a retransmission
                            //xmodem_upload_packet(pkt_dest);
                            //pkt_dest += XMODEM_DATA_SIZE;
                            last_pkt_num = packet_buf[1] & 0xFF;
                        }
                        dart_write(PORT_B, ACK);
                    }
                    break;
                case EOT: // Transfer completed
                    dart_write(PORT_B, ACK);
                    return 1;
                default: // Unknown packet...
                    dart_write(PORT_B, NACK);
                    nack_retries--;
                    break;
            }
        } else dart_write(PORT_B, NACK);

        now = get_tick();
        if((now > last_packet) && ((now - last_packet) > XMODEM_XFER_TIMEOUT)) { nack_retries--; dart_write(PORT_B, NACK); };
    }

    return 0;
}

static uint8_t xmodem_sync(uint8_t tries) {
    uint32_t now;

    while(tries--) {
        now = get_tick();
    
        dart_write(PORT_B, SYNC);
        while((get_tick() - now) < 3000) { // Wait 3 seconds before putting out another SYNC
            if(dart_dataAvailable(PORT_B)) return 1; // Got something!!!
        }
    }

    return 0;
}

static uint8_t xmodem_check_packet(void) {
    uint16_t crc = crc_calc(&packet_buf[XMODEM_DATA_OFFSET], XMODEM_DATA_SIZE);
    uint16_t calc_crc = ((uint16_t)packet_buf[131]) << 8 | packet_buf[132];

    if(crc != calc_crc) return 0; // Corrupted

    return 1;
}

static uint8_t xmodem_recv_pkt(void) {
    uint8_t didx = 0;
    uint8_t data = 0;
    
    uint32_t last_data = get_tick();
    uint32_t now = 0;

    while(didx < XMODEM_PKT_SIZE) {
        if(dart_dataAvailable(PORT_B)) {
            last_data = get_tick();

            data = dart_read(PORT_B);
            packet_buf[didx] = data;
            if(!didx && (data == EOT)) break;
            didx++;
        }

        now = get_tick();
        if((now > last_data) && ((now - last_data) > XMODEM_BYTE_TIMEOUT)) return 0; // Transfer timed out
    }

    return 1;
}

static void xmodem_upload_packet(uint8_t *ptr) {
    for(uint8_t idx = 0; idx < XMODEM_DATA_SIZE; idx++) ptr[idx] = packet_buf[idx];
}


static uint16_t crc_calc(uint8_t *ptr, int16_t count) {
    uint16_t crc;
    uint8_t i;

    crc = 0;
    while (--count >= 0) {
        crc = crc ^ (uint16_t) *ptr++ << 8;
        i = 8;
        do {
            if (crc & 0x8000) crc = crc << 1 ^ 0x1021;
            else crc = crc << 1;
        } while(--i);
    }

    return crc;
}

