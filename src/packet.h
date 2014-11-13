/*
    packet.h -

    Copyright 2013, 2014 Michael L. Gran <spk121@yahoo.com>

    This file is part of Jozabad.

    Jozabad is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jozabad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jozabad.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file packet.h
 * @brief List of allowed maximum packet sizes
 *
 * The DATA and CALL_REQUEST packets have data frames that are limited
 * to an agreed-upon maximum size.  This enum represents the allowed
 * maximum sizes.
 */


#ifndef JOZA_PKT_H
#define JOZA_PKT_H

#include <stdint.h>

/**
 * @brief This enum holds the maximum allowed packet size.
 *
 * This is the maximum size of the payload of the DATA packets.
 */
typedef enum _packet_t {
    p_unspecified = 0, /**< UNUSED */
    p_reserved = 1,    /**< UNUSED */
    p_reserved2 = 2,   /**< UNUSED */
    p_reserved3 = 3,   /**< UNUSED */
    p_16_bytes = 4,
    p_32_bytes = 5,
    p_64_bytes = 6,
    p_128_bytes = 7,
    p_256_bytes = 8,
    p_512_bytes = 9,
    p_1_Kbyte = 10,
    p_2_Kbytes = 11,
    p_4_Kbytes = 12,

    p_first = p_16_bytes,    /**< Smallest valid packet size */
    p_default = p_128_bytes, /**< Default. Also the packet size of the CALL_REQUEST data packet */
    p_negotiate = p_128_bytes,
    p_last = p_4_Kbytes      /**< Largest valid packet size */
} packet_t;

uint32_t packet_bytes(packet_t x);
int packet_rngchk(packet_t p);
char const *packet_name(packet_t x);
int packet_negotiate(packet_t request, packet_t current);
packet_t packet_throttle(packet_t request, packet_t limit);

#endif
