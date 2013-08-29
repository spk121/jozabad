/*
 * File:   parch_packet.h
 * Author: mike
 *
 * Created on August 21, 2013, 7:54 PM
 */

#ifndef PARCH_PACKET_H
#define	PARCH_PACKET_H

#define PACKET_NAME_MAX_LEN 11

typedef enum _packet_t {
    p_unspecified = 0,
    p_reserved = 1,
    p_reserved2 = 2,
    p_reserved3 = 3,
    p_16_bytes,
    p_32_bytes,
    p_64_bytes,
    p_128_bytes,
    p_256_bytes,
    p_512_bytes,
    p_1_Kbyte,
    p_2_Kbytes,
    p_4_Kbytes,

    p_default = p_128_bytes,
    p_last = p_4_Kbytes
} packet_t;

extern const uint16_t packet_bytes[p_last + 1];
extern const char packet_names[p_last + 1][PACKET_NAME_MAX_LEN];

bool
validate(packet_t i);
char const *
name(packet_t i);
packet_t
apply_default(packet_t i);
packet_t
throttle(packet_t request, packet_t limit);
bool
negotiate(packet_t request, packet_t current);
uint16_t
bytes(packet_t i);

#endif	/* PARCH_PACKET_H */

