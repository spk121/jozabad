#include "../include/cause.h"

extern const char cause_names[c_last + 1][CAUSE_NAME_MAX_LEN + 1] = {
    "C_WORKER_ORIGINATED",
    "C_NUMBER_BUSY",
    "C_NOT_OBTAINABLE",
    "C_OUT_OF_ORDER",
    "C_LOCAL_PROCEDURE_ERROR",
    "C_REMOTE_PROCEDURE_ERROR",
    "C_INVALID_FACILITY_REQUEST",
    "C_ACCESS_BARRED",
    "C_NETWORK_CONGESTION"
};

char const *name(cause_t c) {
    return cause_names[c];
}


