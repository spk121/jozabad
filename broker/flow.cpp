#include <cstdint>
#include "../libjoza/joza_lib.h"
#include "flow.h"

flow_t
reset (flow_t f)
{
    f.x_lower_window_edge = 0;
    f.y_lower_window_edge = 0;
    f.x_send_sequence = 0;
    f.y_send_sequence = 0;
    return f;
}

flow_t
init ()
{
    flow_t f;
    f = reset(f);
    f.window_size = WINDOW_DEFAULT;
    return f;
}

bool
flow_sequence_in_range(uint16_t sequence, uint16_t lower_window_edge, uint16_t window_size)
{
    uint32_t s = sequence;
    uint32_t lwe = lower_window_edge;
    uint32_t ws = window_size;
    if (lwe + ws >= PACKET_SEQUENCE_MODULO) {
        if (s >= lwe || s < ((lwe + ws) % PACKET_SEQUENCE_MODULO))
            return true;
    }
    else if (lwe + ws < PACKET_SEQUENCE_MODULO) {
        if (s >= lwe && s < lwe + ws)
            return true;
    }
    return false;
}

bool
sequence_in_x_window(flow_t f, uint16_t sequence) {
    uint32_t s = sequence;
	uint32_t lwe = f.x_lower_window_edge;
    uint32_t ws = f.window_size;
    if (s >= lwe && s < lwe + ws)
        return true;
    return false;
}

