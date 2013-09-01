#include "../include/throughput.h"
#include "../include/diagnostic.h"

const uint32_t throughput_bps[t_last + 1] = {
    0,
    0,
    0,
    75,
    150,
    300,
    600,
    1200,
    2400,
    4800,
    9600,
    19200,
    48000,
    64000,
    128000,
    192000,
    256000,
    320000,
    384000,
    448000,
    512000,
    578000,
    640000,
    704000,
    768000,
    832000,
    896000,
    960000,
    1024000,
    1088000,
    1152000,
    1216000,
    1280000,
    1344000,
    1408000,
    1472000,
    1536000,
    1600000,
    1664000,
    1728000,
    1792000,
    1856000,
    1920000,
    1984000,
    2048000
};

const char throughput_names[t_last + 1][THROUGHPUT_NAME_MAX_LEN + 1] = {
    "reserved",
    "reserved",
    "reserved",
    "75 bps",
    "150 bps",
    "300 bps",
    "600 bps",
    "1.2 kbps",
    "2.4 kbps",
    "4.8 kbps",
    "9.6 kbps",
    "19.2 kbps",
    "48 kbps",
    "64 kbps",
    "128 kbps",
    "192 kbps",
    "256 kbps",
    "320 kbps",
    "384 kbps",
    "448 kbps",
    "512 kbps",
    "578 kbps",
    "640 kbps",
    "704 kbps",
    "768 kbps",
    "832 kbps",
    "896 kbps",
    "960 kbps",
    "1.024 Mbps",
    "1.088 Mbps",
    "1.152 Mbps",
    "1.216 Mbps",
    "1.28 Mbps",
    "1.344 Mbps",
    "1.408 Mbps",
    "1.472 Mbps",
    "1.536 Mbps",
    "1.6 Mbps",
    "1.664 Mbps",
    "1.728 Mbps",
    "1.792 Mbps",
    "1.856 Mbps",
    "1.920 Mbps",
    "1.984 Mbps",
    "2.048 Mbps"
};


bool
validate(throughput_t i)
{
    if (i == t_unspecified)
        return true;
    else if (i < t_75bps) {
        diagnostic = d_throughput_index_too_small;
        return false;
    }
    else if (i > t_last) {
        diagnostic = d_throughput_index_too_large;
        return false;
    }
    return true;
}

throughput_t
apply_default(throughput_t i)
{
    assert (validate(i));

    if (i == t_unspecified)
        return t_default;
    return i;
}

throughput_t
throttle(throughput_t request, throughput_t limit)
{
    assert (validate(request));
    assert (validate(limit));
    throughput_t request2 = apply_default(request);
    throughput_t limit2   = apply_default(limit);
    if (request2 < limit2)
        return request2;
    return limit2;
}

char const *
name(throughput_t i) {
    assert (validate (i));
    return throughput_names[i];
}

// During a negotiation, you can only reduce the throughput.
bool
negotiate(throughput_t request, throughput_t current)
{
    assert (validate(request));
    assert (validate(current));

    throughput_t request2 = apply_default(request);
    throughput_t current2 = apply_default(current);
    if (request2 > current2) {
        diagnostic = d_throughput_index_invalid_negotiation;
        return false;
    }
    return true;
}

uint32_t
bps(throughput_t i) {
    assert (validate(i));
    throughput_t i2 = apply_default(i);
    return throughput_bps[i2];
}
