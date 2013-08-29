#include <cassert>
#include <cinttypes>

#include "../include/packet.h"
#include "../include/diagnostic.h"


const uint16_t packet_bytes[p_last + 1] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
};

const char packet_names[p_last + 1][PACKET_NAME_MAX_LEN] = {
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "16 bytes",
    "32 bytes",
    "64 bytes",
    "128 bytes",
    "256 bytes",
    "512 bytes",
    "1 Kbyte",
    "2 Kbytes",
    "4 Kbytes"
};

char const * const
name(packet_t i)
{
    assert(validate(i));
    return packet_names[i];
}

bool
validate(packet_t i)
{
    if (i == p_unspecified)
        return true;
    else if (i < p_16_bytes) {
        diagnostic = d_packet_index_too_small;
        return false;
    }
    else if (i > p_last) {
        diagnostic = d_packet_index_too_large;
        return false;
    }
    return true;
}

packet_t
apply_default(packet_t i)
{
    assert (validate(i));

    if (i == p_unspecified)
        return p_default;
    return i;
}

packet_t
throttle(packet_t request, packet_t limit)
{
    assert (validate(request));
    assert (validate(limit));
    packet_t request2 = apply_default(request);
    packet_t limit2   = apply_default(limit);
    if (request2 < limit2)
        return request2;
    return limit2;
}

// During a negotiation, you can only move the packet closer to a 128 byte packet.
bool
negotiate(packet_t request, packet_t current)
{
    assert (validate(current));

    packet_t request2 = apply_default(request);
    packet_t current2 = apply_default(current);
    if (current2 >= p_default && (request2 > current2 || request2 < p_default)) {
        diagnostic = d_packet_index_invalid_negotiation;
        return false;
    }
    else if (current2 <= p_default && (request2 < current2 || request2 > p_default)) {
        diagnostic = d_packet_index_invalid_negotiation;
        return false;
    }
    return true;
}

uint16_t
bytes(packet_t i)
{
    assert (validate(i));
    packet_t i2 = apply_default(i);
    return packet_bytes[i2];
}
