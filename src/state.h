/*
 * File:   parch_state.h
 * Author: mike
 *
 * Created on August 21, 2013, 8:25 PM
 */

#ifndef PARCH_STATE_H_INCLUDE
#define PARCH_STATE_H_INCLUDE

#define STATE_NAME_MAX_LEN (24)

typedef enum _state_t {
    state_ready,
    state_x_call_request,
    state_y_call_request,
    state_data_transfer,
    state_call_collision,
    state_x_clear_request,
    state_y_clear_request,
    state_x_reset_request,
    state_y_reset_request,

    state_last = state_y_reset_request
} state_t;

#define STATE_CLEAR_REQUEST(is_y) ((is_y)?state_y_clear_request:state_x_clear_request)

char const *state_name(state_t a);

#endif

