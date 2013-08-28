#define _GNU_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/parch_packet.h"

static const uint32_t packet_classes[] = {
    [0] = 128,
    [4] = 16,
    [5] = 32,
    [6] = 64,
    [7] = 128,
    [8] = 256,
    [9] = 512,
    [10] = 1024,
    [11] = 2048,
    [12] = 4096
};

bool
parch_packet_index_validate(uint8_t i)
{
    if (i == 0)
        return true;
    else if (i >= PACKET_INDEX_MIN || i <= PACKET_INDEX_MAX)
        return true;
    return false;
}

uint8_t
parch_packet_index_apply_default(uint8_t i)
{
    assert (parch_packet_index_validate(i));

    if (i == 0)
        return PACKET_INDEX_DEFAULT;
    return i;
}

uint8_t
parch_packet_index_throttle(uint8_t request, uint8_t limit)
{
    assert (parch_packet_index_validate(request));
    assert (parch_packet_index_validate(limit));
    uint8_t request2 = parch_packet_index_apply_default(request);
    uint8_t limit2 = parch_packet_index_apply_default(limit);
    if (request2 < limit2)
        return request2;
    return limit2;
}

// During a negotiation, you can only move the packet closer to a 128 byte packet.
parch_packet_negotiation_t
parch_packet_index_negotiate(uint8_t request, uint8_t current)
{
    assert (parch_packet_index_validate(request));
    assert (parch_packet_index_validate(current));

    parch_packet_negotiation_t ret;
    uint8_t request2 = parch_packet_index_apply_default(request);
    uint8_t current2 = parch_packet_index_apply_default(current);
    if (current2 >= PACKET_INDEX_NOMINAL && (request2 > current2 || request2 < PACKET_INDEX_NOMINAL)) {
        ret.ok = false;
        ret.index = current2;
    }
    else if (current2 <= PACKET_INDEX_NOMINAL && (request2 < current2 || request2 > PACKET_INDEX_NOMINAL)) {
        ret.ok = false;
        ret.index = current2;
    }
    else {
        ret.ok = true;
        ret.index = request2;
    }
    return ret;
}

uint32_t
parch_packet_bytes(uint8_t i)
{
    assert (parch_packet_index_validate(i));
    uint8_t i2 = parch_packet_index_apply_default(i);
    return packet_classes[i2];
}