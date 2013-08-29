/*
 * File:   action.h
 * Author: mike
 *
 * Created on August 25, 2013, 11:20 AM
 */

#ifndef ACTION_H
#define	ACTION_H

#include <czmq.h>
#include "state.h"
#include "msg.h"

#define ACTION_NAME_MAX_LEN (21)

typedef enum _action_t {
    // These are actions takes by the broker itself.
    a_unspecified = 0,
    a_discard,
    a_reset,
    a_clear,
    a_disconnect,

    // These are actions requested by the node
    a_x_connect,
    a_x_disconnect,
    a_x_call_request,
    a_x_call_accepted,
    a_x_call_collision,
    a_x_clear_request,
    a_x_clear_confirmation,
    a_x_data,
    a_x_rr,
    a_x_rnr,
    a_x_reset,
    a_x_reset_confirmation,

    // These are actions request by the peer
    a_y_disconnect,
    a_y_call_request,
    a_y_call_accepted,
    a_y_call_collision,
    a_y_clear_request,
    a_y_clear_confirmation,
    a_y_data,
    a_y_rr,
    a_y_rnr,
    a_y_reset,
    a_y_reset_confirmation,
    a_last = a_y_reset_confirmation

    //    s0_initialize_timeout,
    //    s2_x_call_waiting_timeout,
    //    s3_y_call_waiting_timeout,
    //    s6_x_clear_request_timeout,
    //    s7_y_clear_request_first_timeout,
    //    s7_y_clear_request_second_timeout,
    //    s8_x_reset_request_first_timeout,
    //    s8_x_reset_request_second_timeout,
    //    s9_y_reset_request_first_timeout,
    //    s9_y_reset_request_second_timeout,
} action_t;

extern const char action_names[(size_t) a_last + 1][ACTION_NAME_MAX_LEN + 1];
extern const action_t x_action_table[(size_t)state_last + 1][MSG_COUNT];
extern const action_t y_action_table[(size_t)state_last + 1][MSG_COUNT];

char const *
    name(action_t a);

bool
    validate(action_t a);

#endif	/* ACTION_H */

