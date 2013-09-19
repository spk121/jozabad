#include "state.h"

const char state_names[state_last + 1][STATE_NAME_MAX_LEN + 1] = {
    "READY",
    "X_CALL_REQUEST", "Y_CALL_REQUEST",
    "DATA_TRANSFER",
    "CALL_COLLISION",
    "X_CLEAR_REQUEST", "Y_CLEAR_REQUEST",
    "X_RESET_REQUEST", "Y_RESET_REQUEST"
};

char const *state_name(state_t a)
{
    return state_names[a];
}

