#include "packet.h"

int rngchk_packet(packet_t p)
{
    if (p < p_16_bytes)
        return -1;
    if (p > p_4_Kbytes)
        return 1;
    return 0;
}
