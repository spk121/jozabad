/*
 * File:   parch_packet.h
 * Author: mike
 *
 * Created on August 21, 2013, 7:54 PM
 */

#ifndef PARCH_PACKET_H
#define	PARCH_PACKET_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#define PACKET_INDEX_MIN 4            // 16 bytes
#define PACKET_INDEX_MAX 12           // 4  kbytes
#define PACKET_INDEX_DEFAULT 7        // 128 bytes
#define PACKET_INDEX_NOMINAL 7

bool
parch_packet_index_validate(uint8_t i);
uint8_t
parch_packet_index_apply_default(uint8_t i);
uint8_t
parch_packet_index_throttle(uint8_t request, uint8_t limit);
uint8_t
parch_packet_index_throttle(uint8_t request, uint8_t limit);
bool
parch_packet_index_negotiate(uint8_t request, uint8_t current);
uint32_t
parch_packet_bytes(uint8_t i);
void
lcn_packet_max_size_store_init(void);
#ifdef	__cplusplus
}
#endif

#endif	/* PARCH_PACKET_H */

