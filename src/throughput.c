#include "../include/throughput.h"
#include "../include/diagnostic.h"

static const uint32_t throughput_classes[THROUGHPUT_INDEX_MAX + 1] = {
    [3] = 75,
    [4] = 150,
    [5] = 300,
    [6] = 600,
    [7] = 1200,
    [8] = 2400,
    [9] = 4800,
    [10] = 9600,
    [11] = 19200,
    [12] = 48000,
    [13] = 64000,
    [14] = 128000,
    [15] = 192000,
    [16] = 256000,
    [17] = 320000,
    [18] = 384000,
    [19] = 448000,
    [20] = 512000,
    [21] = 576000,
    [22] = 640000,
    [23] = 704000,
    [24] = 768000,
    [25] = 832000,
    [26] = 896000,
    [27] = 960000,
    [28] = 1024000,
    [29] = 1088000,
    [30] = 1152000,
    [31] = 1216000,
    [32] = 1280000,
    [33] = 1344000,
    [34] = 1408000,
    [35] = 1472000,
    [36] = 1536000,
    [37] = 1600000,
    [38] = 1664000,
    [39] = 1728000,
    [40] = 1792000,
    [41] = 1856000,
    [42] = 1920000,
    [43] = 1984000,
    [44] = 2048000,
};

bool
parch_throughput_index_validate(uint8_t i)
{
    if (i == 0)
        return true;
    else if (i < THROUGHPUT_INDEX_MIN) {
        diagnostic = d_throughput_index_too_small;
        return false;
    }
    else if (i > THROUGHPUT_INDEX_MAX) {
        diagnostic = d_throughput_index_too_large;
        return false;
    }
    return true;
}

uint8_t
parch_throughput_index_apply_default(uint8_t i)
{
    assert (parch_throughput_index_validate(i));

    if (i == 0)
        return THROUGHPUT_INDEX_DEFAULT;
    return i;
}

uint8_t
parch_throughput_index_throttle(uint8_t request, uint8_t limit)
{
    assert (parch_throughput_index_validate(request));
    assert (parch_throughput_index_validate(limit));
    uint8_t request2 = parch_throughput_index_apply_default(request);
    uint8_t limit2 = parch_throughput_index_apply_default(limit);
    if (request2 < limit2)
        return request2;
    return limit2;
}

// During a negotiation, you can only reduce the throughput.
bool
parch_throughput_index_negotiate(uint8_t request, uint8_t current)
{
    assert (parch_throughput_index_validate(request));
    assert (parch_throughput_index_validate(current));

    uint8_t request2 = parch_throughput_index_apply_default(request);
    uint8_t current2 = parch_throughput_index_apply_default(current);
    if (request2 > current2) {
        diagnostic = d_throughput_index_invalid_negotiation;
        return false;
    }
    return true;
}

uint32_t
parch_throughput_bps(uint8_t i)
{
    assert (parch_throughput_index_validate(i));
    uint8_t i2 = parch_throughput_index_apply_default(i);
    return throughput_classes[i2];
}
