#include "../include/flow.h"
#include "../include/window.h"

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
sequence_in_x_window(flow_t *f, uint16_t sequence);
    uint32_t s = sequence;
    uint32_t lwe = f->lower_window_edge;
    uint32_t ws = f->window_size;
    if (s >= lwe && s < lwe + ws)
        return true;
    return false;
}

