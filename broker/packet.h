/*
    pkt.h - 

    Copyright 2013 Michael L. Gran <spk121@yahoo.com>

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

#ifndef JOZA_PKT_H
#define JOZA_PKT_H

/* This enum holds the maximum allowed packet size. */
typedef enum _packet_t {
    p_unspecified = 0,
    p_reserved = 1,
    p_reserved2 = 2,
    p_reserved3 = 3,
    p_16_bytes = 4,
    p_32_bytes = 5,
    p_64_bytes = 6,
    p_128_bytes = 7,
    p_256_bytes = 8,
    p_512_bytes = 9,
    p_1_Kbyte = 10,
    p_2_Kbytes = 11,
    p_4_Kbytes = 12,
        
    p_first = p_16_bytes,
    p_default = p_128_bytes,
	p_negotiate = p_128_bytes,
    p_last = p_4_Kbytes
} packet_t;

uint32_t packet_bytes(packet_t x);
int packet_rngchk(packet_t p);
int packet_negotiate(packet_t request, packet_t current);

#endif
