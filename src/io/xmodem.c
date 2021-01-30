#include "xmodem.h"

#include <stdlib.h>
#include <string.h>

#include <hardware/dart.h>
#include <hardware/ctc.h>

#include "utilities.h"

#define SOH     0x01
#define ETX     0x03
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

static uint8_t xmodem_send_sync(uint16_t timeout);
static uint8_t xmodem_receive_sync(uint8_t tries);
static uint8_t xmodem_recv_pkt(void);
static uint8_t xmodem_check_packet(void);
static void xmodem_packet2ram(uint8_t *ptr);
static uint16_t crc_calc(uint8_t *ptr, int16_t count);

uint8_t xmodem_receive(uint8_t* dest) {
    uint8_t *pkt_dest = dest;

    uint8_t nack_retries = 0xFF;
    uint32_t last_packet = get_tick();
    uint32_t now = 0;
    uint8_t last_pkt_num = 0xFF; // As we start from 0, this should be different from the first we get

    if(!xmodem_receive_sync(XMODEM_DEFAULT_TRIES)) return 0; // No SYNC, time to exit

    while(nack_retries) {
        if(xmodem_recv_pkt()) {
            last_packet = get_tick();

            switch(packet_buf[0]) {
                case ETX: // CTRL-C, immediate failure
                    return 0;
                case SOH:
                    if(!xmodem_check_packet()) { 
                        nack_retries--;
                        dart_write(PORT_B, NACK);
                    } else {
                        nack_retries = 0xFF;
                        if(last_pkt_num != packet_buf[1]) { // Upload this only if it is not a retransmission
                            if((uint16_t)pkt_dest < (uint16_t)dest) {
                                dart_write(PORT_B, NACK);
                                return 0; // Immediate failure, we wrapped around the memory
                            }
                        
                            xmodem_packet2ram(pkt_dest);
                            pkt_dest += XMODEM_DATA_SIZE;
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

uint8_t xmodem_upload(uint8_t* source, uint16_t len) {
    uint8_t response;
    uint8_t pkt_num = 1;
    uint8_t pkt_size = 0;
    uint8_t retries = 0xFF;
    uint32_t now;


    while(len && retries--) {
        response = NACK;
        pkt_size = XMODEM_DATA_SIZE < len ? XMODEM_DATA_SIZE : len;
        memset(packet_buf, 0, XMODEM_PKT_SIZE);
        memcpy(packet_buf + XMODEM_DATA_OFFSET, source, pkt_size);
        
        packet_buf[0] = SOH;
        packet_buf[1] = pkt_num;
        packet_buf[2] = ~pkt_num;
        
        for(uint8_t idx = 3; idx < XMODEM_PKT_SIZE-2; idx++) packet_buf[131] += packet_buf[idx];
        
        for(uint8_t idx = 0; idx < XMODEM_PKT_SIZE-1; idx++) dart_write(PORT_B, packet_buf[idx]);
        now = get_tick();
        
        while((get_tick() - now) < XMODEM_XFER_TIMEOUT) {
            if(dart_dataAvailable(PORT_B)) {
                response = dart_read(PORT_B);
                if(response == ACK) {
                    retries = 0xFF;
                    break;
                } else break;
            }
            
            __asm nop __endasm;
        }
        
        if(response != ACK) continue; // Don't go to the next packet. Resend this!
        
        len -= pkt_size;
        source += pkt_size;
        pkt_num++;
    }
    
    
    // Send the EOT
    if(retries) {
        retries = 0xFF;
        while(retries--) {
            now = get_tick();
            dart_write(PORT_B, EOT); // End of transmission
            while((get_tick() - now) < XMODEM_XFER_TIMEOUT) {
                if(dart_dataAvailable(PORT_B)) {
                    if(dart_read(PORT_B) == ACK)  {
                        dart_write(PORT_B, EOT); 
                        delay_ms_ctc(5000);
                        return 1;
                    } else break;
                }
                
                delay_ms_ctc(100);
            }
        }
        
    }
    
    return 0;
}

static uint8_t xmodem_receive_sync(uint8_t tries) {
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

static uint8_t xmodem_send_sync(uint16_t timeout) {
    uint32_t now = get_tick();
    uint8_t ch;
    
    while((get_tick() - now) < timeout) {
        if(dart_dataAvailable(PORT_B)) {
           ch = dart_read(PORT_B);
           if(ch == SYNC) return 1;
           else return 0; // Got something else...
        }
        
        __asm nop __endasm;
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
            if(!didx && ((data == EOT) || (data == ETX))) break;
            didx++;
        }

        now = get_tick();
        if((now > last_data) && ((now - last_data) > XMODEM_BYTE_TIMEOUT)) return 0; // Transfer timed out
    }

    return 1;
}

static void xmodem_packet2ram(uint8_t *ptr) {
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

