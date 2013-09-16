/*
    packet.c

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

#include <assert.h>
#include <stdint.h>
#include "packet.h"

#define PACKET_NAME_LEN (9)
packet_t g_packet_threshold = p_last;

static const uint32_t packet_rate[p_last + 1] = {
	1,
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
    4096
};
    

static const char packet_names[p_last + 1][PACKET_NAME_LEN] = {
    "reserved",
    "reserved",
    "reserved",
	"reserved",
	"16 B",
    "32 B",
    "64 B",
    "128 B",
	"256 B",
	"512 B",
    "1 KiB",
	"2 KiB",
	"4 KiB"
};


uint32_t packet_bytes(packet_t x)
{
    assert (packet_rngchk(x) == 0);
    return packet_rate[x];
}

char const *packet_name(packet_t x)
{
    assert (packet_rngchk(x) == 0);
    return packet_names[x];
}

// Checks to see if, in the context of an CALL_REQUEST negotiation, it
// is valid to set the throughput from CURRENT to REQUEST.  Returns
// non-zero if true.
int packet_negotiate(packet_t request, packet_t current)
{
    assert (packet_rngchk(request) == 0);
    assert (packet_rngchk(current) == 0);

    if (request > p_negotiate) {
        if (request <= current)
            return 1;
        else
            return 0;
    }
    else if (request < p_negotiate) {
        if (request >= current)
            return 1;
        else
            return 0;
    }
	/* else request == t_negotiate */
	return 1;
}

// Check the range of the packet value X.  Return -1 if it is too small,
// 0 if it is valid, or 1 if it is too large.
int packet_rngchk(packet_t p)
{
    if (p < p_16_bytes)
        return -1;
    if (p > p_4_Kbytes)
        return 1;
    return 0;
}

// Return the lesser of REQUEST and LIMIT.
packet_t packet_throttle(packet_t request, packet_t limit)
{
    assert (packet_rngchk(request) == 0);
    assert (packet_rngchk(limit) == 0);

    if (request < limit)
        return request;
    return limit;
}