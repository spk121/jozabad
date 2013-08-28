/*
 * File:   action.h
 * Author: mike
 *
 * Created on August 25, 2013, 11:20 AM
 */

#ifndef ACTION_H
#define	ACTION_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <czmq.h>
#include "state.h"
#include "parch_msg.h"

enum _action_t {
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
    a_last = a_y_reset_confirmation,

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

};

typedef enum _action_t action_t;

static const char action_names[][21] = {
    [a_discard] = "DISCARD",
    [a_reset] = "RESET",
    [a_clear] = "CLEAR",
    [a_disconnect] = "DISCONNECT",

    [a_x_connect] = "X_CONNECT",
    [a_x_disconnect] = "X_DISCONNECT",
    [a_x_call_request] = "X_CALL_REQUEST",
    [a_x_call_accepted] = "X_CALL_ACCEPTED",
    [a_x_clear_request] = "X_CLEAR_REQUEST",
    [a_x_clear_confirmation] = "X_CLEAR_CONFIRMATION",
    [a_x_data] = "X_DATA",
    [a_x_rr] = "X_RR",
    [a_x_rnr] = "X_RNR",
    [a_x_reset] = "X_RESET",
    [a_x_reset_confirmation] = "X_RESET_CONFIRMATION",

    [a_y_disconnect] = "Y_DISCONNECT",
    [a_y_call_request] = "Y_CALL_REQUEST",
    [a_y_call_accepted] = "Y_CALL_ACCEPTED",
    [a_y_clear_request] = "Y_CLEAR_REQUEST",
    [a_y_clear_confirmation] = "Y_CLEAR_CONFIRMATION",
    [a_y_data] = "Y_DATA",
    [a_y_rr] = "Y_RR",
    [a_y_rnr] = "Y_RNR",
    [a_y_reset] = "Y_RESET",
    [a_y_reset_confirmation] = "Y_RESET_CONFIRMATION",
};


// FIXME: find more robust method to get max message id.

extern const action_t x_action_table[(size_t)state_max + 1][PARCH_MSG_DISCONNECT_INDICATION + 1];
extern const action_t y_action_table[(size_t)state_max + 1][PARCH_MSG_DISCONNECT_INDICATION + 1];


#ifdef	__cplusplus
}
#endif

#endif	/* ACTION_H */

