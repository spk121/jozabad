#include "joza_lib.h"

const char cause_names[c_last + 2][44] = {
    "unspecified",
    "worker originated",
    "number busy",
    "out of order",
    "remote procedure error",
    "reverse charging acceptance not subscribed",
    "incompatible destination",
    "fast select acceptance not subscribed",
    "ship absent",
    "invalid facility request",
    "access barred",
    "local procedure error",
    "network congestion",
    "not obtainable",
    "ROA out of order",
    "invalid cause"
};

const char diagnostic_names[d_last + 2][75] = {
    "unspecified",
    "invalid P(S)",
    "invalid P(R)",

    "packet type invalid",
    "packet type invalid for state s1",
    "packet type invalid for state s2",
    "packet type invalid for state s3",
    "packet type invalid for state s4",
    "packet type invalid for state s5",
    "packet type invalid for state s6",
    "packet type invalid for state s7",
    "packet type invalid for state s8",
    "packet type invalid for state s9",

    "packet not allowed",
    "unidentifiable packet",
    "call on one-way logical channel",
    "invalid packet type on a permanent virtual circuit",
    "packet on unassigned logical channel",
    "reject not subscribed to"
    "packet too short",
    "packet too long",
    "invalid general format identifier",
    "restart packet with non-zero logical channel",
    "packet type not compatible with facility",
    "unauthorized interrupt confirmation",
    "unauthorized interrupt",
    "unauthorized reject"
    "TOA/NPI address subscription facility not subscribed to",
    
    "time expired",
    "time expired for incoming call",
    "time expired for clear indication",
    "time expired for reset indication",
    "time expired for restart indication",
    "time expired for call deflection",
    
    "call set-up or clearing problem",
    "facility code not allowed",
    "facility parameter not allowed",
    "invalid called address",
    "invalid calling address",
    "incoming call barred",
    "no logical channel available",
    "call collision",
    "duplicate facility request",
    "non-zero address length",
    "non-zero facility length",
    "facility no provided when expected",
    "maximum number of call redirections exceeded",

    "miscellaneous problem",
    "improper cause code from worker",
    "not aligned octet",
    "inconsistent Q-bit setting",
    "NUI problem",
    "ICRD problem",

    "not assigned problem",
    
    "international problem",
    "remote network problem",
    "international protocol problem",
    "international link out of order",
    "international link busy",
    "transit network facility problem",
    "remote network facility problem",
    "international routing problem",
    "temporary routing problem",
    "unknown called DNIC",
    "maintenance action",

    "invalid diagnostic"
};

const char direction_name_arr[direction_last + 2][23] = {
    "bidirectional",
    "incoming calls barred",
    "outgoing calls barred",
    "calls barred",
    "invalid direction"
};

const uint32_t packet_byte_arr[p_last + 1] = {
    0, 0, 0, 0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
};
    
const char packet_names[p_last + 2][21] = {
    "unspecified",
    "reserved",
    "reserved",
    "reserved",
    "16 b",
    "32 b",
    "64 b",
    "128 b",
    "256 b",
    "512 b",
    "1 kb",
    "2 kb",
    "4 kb",
    "invalid packet size"
};

const uint32_t throughput_bps_arr[t_last + 1] = {
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

const char throughput_names[t_last + 2][20] = {
    "unspecified",
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
    "1.344 Mpbs",
    "1.408 Mbps",
    "1.472 Mbps",
    "1.536 Mbps",
    "1.6 Mbps",
    "1.664 Mbps",
    "1.728 Mbps",
    "1.856 Mbps",
    "1.928 Mbps",
    "1.984 Mbps",
    "2.048 Mbps",
    "invalid throughput"
};

char const *cause_name(cause_t c) {
    if (c > c_last)
        return cause_names[c_last + 1];
    return cause_names[c];
}

char const *diagnostic_name(diagnostic_t d) {
    if (d > d_last)
        return diagnostic_names[d_last + 1];
    return diagnostic_names[d];
}

char const *direction_name(direction_t d) {
    if (d > direction_last)
        return direction_name_arr[direction_last + 1];
    return direction_name_arr[d];
}

bool direction_validate (direction_t d) {
    return (d <= direction_last);
}

// Returns a directionality from 'r' that isn't less strict than 'c'
direction_t direction_negotiate (direction_t r, direction_t c, bool *valid) {
    direction_t ret;
    if (c == direction_io_barred && r != direction_io_barred) {
        ret = c;
        *valid = false;
    }
    else if ((c == direction_input_barred)
             && (r == direction_bidirectional
                 || r == direction_output_barred)) {
        *valid = false;
        ret = c;
    }
    else if ((c == direction_output_barred)
             && (r == direction_bidirectional
                 || r == direction_input_barred)) {
        *valid = false;
        ret = c;
    }
    else {
        *valid = true;
        ret = r;
    }
    return ret;
}

char const *packet_name(packet_t p) {
    if (p > p_last)
        return packet_names[p_last + 1];
    return packet_names[p];
}

bool packet_validate(packet_t p) {
    if (p < p_first || p > p_last)
        return false;
    return true;
}

packet_t packet_throttle (packet_t request, packet_t limit) {
    packet_t R, L;
    R = request;
    L = limit;
    if (R < p_first)
        R = p_first;
    if (R > p_last)
        R = p_last;
    if (L < p_first)
        L = p_first;
    if (L > p_last)
        L = p_last;
    if (R > L)
        R = L;
    return R;
}

packet_t packet_negotiate (packet_t request, packet_t current, bool *valid)
{
    packet_t R, C, ret;
    R = request;
    C = current;
    if (R < p_first)
        R = p_first;
    if (R > p_last)
        R = p_last;
    if (C < p_first)
        C = p_first;
    if (C > p_last)
        C = p_last;

    if (C >= p_default && (R > C || R < p_default)) {
        ret = C;
        if (valid)
            *valid = false;
    }
    else if (C <= p_default && (R < C || R > p_default)) {
        ret = C;
        if (valid)
            *valid = false;
    }
    else {
        ret = R;
        if (valid)
        *valid = true;
    }
    return ret;
}

uint32_t packet_bytes (packet_t p) {
    uint32_t R = p;
    if (R < p_first)
        R = p_first;
    if (R > p_last)
        R = p_last;
    return packet_byte_arr[R];
}

char const *throughput_name(throughput_t t) {
    if (t > t_last)
        return throughput_names[t_last + 1];
    return throughput_names[t];
}

bool throughput_validate(throughput_t p) {
    if (p < t_first || p > t_last)
        return false;
    return true;
}

throughput_t throughput_throttle (throughput_t request, throughput_t limit) {
    throughput_t R, L;
    R = request;
    L = limit;
    if (R < t_first)
        R = t_first;
    if (R > t_last)
        R = t_last;
    if (L < t_first)
        L = t_first;
    if (L > t_last)
        L = t_last;
    if (R > L)
        R = L;
    return R;
}

/* Checks to see if requested throughput 'r' is an acceptable
   modification from the current throughput value 'c'. */
throughput_t throughput_negotiate (throughput_t r, throughput_t c, bool *valid)
{
    throughput_t R, C, ret;
    R = r;
    C = c;
    if (R < t_first)
        R = t_first;
    if (R > t_last)
        R = t_last;
    if (C < t_first)
        C = t_first;
    if (C > t_last)
        C = t_last;

    if (C >= t_default && (R > C || R < t_default)) {
        ret = C;
        if (valid)
            *valid = false;
    }
    else if (C <= t_default && (R < C || R > t_default)) {
        ret = C;
        if (valid)
            *valid = false;
    }
    else {
        ret = R;
        if (valid)
        *valid = true;
    }
    return ret;
}

uint32_t throughput_bps (throughput_t p) {
    throughput_t R = p;
    if (R < t_first)
        R = t_first;
    if (R > t_last)
        R = t_last;
    return throughput_bps_arr[R];
}

bool window_validate(uint16_t p) {
    if (p < WINDOW_MIN || p > WINDOW_MAX)
        return false;
    return true;
}

uint16_t window_throttle (uint16_t request, uint16_t limit) {
    uint16_t R, L;
    R = request;
    L = limit;
    if (R < WINDOW_MIN)
        R = WINDOW_MIN;
    if (R > WINDOW_MAX)
        R = WINDOW_MAX;
    if (L < WINDOW_MIN)
        L = WINDOW_MIN;
    if (L > WINDOW_MAX)
        L = WINDOW_MAX;
    if (R > L)
        R = L;
    return R;
}

uint16_t window_negotiate (uint16_t request, uint16_t current, bool *valid)
{
    uint16_t R, C, ret;
    R = request;
    C = current;
    if (R < WINDOW_MIN)
        R = WINDOW_MIN;
    if (R > WINDOW_MAX)
        R = WINDOW_MAX;
    if (C < WINDOW_MIN)
        C = WINDOW_MIN;
    if (C > WINDOW_MAX)
        C = WINDOW_MAX;

    if (C >= WINDOW_DEFAULT && (R > C || R < WINDOW_DEFAULT)) {
        ret = C;
        if (valid)
            *valid = false;
    }
    else if (C <= WINDOW_DEFAULT && (R < C || R > WINDOW_DEFAULT)) {
        ret = C;
        if (valid)
            *valid = false;
    }
    else {
        ret = R;
        if (valid)
        *valid = true;
    }
    return ret;
}

bool address_validate(const char *str) {
    bool safe = true;
    size_t len = strlen(str);
    if (len == 0 || len > 16)
        safe = false;
    else {
        for (size_t i = 0; i < len; i++) {
            if ((unsigned char) str[i] >= (unsigned char) 128 || (unsigned char) str[i] <= (unsigned char) 31)
                safe = false;
        }
        if (str[0] == ' ' || str[len - 1] == ' ')
            safe = false;
    }
    return safe;

}

zframe_t *zframe_const_dup(const zframe_t *f) {
	return zframe_dup((zframe_t *)f);
}
