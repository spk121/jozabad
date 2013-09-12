/*
 * File:   action.h
 * Author: mike
 *
 * Created on August 25, 2013, 11:20 AM
 */

#ifndef ACTION_H
#define	ACTION_H

#include <czmq.h>

#define ACTION_NAME_MAX_LEN (21)

enum class State {
    ready,
    x_call_request,
    y_call_request,
    data_transfer,
    call_collision,
    x_clear_request,
    y_clear_request,
    x_reset_request,
    y_reset_request,

    last = y_reset_request
};


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
} action_t;

char const* name(action_t a);
bool        validate(action_t a);
action_t    find_action(State s, int msg_id, bool is_y);

#endif	/* ACTION_H */

