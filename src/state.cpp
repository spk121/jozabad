#include <cassert>
#include "../include/state.h"
#include "../include/diagnostic.h"

const char state_names[state_last + 1][STATE_NAME_MAX_LEN + 1] = {
    "STATE_READY", "STATE_X_CALL_REQUEST",
    "STATE_Y_CALL_REQUEST", "STATE_DATA_TRANSFER",
    "STATE_CALL_COLLISION", "STATE_X_CLEAR_REQUEST",
    "STATE_Y_CLEAR_REQUEST", "STATE_X_RESET_REQUEST",
    "STATE_Y_RESET_REQUEST"
};

char const *
name (state_t i) {
    assert (validate(i));
    return state_names[i];
}

bool
validate(state_t i) {
    bool ret = true;
    if (i > state_last) {
        diagnostic = d_state_invalid;
        ret =  false;
    }
    return ret;
}
