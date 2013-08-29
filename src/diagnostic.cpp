#include "../include/diagnostic.h"

diagnostic_t diagnostic = d_unknown;

extern const char diagnostic_names[d_last + 1][DIAGNOSTIC_NAME_MAX_LEN + 1] = {
    "D_UNKNOWN",
    "D_ACTION_INVALID",
    "D_DATA_PACKET_NOT_IN_WINDOW",
    "D_DATA_PACKET_OUT_OF_ORDER",
    "D_DATA_PACKET_TOO_LARGE",
    "D_DATA_PACKET_TOO_SMALL",
    "D_DIRECTION_INVALID",
    "D_DIRECTION_INVALID_NEGOTIATION",
    "D_PACKET_INDEX_INVALID_NEGOTIATION",
    "D_PACKET_INDEX_TOO_LARGE",
    "D_PACKET_INDEX_TOO_SMALL",
    "D_STATE_INVALID",
    "D_THROUGHPUT_INDEX_INVALID_NEGOTIATION",
    "D_THROUGHPUT_INDEX_TOO_LARGE",
    "D_THROUGHPUT_INDEX_TOO_SMALL",
    "D_WINDOW_EDGE_OUT_OF_RANGE",
    "D_WINDOW_INVALID_NEGOTIATION",
    "D_WINDOW_TOO_LARGE",
    "D_WINDOW_TOO_SMALL"
};

char const *name(diagnostic_t d) {
    return diagnostic_names[d];
}
    
