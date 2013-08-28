#include "../include/flow.h"
#include "../include/window.h"
#include <stdbool.h>
#include <stdint.h>

flow_t
flow_reset (flow_t f)
{
    f.x_lower_window_edge = 0;
    f.y_lower_window_edge = 0;
    f.x_send_sequence = 0;
    f.y_send_sequence = 0;
    return f;
}

flow_t
flow_init ()
{
    flow_t f;
    f = flow_reset(f);
    f.window_size = WINDOW_DEFAULT;
    return f;
}

bool
flow_sequence_in_range(uint16_t sequence, uint16_t lower_window_edge, uint16_t window_size) {
    uint32_t s = sequence;
    uint32_t lwe = lower_window_edge;
    uint32_t ws = window_size;
    if (s >= lwe && s <= lwe + ws)
        return true;
    return false;
}

