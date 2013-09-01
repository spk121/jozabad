#include "../include/action.h"
#include "../include/diagnostic.h"
#include "../include/msg.h"

const char action_names[a_last + 1][ACTION_NAME_MAX_LEN + 1] = {
    "UNSPECIFIED", "DISCARD", "RESET", "CLEAR", "DISCONNECT",
    "X_CONNECT", "X_DISCONNECT", "X_CALL_REQUEST", "X_CALL_ACCEPTED",
    "X_CLEAR_REQUEST", "X_CLEAR_CONFIRMATION", "X_DATA", "X_RR",
    "X_RNR" "X_RESET", "X_RESET_CONFIRMATION", "Y_DISCONNECT",
    "Y_CALL_REQUEST", "Y_CALL_ACCEPTED", "Y_CLEAR_REQUEST",
    "Y_CLEAR_CONFIRMATION", "Y_DATA", "Y_RR", "Y_RNR", "Y_RESET",
    "Y_RESET_CONFIRMATION"
};

const action_t x_action_table[state_last + 1][MSG_COUNT] = {
    // state_ready
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_x_call_request, // call_request
        a_clear, // call accepted
        a_x_clear_request, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_x_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_x_call_request
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_x_clear_request, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_x_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_y_call_request
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_clear, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_x_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_data_transfer
    {
        a_x_data, // data
        a_x_rr, // rr
        a_x_rnr, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_x_clear_request, // clear request
        a_clear, // clear confirmation
        a_x_reset, // reset request
        a_reset, // reset confirmation
        a_reset, // connect
        a_reset, // connect indication
        a_x_disconnect, // disconnect
        a_reset, // disconnect indication
    },
    // state_call_collision
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_x_clear_request, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_x_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_x_clear_request
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_discard, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_x_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_y_clear_request
    {
        a_discard, // data
        a_discard, // rr
        a_discard, // rnr
        a_discard, // call request
        a_discard, // call accepted
        a_x_clear_confirmation, // clear request
        a_x_clear_confirmation, // clear confirmation
        a_discard, // reset request
        a_discard, // reset confirmation
        a_discard, // connect
        a_discard, // connect indication
        a_x_disconnect, // disconnect
        a_discard, // disconnect indicatio
    },
    // state_x_reset_request
    {
        a_reset, // data
        a_reset, // rr
        a_reset, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_x_clear_request, // clear request
        a_clear, // clear confirmation
        a_discard, // reset request
        a_reset, // reset confirmation
        a_reset, // connect
        a_reset, // connect indication
        a_x_disconnect, // disconnect
        a_reset, // disconnect indication
    },
    // state_y_reset_request
    {
        a_discard, // data
        a_discard, // rr
        a_discard, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_x_clear_request, // clear request
        a_clear, // clear confirmation
        a_x_reset_confirmation, // reset request
        a_x_reset_confirmation, // reset confirmation
        a_discard, // connect,
        a_discard, // connect indication,
        a_x_disconnect, // disconnect
        a_discard, // disconnect indication
    }
};

const action_t y_action_table[state_last + 1][MSG_COUNT] = {
    // state_ready
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_y_call_request, // call_request
        a_clear, // call accepted
        a_y_clear_request, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_y_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_x_call_request
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_y_call_collision, // call request
        a_y_call_accepted, // call accepted
        a_y_clear_request, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_y_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_y_call_request
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_y_clear_request, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_y_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_data_transfer
    {
        a_y_data, // data
        a_y_rr, // rr
        a_y_rnr, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_y_clear_request, // clear request
        a_clear, // clear confirmation
        a_y_reset, // reset request
        a_reset, // reset confirmation
        a_reset, // connect
        a_reset, // connect indication
        a_y_disconnect, // disconnect
        a_reset, // disconnect indication
    },
    // state_call_collision
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_y_clear_request, // clear request
        a_clear, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_y_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_x_clear_request
    {
        a_clear, // data
        a_clear, // rr
        a_clear, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_y_clear_confirmation, // clear request
        a_y_clear_confirmation, // clear confirmation
        a_clear, // reset request
        a_clear, // reset confirmation
        a_clear, // connect
        a_clear, // connect indication
        a_y_disconnect, // disconnect
        a_clear, // disconnect indication
    },
    // state_y_clear_request
    {
        a_discard, // data
        a_discard, // rr
        a_discard, // rnr
        a_discard, // call request
        a_discard, // call accepted
        a_discard, // clear request
        a_discard, // clear confirmation
        a_discard, // reset request
        a_discard, // reset confirmation
        a_discard, // connect
        a_discard, // connect indication
        a_y_disconnect, // disconnect
        a_discard, // disconnect indicatio
    },
    // state_x_reset_request
    {
        a_discard, // data
        a_discard, // rr
        a_discard, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_y_clear_request, // clear request
        a_clear, // clear confirmation
        a_y_reset_confirmation, // reset request
        a_y_reset_confirmation, // reset confirmation
        a_reset, // connect
        a_reset, // connect indication
        a_y_disconnect, // disconnect
        a_reset, // disconnect indication
    },
    // state_y_reset_request
    {
        a_reset, // data
        a_reset, // rr
        a_reset, // rnr
        a_clear, // call request
        a_clear, // call accepted
        a_y_clear_request, // clear request
        a_clear, // clear confirmation
        a_discard, // reset request
        a_reset, // reset confirmation
        a_reset, // connect,
        a_reset, // connect indication,
        a_y_disconnect, // disconnect
        a_reset, // disconnect indication
    }
};

char const *
name(action_t a) {
    assert(validate(a));
    return action_names[(size_t) a];
}

bool
validate(action_t a) {
    bool ret = true;
    if (a > a_last) {
        diagnostic = d_action_invalid;
        ret = false;
    }
    return ret;
}

action_t find_action(state_t s, const msg_t *msg, bool is_y) {
    action_t a;
    if (is_y)
        a = y_action_table[s][msg_const_id(msg)];
    else
        a = x_action_table[s][msg_const_id(msg)];
    return a;
}