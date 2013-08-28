#include "../include/state.h"
#include "../include/diagnostic.h"

const char state_names[][25] = {
    [state_ready]           = "STATE_READY",
    [state_x_call_request]  = "STATE_X_CALL_REQUEST",
    [state_data_transfer]   = "STATE_DATA_TRANSFER",
    [state_call_collision]  = "STATE_CALL_COLLISION",
    [state_x_clear_request] = "STATE_X_CLEAR_REQUEST",
    [state_y_clear_request] = "STATE_Y_CLEAR_REQUEST",
    [state_x_reset_request] = "STATE_X_RESET_REQUEST",
    [state_y_reset_request] = "STATE_Y_RESET_REQUEST",
};

bool
parch_state_validate(state_t i) {
    if ((i < state_min) || (i > state_max)) {
        diagnostic = d_state_invalid;
        return false;
    }
    return true;
}
