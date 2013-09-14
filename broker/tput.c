#include "tput.h"

static const uint32_t tput_rate[t_last + 1] = {
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

static const char tput_names[t_last + 1][11] = {
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

// Check the range of the tput value X.  Return -1 if it is too small,
// 0 if it is valid, or 1 if it is too large.
int tput_rngchk(tput_t x)
{
    if (x < t_75bps)
        return -1;
    else if (x > t_2048kbps)
        return 1;
    return 0;
}

// Return the lesser of REQUEST and LIMIT.  
tput_t tput_throttle(tput_t request, tput_t limit)
{
    assert (tput_rngchk(request) == 0);
    assert (tput_rngchk(limit) == 0);

    if (request < limit)
        return request;
    return limit;
}

char const *tput_name(tput_t x) {
    assert (tput_rngchk(x) == 0);
    return tput_names[x];
}

// Checks to see if, in the context of an CALL_REQUEST negotiation, it
// is valid to set the throughput from CURRENT to REQUEST.  Returns
// TRUE if that is valid.
bool_t tput_negotiate(tput_t request, tput_t current)
{
    assert (tput_rngchk(request) == 0);
    assert (tput_rngchk(current) == 0);

    if (request > t_negotiate) {
        if (request <= current)
            return TRUE;
        else
            return FALSE;
    }
    else if (request == t_negotiate)
        return TRUE;
    else if (request < t_negotiate) {
        if (request >= current)
            return TRUE;
        else
            return FALSE;
    }
}

uint32_t tput_bps(tput_t x) {
    assert (tput_rngchk(x) == 0);
    return tput_rate[x];
}
