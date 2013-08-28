#define _GNU_SOURCE
#include "../include/packet.h"
#include "../include/parch_channel.h"
#include "../include/diagnostic.h"

static const uint32_t packet_classes[] = {
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

uint8_t lcn_packet_max_size_store[LOGICAL_CHANNEL_MAX + 1];

bool
parch_packet_index_validate(uint8_t i)
{
    if (i == 0)
        return true;
    else if (i < PACKET_INDEX_MIN) {
        diagnostic = d_packet_index_too_small;
        return false;
    }
    else if (i > PACKET_INDEX_MAX) {
        diagnostic = d_packet_index_too_large;
        return false;
    }
    return true;
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
    uint8_t limit2   = parch_packet_index_apply_default(limit);
    if (request2 < limit2)
        return request2;
    return limit2;
}

// During a negotiation, you can only move the packet closer to a 128 byte packet.
bool
parch_packet_index_negotiate(uint8_t request, uint8_t current)
{
    assert (parch_packet_index_validate(request));
    assert (parch_packet_index_validate(current));

    uint8_t request2 = parch_packet_index_apply_default(request);
    uint8_t current2 = parch_packet_index_apply_default(current);
    if (current2 >= PACKET_INDEX_NOMINAL && (request2 > current2 || request2 < PACKET_INDEX_NOMINAL)) {
        diagnostic = d_packet_index_invalid_negotiation;
        return false;
    }
    else if (current2 <= PACKET_INDEX_NOMINAL && (request2 < current2 || request2 > PACKET_INDEX_NOMINAL)) {
        diagnostic = d_packet_index_invalid_negotiation;
        return false;
    }
    return true;
}

uint32_t
parch_packet_bytes(uint8_t i)
{
    assert (parch_packet_index_validate(i));
    uint8_t i2 = parch_packet_index_apply_default(i);
    return packet_classes[i2];
}

void
lcn_packet_max_size_store_init()
{
    for (int i = LOGICAL_CHANNEL_MIN; i <= LOGICAL_CHANNEL_MAX; i++) {
        lcn_packet_max_size_store[i] = PACKET_INDEX_DEFAULT;
    }
}
