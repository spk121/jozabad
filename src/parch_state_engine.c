/*  =========================================================================
    <name> - <description>

    -------------------------------------------------------------------------
    Copyright (c) <year> - <company name> - <website>
    Copyright other contributors as noted in the AUTHORS file.

    This file is part of <project name>, <description>
    <website>

    This is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    This software is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANT-
    ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
    Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program. If not see http://www.gnu.org/licenses.
    =========================================================================
 */

#include "parch.h"

//  ---------------------------------------------------------------------------
//  DATA STRUCTURES

enum _action_t {

    // These are actions takes by the broker itself.
    a_error = 0,
    a_first = a_error,
    a_discard = 1,
    a_connect,
    a_service,
    a_reset,

    // These are actions requested by the node
    a_x_disconnect,
    a_x_call_request,
    a_x_call_accepted,
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

enum _event_t {
    no_event = -1,
    event_first = no_event,

    // Node-initiated events
    e_x_connect = 0,
    e_x_service = 1,
    e_x_disconnect = 2,
    e_x_call_request = 3,
    e_x_call_accepted = 4,
    e_x_clear_request = 5,
    e_x_clear_confirmation = 6,
    e_x_data = 7,
    e_x_rr = 8,
    e_x_rnr = 9,
    e_x_reset_request = 10,
    e_x_reset_confirmation = 11,

    // Peer-initiated events
    e_y_disconnect = 12,
    e_y_call_request = 13,
    e_y_call_accepted = 14,
    e_y_clear_request = 15,
    e_y_clear_confirmation = 16,
    e_y_data = 17,
    e_y_rr = 18,
    e_y_rnr = 19,
    e_y_reset_request = 20,
    e_y_reset_confirmation = 21,

    // Broker or state-engine initiated events
    e_reset = 22,
    // e_error = 22,

    max_event = e_reset,
    event_last = e_reset
};

enum _state_t {
    no_state = -1,
    no_state_change = no_state,
    state_first = no_state,
    s0_init = 0,
    s1_ready = 1,
    s2_x_call = 2,
    s3_y_call = 3,
    s4_data = 4,
    s5_collision = 5,
    s6_x_clear = 6,
    s7_y_clear = 7,
    s8_x_reset = 8,
    s9_y_reset = 9,
    max_state = 9,
    state_last = s9_y_reset,
};

struct _parch_state_engine_t {
    broker_t *broker; // loopback to the broker that contains this node
    node_t *node; // loopback to the node that contains this state engine
    //    zframe_t *node_address; // address of node to which this state machine is attached
    //    char *service_name; // node's public self-identification of what it does
    state_t state; // current state in the state machine
    event_t event; // current event being processed by the state machine

    event_t next_event; // next event to be processed by the state machine
    diagnostic_t next_diagnostic; // diagnostic for next event, if required

    uint32_t x_sequence_number; // serial number of last DATA from X
    uint32_t y_sequence_number; // serial number of last DATA from Y
    uint32_t x_window; // serial number of last RR from X for Y
    uint32_t y_window; // serial number of last RR from Y for X
    int x_not_ready; // true if we received a RNR from X
    int y_not_ready; // true if we received a RNR from Y

    parch_msg_t *request; // current message being processed
    // parch_msg_t *reply; // current reply message being generated
    zconfig_t *config;
    int stopped;
};

struct _state_machine_table_element_t {
    action_t action;
    diagnostic_t diagnostic;
    state_t next;
};

//  ---------------------------------------------------------------------------
//  MACROS

#define MAX_SERVICE_NAME_LEN 128

#define STATE_ENGINE_VALIDITY_CHECKS(s) \
do {                                    \
        assert (s != NULL);             \
        assert (s->broker != NULL);      \
        assert (parch_broker_get_verbose(s->broker) == 0 || parch_broker_get_verbose(s->broker) == 1); \
        assert (parch_broker_get_socket(s->broker)); \
        assert (streq(zsocket_type_str(parch_broker_get_socket(s->broker)), "ROUTER")); \
        assert (parch_node_get_state_engine(s->node) == s); \
        assert (parch_node_get_service_name(s->node) != NULL); \
        assert (strlen(parch_node_get_service_name(s->node)) < MAX_SERVICE_NAME_LEN); \
        assert (is_safe_ascii(parch_node_get_service_name(s->node))); \
        assert (s->state >= state_first && s->state <= state_last); \
        assert (s->event >= event_first && s->event <= event_last); \
        assert (s->next_event >= event_first && s->next_event <= event_last); \
        assert (s->x_window <= s->y_sequence_number); \
        assert (s->y_window <= s->x_sequence_number); \
        assert (s->x_not_ready == 0 || s->x_not_ready == 1); \
        assert (s->y_not_ready == 0 || s->y_not_ready == 1); \
        assert (s->request == 0 || s_msg_id_is_valid(parch_msg_id(s->request))); \
        /* assert (s->request == 0 || (parch_msg_address(s->request) != 0)); */ \
        /* assert (s->reply == 0 || is_valid_id(parch_msg_id(s->reply))); */ \
        /* assert (s->reply == 0 || (parch_msg_address(s->reply) != 0)); */ \
} while (0);

//  ---------------------------------------------------------------------------
//  TYPEDEFS
typedef enum _clearing_cause_t clearing_cause_t;
typedef struct _state_machine_table_element_t state_machine_table_element_t;

//  ---------------------------------------------------------------------
//  State machine constants

static const char action_names[][21] = {
    [a_error] = "error",
    [a_discard] = "discard",
    [a_connect] = "connect",
    [a_service] = "service",
    [a_reset] = "reset",

    [a_x_disconnect] = "x_disconnect",
    [a_x_call_request] = "x_call_request",
    [a_x_call_accepted] = "x_call_accepted",
    [a_x_clear_request] = "x_clear_request",
    [a_x_clear_confirmation] = "x_clear_confirmation",
    [a_x_data] = "x_data",
    [a_x_rr] = "x_rr",
    [a_x_rnr] = "x_rnr",
    [a_x_reset] = "x_reset",
    [a_x_reset_confirmation] = "x_reset_confirmation",

    [a_y_disconnect] = "y_disconnect",
    [a_y_call_request] = "y_call_request",
    [a_y_call_accepted] = "y_call_accepted",
    [a_y_clear_request] = "y_clear_request",
    [a_y_clear_confirmation] = "y_clear_confirmation",
    [a_y_data] = "y_data",
    [a_y_rr] = "y_rr",
    [a_y_rnr] = "y_rnr",
    [a_y_reset] = "y_reset",
    [a_y_reset_confirmation] = "y_reset_confirmation",
};

static const char diagnostic_messages[][50] = {
    [err_packet_type_invalid_for_state_s0] = "invalid packet type for s0_init state",
    [err_packet_type_invalid_for_state_s1] = "invalid packet type for s1_ready state",
    [err_packet_type_invalid_for_state_s2] = "invalid packet type for s2_x_call state",
    [err_packet_type_invalid_for_state_s3] = "invalid packet type for s3_y_call state",
    [err_packet_type_invalid_for_state_s4] = "invalid packet type for s4_data state",
    [err_packet_type_invalid_for_state_s5] = "invalid packet type for s5_collision state",
    [err_packet_type_invalid_for_state_s6] = "invalid packet type for s6_x_clear state",
    [err_packet_type_invalid_for_state_s7] = "invalid packet type for s7_y_clear state",
    [err_packet_type_invalid_for_state_s8] = "invalid packet type for s8_x_reset state",
    [err_packet_type_invalid_for_state_s9] = "invalid packet type for s9_y_reset state"
};

static const state_machine_table_element_t state_machine_table[max_state + 1][max_event + 1] = {
    /* S0 Initialize          */
    {
        /* e_x_connect            */
        {a_connect, err_none, s1_ready},
        /* e_x_service            */
        {a_service, err_none, no_state},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_accepted      */
        {a_x_disconnect, err_none, no_state},
        /* e_x_clear_request      */
        {a_x_disconnect, err_none, no_state},
        /* e_x_clear_confirmation */
        {a_x_disconnect, err_none, no_state},
        /* e_x_data               */
        {a_x_disconnect, err_none, no_state},
        /* e_x_rr                 */
        {a_x_disconnect, err_none, no_state},
        /* e_x_rnr                */
        {a_x_disconnect, err_none, no_state},
        /* e_x_reset_request      */
        {a_x_disconnect, err_none, no_state},
        /* e_x_reset_confirmation */
        {a_x_disconnect, err_none, no_state},

        /* e_y_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_y_call_request       */
        {a_x_disconnect, err_none, no_state},
        /* e_y_call_accepted      */
        {a_x_disconnect, err_none, no_state},
        /* e_y_clear_request      */
        {a_x_disconnect, err_none, no_state},
        /* e_y_clear_confirmation */
        {a_x_disconnect, err_none, no_state},
        /* e_y_data               */
        {a_x_disconnect, err_none, no_state},
        /* e_y_rr                 */
        {a_x_disconnect, err_none, no_state},
        /* e_y_rnr                */
        {a_x_disconnect, err_none, no_state},
        /* e_y_reset_request      */
        {a_x_disconnect, err_none, no_state},
        /* e_y_reset_confirmation */
        {a_x_disconnect, err_none, no_state},

        /* e_reset */
        {a_discard, err_none, s0_init}
    },
    /* S1 READY                */
    {
        /* e_x_connect            */
        {a_connect, err_none, s1_ready},
        /* e_x_service            */
        {a_service, err_none, s1_ready},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_x_call_request, err_none, no_state},
        /* e_x_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s6_x_clear},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_x_data               */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_x_rr                 */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_x_rnr                */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_x_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_x_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_y_call_request, err_none, s3_y_call},
        /* e_y_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_y_clear_request      */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_y_data               */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_y_rr                 */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_y_rnr                */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_y_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},
        /* e_y_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s1, s7_y_clear},

        /* e_reset */
        {a_discard, err_none, s1_ready}
    },
    /* S2 X CALL                */
    {
        /* e_x_connect            */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_service            */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s6_x_clear},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_data               */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_rr                 */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_rnr                */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_x_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_y_call_request, err_none, s5_collision},
        /* e_y_call_accepted      */
        {a_y_call_accepted, err_none, s4_data},
        /* e_y_clear_request      */
        {a_y_clear_request, err_none, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_y_data               */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_y_rr                 */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_y_rnr                */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_y_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},
        /* e_y_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s2, s7_y_clear},

        /* e_reset */
        {a_discard, err_none, s2_x_call}
    },
    /* S3 y CALL                */
    {
        /* e_x_connect            */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_x_service            */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_discard, err_none, s5_collision},
        /* e_x_call_accepted      */
        {a_x_call_accepted, err_none, s4_data},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s6_x_clear},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_x_data               */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_x_rr                 */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_x_rnr                */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_x_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_x_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_y_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_y_clear_request      */
        {a_y_clear_request, err_none, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_y_data               */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_y_rr                 */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_y_rnr                */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_y_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},
        /* e_y_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s3, s7_y_clear},

        /* e_reset */
        {a_discard, err_none, s3_y_call}

    },
    /* S4 data                 */
    {
        /* e_x_connect            */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_x_service            */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_x_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s6_x_clear},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_x_data               */
        {a_x_data, err_none, s4_data},
        /* e_x_rr                 */
        {a_x_rr, err_none, s4_data},
        /* e_x_rnr                */
        {a_x_rnr, err_none, s4_data},
        /* e_x_reset_request      */
        {a_x_reset, err_none, s8_x_reset},
        /* e_x_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_y_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_y_clear_request      */
        {a_y_clear_request, err_none, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},
        /* e_y_data               */
        {a_y_data, err_none, s4_data},
        /* e_y_rr                 */
        {a_y_rr, err_none, s4_data},
        /* e_y_rnr                */
        {a_y_rnr, err_none, s4_data},
        /* e_y_reset_request      */
        {a_y_reset, err_none, s9_y_reset},
        /* e_y_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s4, s7_y_clear},

        /* e_reset */
        {a_reset, err_none, s4_data}
    },
    /* S5 COLLISION             */
    {
        /* e_x_connect            */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_service            */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s6_x_clear},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_data               */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_rr                 */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_rnr                */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_x_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_y_call_accepted      */
        {a_y_call_accepted, err_none, s4_data},
        /* e_y_clear_request      */
        {a_y_clear_request, err_none, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_y_data               */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_y_rr                 */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_y_rnr                */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_y_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},
        /* e_y_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s5, s7_y_clear},

        /* e_reset */
        {a_discard, err_none, s5_collision}

    },
    /* S6 X CLEAR                */
    {
        /* e_x_connect            */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_service            */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_clear_request      */
        {a_discard, err_none, s6_x_clear},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_data               */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_rr                 */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_rnr                */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_x_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_discard, err_none, s6_x_clear},
        /* e_y_call_accepted      */
        {a_discard, err_none, s6_x_clear},
        /* e_y_clear_request      */
        {a_y_clear_request, err_none, s1_ready},
        /* e_y_clear_confirmation */
        {a_y_clear_confirmation, err_none, s1_ready},
        /* e_y_data               */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_y_rr                 */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_y_rnr                */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_y_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},
        /* e_y_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s6, s7_y_clear},

        /* e_reset */
        {a_discard, err_none, s6_x_clear}

    },
    /* S7 Y CLEAR                */
    {
        /* e_x_connect            */
        {a_discard, err_none, s7_y_clear},
        /* e_x_service            */
        {a_discard, err_none, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_discard, err_none, s7_y_clear},
        /* e_x_call_accepted      */
        {a_discard, err_none, s7_y_clear},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s1_ready},
        /* e_x_clear_confirmation */
        {a_x_clear_confirmation, err_none, s1_ready},
        /* e_x_data               */
        {a_discard, err_none, s7_y_clear},
        /* e_x_rr                 */
        {a_discard, err_none, s7_y_clear},
        /* e_x_rnr                */
        {a_discard, err_none, s7_y_clear},
        /* e_x_reset_request      */
        {a_discard, err_none, s7_y_clear},
        /* e_x_reset_confirmation */
        {a_discard, err_none, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_discard, err_none, s7_y_clear},
        /* e_y_call_accepted      */
        {a_discard, err_none, s7_y_clear},
        /* e_y_clear_request      */
        {a_discard, err_none, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_discard, err_none, s7_y_clear},
        /* e_y_data               */
        {a_discard, err_none, s7_y_clear},
        /* e_y_rr                 */
        {a_discard, err_none, s7_y_clear},
        /* e_y_rnr                */
        {a_discard, err_none, s7_y_clear},
        /* e_y_reset_request      */
        {a_discard, err_none, s7_y_clear},
        /* e_y_reset_confirmation */
        {a_discard, err_none, s7_y_clear},

        /* e_reset */
        {a_discard, err_none, s7_y_clear}

    },
    /* S8 X RESET                */
    {
        /* e_x_connect            */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_service            */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s1_ready},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_data               */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_rr                 */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_rnr                */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_x_reset_request      */
        {a_discard, err_none, s8_x_reset},
        /* e_x_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_y_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_y_clear_request      */
        {a_y_clear_request, err_none, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s8, s7_y_clear},
        /* e_y_data               */
        {a_discard, err_none, s8_x_reset},
        /* e_y_rr                 */
        {a_discard, err_none, s8_x_reset},
        /* e_y_rnr                */
        {a_discard, err_none, s8_x_reset},
        /* e_y_reset_request      */
        {a_y_reset, err_none, s4_data},
        /* e_y_reset_confirmation */
        {a_y_reset_confirmation, err_none, s4_data},

        /* e_reset */
        {a_discard, err_none, s8_x_reset}
    },
    /* S9 Y RESET                */
    {
        /* e_x_connect            */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_x_service            */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_x_disconnect         */
        {a_x_disconnect, err_none, no_state},
        /* e_x_call_request       */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_x_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_x_clear_request      */
        {a_x_clear_request, err_none, s6_x_clear},
        /* e_x_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_x_data               */
        {a_discard, err_none, s9_y_reset},
        /* e_x_rr                 */
        {a_discard, err_none, s9_y_reset},
        /* e_x_rnr                */
        {a_discard, err_none, s9_y_reset},
        /* e_x_reset_request      */
        {a_x_reset, err_none, s4_data},
        /* e_x_reset_confirmation */
        {a_x_reset_confirmation, err_none, s4_data},

        /* e_y_disconnect         */
        {a_y_disconnect, err_none, s1_ready},
        /* e_y_call_request       */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_y_call_accepted      */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_y_clear_request      */
        {a_y_clear_request, err_none, s7_y_clear},
        /* e_y_clear_confirmation */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_y_data               */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_y_rr                 */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_y_rnr                */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_y_reset_request      */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},
        /* e_y_reset_confirmation */
        {a_error, err_packet_type_invalid_for_state_s9, s7_y_clear},

        /* e_reset */
        {a_discard, err_none, s9_y_reset}
    }
};

static const char state_names[][13] = {
    [s0_init] = "s0_init",
    [s1_ready] = "s1_ready",
    [s2_x_call] = "s2_x_call",
    [s3_y_call] = "s3_y_call",
    [s4_data] = "s4_data",
    [s5_collision] = "s5_collision",
    [s6_x_clear] = "s6_x_clear",
    [s7_y_clear] = "s7_y_clear",
    [s8_x_reset] = "s8_x_reset",
    [s9_y_reset] = "s9_y_reset"
};

//  ---------------------------------------------------------------------------
//  DECLARATIONS

int
s_msg_id_is_valid(int id) __attribute__((const));
static void
s_state_engine_do_connect(parch_state_engine_t *self);
static void
s_state_engine_do_error(parch_state_engine_t *self, diagnostic_t diagnostic) __attribute__((nonnull(1)));
static void
s_state_engine_do_reset(parch_state_engine_t *self, diagnostic_t diagnostic);
static void
s_state_engine_do_service(parch_state_engine_t *self) __attribute__((nonnull(1)));
static void
s_state_engine_do_x_call_accepted(parch_state_engine_t *self) __attribute__((nonnull(1)));
static void
s_state_engine_do_x_call_request(parch_state_engine_t * const self) __attribute__((nonnull(1)));
static void
state_engine_do_x_clear_request(parch_state_engine_t *self);
static void
s_state_engine_do_x_disconnect(parch_state_engine_t *self) __attribute__((nonnull(1)));
static void
s_state_engine_do_x_rnr(parch_state_engine_t *self);
static void
s_state_engine_do_x_rr(parch_state_engine_t *self);
static void
s_state_engine_do_x_rr_internal(parch_state_engine_t *self, int not_ready);
static void
s_state_engine_log(const parch_state_engine_t * const self, char *msg) __attribute__((nonnull(1,2)));
static void
s_state_engine_log_send_to_node(const parch_state_engine_t * const self, const parch_msg_t * const msg, diagnostic_t d);
static void
s_state_engine_log_send_to_peer(const parch_state_engine_t * const self, const parch_msg_t * const msg, diagnostic_t d);
static void
s_state_engine_send_msg_to_node_and_log(parch_state_engine_t *self, parch_msg_t **msg_p);
static void
s_state_engine_send_msg_to_peer_and_log(parch_state_engine_t *self, parch_msg_t **msg_p);
static void
s_state_engine_reset_flow_control(parch_state_engine_t * const self) __attribute__((nonnull(1)));





static int
s_state_engine_y_message(parch_state_engine_t *self, parch_msg_t **msg_p);


// ================================================================
// Lowest-level operations for state machine

int
s_msg_id_is_valid(int id) {
    return (id == PARCH_MSG_DATA
            || id == PARCH_MSG_RR
            || id == PARCH_MSG_RNR
            || id == PARCH_MSG_CALL_REQUEST
            || id == PARCH_MSG_CALL_ACCEPTED
            || id == PARCH_MSG_CLEAR_REQUEST
            || id == PARCH_MSG_CLEAR_CONFIRMATION
            || id == PARCH_MSG_RESET_REQUEST
            || id == PARCH_MSG_RESET_CONFIRMATION
            || id == PARCH_MSG_CONNECT
            || id == PARCH_MSG_SERVICE
            || id == PARCH_MSG_DISCONNECT);
}

static void
s_state_engine_do_connect(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    // Well, we're in the state engine, so obviously this node
    // has a state engine, already.
    // So we need to register its service, which happens in the broker.
    s_state_engine_do_service(self);
}

static void
s_state_engine_do_error(parch_state_engine_t *self, diagnostic_t diagnostic) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    zframe_t *addr = parch_node_get_node_address(self->node);
    parch_msg_t *node_msg = parch_msg_new_clear_request_msg (addr, local_procedure_error, diagnostic);
    s_state_engine_send_msg_to_node_and_log(self, &node_msg);

    node_t *peer = parch_node_get_peer(self->node);
    if (peer) {
        parch_msg_t *peer_msg = parch_msg_new_clear_request_msg (addr, remote_procedure_error, diagnostic);
        s_state_engine_send_msg_to_peer_and_log(self, &peer_msg);
    }
}

// This is the state-engine initiated reset request, where it is flagging a flow control problem
// and requires a reset.  This is different from the node-initiated reset request.
static void
s_state_engine_do_reset(parch_state_engine_t *self, diagnostic_t diagnostic) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    zframe_t *addr = parch_node_get_node_address(self->node);
    parch_msg_t *node_msg = parch_msg_new_reset_request_msg (addr, local_procedure_error, diagnostic);
    s_state_engine_send_msg_to_node_and_log(self, &node_msg);

    node_t *peer = parch_node_get_peer(self->node);
    if (peer) {
        parch_msg_t *peer_msg = parch_msg_new_reset_request_msg (addr, remote_procedure_error, diagnostic);
        s_state_engine_send_msg_to_peer_and_log(self, &peer_msg);
    }
}

static void
s_state_engine_do_service(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // At the state machine, this does the same thing as state_engine_do_connect.
    // It updates the service.  The difference between the two messages it as
    // the broker level.
    char *sname = parch_msg_service_name(self->request);
    parch_node_set_service_name(self->node, sname);
    parch_node_update_service_name_with_broker(self->node);
}

static void
s_state_engine_do_x_call_accepted(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y requested a call and X agreed
    // Just forward the CALL_ACCEPTED to Y
    parch_msg_t *reply = parch_msg_dup(self->request);

    s_state_engine_send_msg_to_peer_and_log(self, &reply);
    s_state_engine_reset_flow_control(self);
}

static void
s_state_engine_do_x_call_request(parch_state_engine_t * const self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Tell the broker to connect X to an idle node of the requested service.

    // If an idle node for that service is found, a virtual call is
    // made with the idle node becoming Y.  The call request is
    // forwarded on to the Y node.
    if (parch_node_call_request(self->node, parch_msg_service_requested(self->request))) {
        parch_msg_t *peer_msg = parch_msg_dup(self->request);
        self->state = s2_x_call;
        s_state_engine_send_msg_to_peer_and_log(self, &peer_msg);
    } else {
        // If an idle node is not found, a clear request is sent to X.
        s_state_engine_log (self, "no peer available");
        self->state = no_state_change;
        self->next_event = e_y_clear_request;
        self->next_diagnostic = err_no_logical_channel_available;
    }
}

static void
state_engine_do_x_clear_confirmation(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X has agreed to clear the channel.  So, tell the broker to
    // close the connection to Y and end the virtual call.  Then reset
    // flow control.

    parch_msg_t *reply = parch_msg_dup(self->request);
    s_state_engine_send_msg_to_peer_and_log(self, &reply);

    // Disconnect from peer
    s_state_engine_log(self, "clearing");
    parch_node_disconnect_from_peer(self->node);
    s_state_engine_reset_flow_control(self);
}

static void
state_engine_do_x_clear_request(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // The X node has requested that the connection to Y be cleared.
    // The clear request is forwarded on to Y.
    parch_msg_t *reply = parch_msg_dup(self->request);
    s_state_engine_send_msg_to_peer_and_log(self, &reply);
}

static void
state_engine_do_x_data(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X has sent data for Y.  Verify that the DATA packet has the
    // right sequence number and that it is within Y's transmission
    // window.  If it is, forward it to Y.  If it isn't, reject it.
    assert(self->request);
    assert(parch_msg_id(self->request) == PARCH_MSG_DATA);

    // FIXME: lots of flow control logic goes here
    parch_msg_t *reply = parch_msg_dup(self->request);
    s_state_engine_send_msg_to_peer_and_log(self, &reply);
}

static void
s_state_engine_do_x_disconnect(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Our node is quitting or dying.  Clean up as fast as possible.
    // Tell the peer we're going to die

    node_t *peer = parch_node_get_peer(self->node);
    if (peer) {
        parch_msg_t *msg = parch_msg_new(PARCH_MSG_DISCONNECT);
        s_state_engine_send_msg_to_peer_and_log(self, &msg);
    }

    // Disconnect from peer
    s_state_engine_log(self, "disconnect");
    parch_node_disconnect_from_peer(self->node);
    s_state_engine_reset_flow_control(self);
    parch_node_disconnect_from_service(self->node);
    self->stopped = 1;
}
static void
s_state_engine_do_x_rnr(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X has informed that it can't receive any data right now.  Store
    // info about the window and forward the message to Y.
    s_state_engine_do_x_rr_internal(self, 1);
}

static void
s_state_engine_do_x_rr(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X had updated its allowed transmission window.  Store info
    // about the window and then forward the message to Y.
    s_state_engine_do_x_rr_internal(self, 0);
}

static void
s_state_engine_do_x_rr_internal(parch_state_engine_t *self, int not_ready) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X had updated its allowed transmission window.  Store info
    // about the window and then forward the message to Y.
    uint32_t rs = parch_msg_receive_sequence_number(self->request);

    // The node updates the range of packets it will accept.  That
    // range has to be greater than the last range, but, it can't
    // start later than the next packet expected from the peer.
    if (rs > self->x_window && rs <= self->y_sequence_number + 1) {
        self->x_window = rs;
        self->x_not_ready = not_ready;
        parch_msg_t *msg = parch_msg_dup(self->request);
        s_state_engine_send_msg_to_peer_and_log(self, &msg);
    } else {
        // We're out of sync, so we reset the connection
        if (not_ready)
                s_state_engine_log (self, "bad RNR sequence id from node");
        else
                s_state_engine_log (self, "bad RR sequence id from node");
        self->next_event = e_reset;
        self->next_diagnostic = err_invalid_pr;
    }
}

static void
state_engine_do_y_disconnect(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has requested a disconnect.
    // Forward this on to the node so it knows that its peer has disappeared.
    // Forward the call request to the X node.
    parch_msg_t *r = parch_msg_dup(self->request);
    s_state_engine_reset_flow_control(self);
    s_state_engine_send_msg_to_node_and_log(self, &r);
}

static void
s_state_engine_log(const parch_state_engine_t * const self, char *msg) {
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr;
        if (parch_node_get_node_address(self->node) != NULL)
            self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        else
            self_addr = strdup("UNKNOWN");
            zclock_log("I: [%s] state engine -- %s",
                self_addr,
                msg);
        free(self_addr);
    }
}

static void
s_state_engine_log_send_to_node(const parch_state_engine_t * const self, const parch_msg_t * const msg, diagnostic_t d) {
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr;
        if (parch_node_get_node_address(self->node) != NULL)
            self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        else
            self_addr = strdup("UNKNOWN");
        if (d != err_none)
            zclock_log("I: [%s] state engine -- sending '%s' (%s) to node",
                self_addr,
                parch_msg_command(msg),
                diagnostic_messages[d]);
        else
            zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(msg));
        free(self_addr);
    }
}

static void
s_state_engine_log_send_to_peer(const parch_state_engine_t * const self, const parch_msg_t * const msg, diagnostic_t d) {
    if (parch_broker_get_verbose(self->broker)) {
        const node_t * const peer = parch_node_get_peer(self->node);
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        char *peer_addr = zframe_strhex(parch_node_get_node_address(peer));
        if (d != err_none)
            zclock_log("I: [%s] state engine -- sending '%s' (%s) to [%s]",
                self_addr,
                parch_msg_command(msg),
                diagnostic_messages[d],
                peer_addr);
        else
            zclock_log("I: [%s] state engine -- sending '%s' to [%s]",
                self_addr,
                parch_msg_command(msg),
                peer_addr);
        free(self_addr);
        free(peer_addr);
    }
}

static void
s_state_engine_send_msg_to_node_and_log(parch_state_engine_t *self, parch_msg_t **msg_p) {
    parch_msg_t *msg = msg_p[0];

    // Log the sending of the message
    if (parch_broker_get_verbose(self->broker)) {
        diagnostic_t d = err_none;
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        if (parch_msg_id(msg) == PARCH_MSG_CLEAR_REQUEST) {
            d = parch_msg_diagnostic(msg);
        }
        if (d != err_none)
            zclock_log("I: [%s] state engine -- sending '%s' (%s) to node",
                self_addr,
                parch_msg_command(msg),
                diagnostic_messages[d]);
        else
            zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(msg));
        free(self_addr);
    }

    // Send the message
    parch_msg_send(&msg, parch_broker_get_socket(self->broker));
}

static void
s_state_engine_send_msg_to_peer_and_log(parch_state_engine_t *self, parch_msg_t **msg_p) {
    parch_msg_t *msg = msg_p[0];
    const node_t * const peer = parch_node_get_peer(self->node);

    // Log the sending of the message
    if (parch_broker_get_verbose(self->broker)) {
        diagnostic_t d = err_none;
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        char *peer_addr = zframe_strhex(parch_node_get_node_address(peer));
        if (parch_msg_id(msg) == PARCH_MSG_CLEAR_REQUEST) {
            d = parch_msg_diagnostic(msg);
        }
        if (d != err_none)
            zclock_log("I: [%s] state engine -- sending '%s' (%s) to [%s]",
                self_addr,
                parch_msg_command(msg),
                diagnostic_messages[d],
                peer_addr);
        else
            zclock_log("I: [%s] state engine -- sending '%s' to [%s]",
                self_addr,
                parch_msg_command(msg),
                peer_addr);
        free(self_addr);
        free(peer_addr);
    }

    // Send the message
    parch_state_engine_t *peer_engine = parch_node_get_state_engine(peer);
    s_state_engine_y_message(peer_engine, msg_p);
}

static void
s_state_engine_reset_flow_control(parch_state_engine_t * const self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    self->x_sequence_number = 0;
    self->y_sequence_number = 0;
    self->x_window = 0;
    self->y_window = 0;
    self->x_not_ready = 0;
    self->y_not_ready = 0;
}

// ================================================================
// State machine actions












static void
state_engine_do_x_reset_request(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X has requested that data flow control be reset.  Forward the
    // message on to Y.
}

static void
state_engine_do_x_reset_confirmation(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y had requested that flow control be reset and X agreed.  Reset
    // the sequence numbers and flow control.
}

static void
state_engine_do_y_call_request(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y had requested that the broker connect it to an idle node of
    // the requested service, and the broker had determined that this
    // node is and idle node of that service.  The broker had also
    // connected this node in a virtual circuit.

    // Forward the call request to the X node.
    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(r));
        free(self_addr);
    }
    parch_msg_send(&r, parch_broker_get_socket(self->broker));
}

static void
state_engine_do_y_call_accepted(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X requested a call and Y agreed.

    // Forward the call request to the X node.
    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(r));
        free(self_addr);
    }
    parch_msg_send(&r, parch_broker_get_socket(self->broker));

    s_state_engine_reset_flow_control(self);
}

static void
state_engine_do_y_clear_request(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // The Y node has requested that the connection to X be cleared.
    // The clear request is forwarded on to X.
    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(r));
        free(self_addr);
    }
    s_state_engine_reset_flow_control(self);
    parch_msg_send(&r, parch_broker_get_socket(self->broker));
}

static void
state_engine_do_y_clear_confirmation(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has agreed to clear the channel.  So, tell the broker to
    // close the connection to Y and end the virtual call.  Then reset
    // flow control.

    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(r));
        free(self_addr);
    }
    s_state_engine_reset_flow_control(self);
    parch_msg_send(&r, parch_broker_get_socket(self->broker));

    // Disconnect from peer
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- clearing",
                self_addr);
        free(self_addr);
    }
    parch_node_disconnect_from_peer(self->node);
    s_state_engine_reset_flow_control(self);
}

static void
state_engine_do_y_data(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has sent data for X.  Verify that the DATA packet has the
    // right sequence number and that it is within X's transmission
    // window.  If it is, forward it to X.  If it isn't, reject it.

    assert(self->request);
    assert(parch_msg_id(self->request) == PARCH_MSG_DATA);

    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(r));
        free(self_addr);
    }
    parch_msg_send(&r, parch_broker_get_socket(self->broker));
}

static void
state_engine_do_y_rr(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y had updated its allowed transmission window.  Store info
    // about the window and then forward the message to X.
}

static void
state_engine_do_y_rnr(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has informed that it can't receive any data right now.  Store
    // info about the window and forward the message to X.
}

static void
state_engine_do_y_reset_request(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has requested that data flow control be reset.  Forward the
    // message on to X.
    // Forward the call request to the X node.
    parch_msg_t *r = parch_msg_dup(self->request);

    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- sending '%s' to node",
                self_addr,
                parch_msg_command(r));
        free(self_addr);
    }

    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    parch_msg_send(&r, parch_broker_get_socket(self->broker));
}

static void
state_engine_do_y_reset_confirmation(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X had requested that flow control be reset and Y agreed.  Reset
    // the sequence numbers and flow control.
}

static void
state_engine_do_s0_initialize_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // We've been in S0 for a while without registering.  Disconnect
}

static void
state_engine_do_s2_x_call_waiting_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s3_y_call_waiting_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s6_x_clear_request_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s7_y_clear_request_first_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s7_y_clear_request_second_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s8_x_reset_request_first_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s8_x_reset_request_second_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s9_y_reset_request_first_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s9_y_reset_request_second_timeout(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_config_self(parch_state_engine_t *self) {
    // state engine is still partly uninitialized here.
    // Can't do STATE_ENGINE_VALIDITY_CHECKS(self);
}

static int
state_machine_dispatch(parch_state_engine_t *self, action_t action, diagnostic_t diagnostic, state_t next) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    self->event = self->next_event;
    self->next_event = no_event;
    self->next_diagnostic = err_none;

    state_t state_orig = self->state;
    char *node_addr = zframe_strhex(parch_node_get_node_address(self->node));
    if (parch_broker_get_verbose(self->broker))
        zclock_log("I: [%s] state engine -- doing '%s' action in '%s'",
            node_addr,
            action_names[action], state_names[self->state]);
    free(node_addr);
    usleep(1);
    switch (action) {
        case a_error:
            s_state_engine_do_error(self, diagnostic);
            break;
        case a_discard:
            break;
        case a_connect:
            s_state_engine_do_connect(self);
            break;
        case a_service:
            s_state_engine_do_service(self);
            break;
        case a_reset:
            s_state_engine_do_reset(self, diagnostic);
            break;

        case a_x_disconnect:
            s_state_engine_do_x_disconnect(self);
            break;
        case a_x_call_request:
            s_state_engine_do_x_call_request(self);
            break;
        case a_x_call_accepted:
            s_state_engine_do_x_call_accepted(self);
            break;
        case a_x_clear_request:
            state_engine_do_x_clear_request(self);
            break;
        case a_x_clear_confirmation:
            state_engine_do_x_clear_confirmation(self);
            break;
        case a_x_data:
            state_engine_do_x_data(self);
            break;
        case a_x_rr:
            s_state_engine_do_x_rr(self);
            break;
        case a_x_rnr:
            s_state_engine_do_x_rnr(self);
            break;
        case a_x_reset:
            state_engine_do_x_reset_request(self);
            break;
        case a_x_reset_confirmation:
            state_engine_do_x_reset_confirmation(self);
            break;

        case a_y_disconnect:
            state_engine_do_y_disconnect(self);
            break;
        case a_y_call_request:
            state_engine_do_y_call_request(self);
            break;
        case a_y_call_accepted:
            state_engine_do_y_call_accepted(self);
            break;
        case a_y_clear_request:
            state_engine_do_y_clear_request(self);
            break;
        case a_y_clear_confirmation:
            state_engine_do_y_clear_confirmation(self);
            break;
        case a_y_data:
            state_engine_do_y_data(self);
            break;
        case a_y_rr:
            state_engine_do_y_rr(self);
            break;
        case a_y_rnr:
            state_engine_do_y_rnr(self);
            break;
        case a_y_reset:
            state_engine_do_y_reset_request(self);
            break;
        case a_y_reset_confirmation:
            state_engine_do_y_reset_confirmation(self);
            break;

            //            case s0_initialize_timeout:
            //                state_engine_do_s0_initialize_timeout(self);
            //                break;
            //            case s2_x_call_waiting_timeout:
            //                state_engine_do_s2_x_call_waiting_timeout(self);
            //                break;
            //            case s3_y_call_waiting_timeout:
            //                state_engine_do_s3_y_call_waiting_timeout(self);
            //                break;
            //            case s6_x_clear_request_timeout:
            //                state_engine_do_s6_x_clear_request_timeout(self);
            //                break;
            //            case s7_y_clear_request_first_timeout:
            //                state_engine_do_s7_y_clear_request_first_timeout(self);
            //                break;
            //            case s7_y_clear_request_second_timeout:
            //                state_engine_do_s7_y_clear_request_second_timeout(self);
            //                break;
            //            case s8_x_reset_request_first_timeout:
            //                state_engine_do_s8_x_reset_request_first_timeout(self);
            //                break;
            //            case s8_x_reset_request_second_timeout:
            //                state_engine_do_s8_x_reset_request_second_timeout(self);
            //                break;
            //            case s9_y_reset_request_first_timeout:
            //                state_engine_do_s9_y_reset_request_first_timeout(self);
            //                break;
            //            case s9_y_reset_request_second_timeout:
            //                state_engine_do_s9_y_reset_request_second_timeout(self);
            //                break;
    }
    if (self->stopped)
        return 1;
    if (next != no_state)
        self->state = next;
    if (state_orig != self->state) {
        if (parch_broker_get_verbose(self->broker)) {
            char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
            zclock_log("I: [%s] state engine -- changing state '%s' -> '%s'",
                    self_addr,
                    state_names[state_orig], state_names[self->state]);
            free(self_addr);
        }

        // Certain timers are initialized when state is changed
        // First, clear all the timers
        // Then...
        if (self->state == s3_y_call) {
            // Start 180s s3 timer.
            // (On timeout, it will do the standard error)
        } else if (self->state == s7_y_clear) {
            // Start 60s, two-part, clear indication timer.
            // (First timeout will resend the CLEAR_REQUEST to the node)
            // (Second timeout will send a diagnostic to the node and transfer
            // to s1_ready)
        }
    }
    return 0;
}

static int
state_machine_dispatch_event(parch_state_engine_t *self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    int done = 0;
    assert(self->next_event >= e_x_connect && self->next_event <= max_event);
    while (self->next_event != no_event) {
        state_t s = self->state;
        event_t e = self->next_event;
        diagnostic_t d = self->next_diagnostic;
        assert(s >= s0_init && s <= s9_y_reset);
        assert(e >= 0 && e <= max_event);
        const state_machine_table_element_t * const elt = &state_machine_table[s][e];
        if (d != err_none) {
            // Some events, like reset and clear events, override the state machine's generic
            // diagnostic message with something more specific.
            done = state_machine_dispatch(self, elt->action, d, elt->next);
            self->next_diagnostic = err_none;
        }
        else
            // But usually, we just have a diagnostic message based on the state of the table.
            done = state_machine_dispatch(self, elt->action, elt->diagnostic, elt->next);

    }
    return done;
}

parch_state_engine_t *
parch_state_engine_new(broker_t *broker, node_t *node) {
    parch_state_engine_t *self = (parch_state_engine_t *) zmalloc(sizeof (parch_state_engine_t));
    self->broker = broker;
    self->node = node;
    self->state = s0_init;
    self->event = no_event;
    self->next_event = no_event;
    self->request = NULL;
    self->config = zconfig_new("state", NULL);
    state_engine_config_self(self);

    if (parch_broker_get_verbose(self->broker)) {
        char * const self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] state engine -- allocated, service = '%s'",
                self_addr,
                parch_node_get_service_name(self->node));
        free(self_addr);
    }

    return self;
}

void
parch_state_engine_destroy(parch_state_engine_t **self_p) {

    assert(self_p);
    parch_state_engine_t *self = (parch_state_engine_t *) self_p[0];
    STATE_ENGINE_VALIDITY_CHECKS(self);
    assert(self);
    zconfig_destroy(&self->config);
    if (self->request)
        parch_msg_destroy(&self->request);
    free(self);
}

void
parch_state_engine_test(bool verbose);

// Some external methods

//  Process message from pipe

int
parch_state_engine_x_message(parch_state_engine_t *self, parch_msg_t *msg) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    assert(msg);

    if (self->request)
        parch_msg_destroy(&self->request);
    assert(self->request == NULL);
    self->request = parch_msg_dup(msg);
    switch (parch_msg_id(msg)) {
        case PARCH_MSG_DATA:
            assert(parch_msg_send_sequence_number(msg) == self->x_sequence_number);
            self->next_event = e_x_data;
            break;

        case PARCH_MSG_RR:
            assert(parch_msg_receive_sequence_number(msg) >= self->x_window);
            self->next_event = e_x_rr;
            break;

        case PARCH_MSG_RNR:
            self->next_event = e_x_rnr;
            break;

        case PARCH_MSG_CALL_REQUEST:
            assert(parch_msg_service_requested(msg) != NULL);
            assert(strlen(parch_msg_service_requested(msg)) > 0);
            self->next_event = e_x_call_request;
            break;

        case PARCH_MSG_CALL_ACCEPTED:
            self->next_event = e_x_call_accepted;
            break;

        case PARCH_MSG_CLEAR_REQUEST:
            self->next_event = e_x_clear_request;
            break;

        case PARCH_MSG_CLEAR_CONFIRMATION:
            self->next_event = e_x_clear_confirmation;
            break;

        case PARCH_MSG_RESET_REQUEST:
            self->next_event = e_x_reset_request;
            break;

        case PARCH_MSG_RESET_CONFIRMATION:
            self->next_event = e_x_reset_confirmation;
            break;

        case PARCH_MSG_CONNECT:
            self->next_event = e_x_connect;
            break;

        case PARCH_MSG_SERVICE:
            self->next_event = e_x_service;
            break;

        case PARCH_MSG_DISCONNECT:
            self->next_event = e_x_disconnect;
            break;
    }
    int done = state_machine_dispatch_event(self);

    STATE_ENGINE_VALIDITY_CHECKS(self);
    return done;
}

static int
s_state_engine_y_message(parch_state_engine_t *self, parch_msg_t **msg_p) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    parch_msg_t *msg = msg_p[0];
    self->request = parch_msg_dup(msg);

    if (parch_broker_get_verbose(self->broker)) {
        char * const self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] socket -- received %s msg from peer", self_addr, parch_msg_command(msg));
        free(self_addr);
        parch_msg_dump(msg);
    }

    switch (parch_msg_id(msg)) {
        case PARCH_MSG_DATA:
            assert(parch_msg_send_sequence_number(msg) == self->y_sequence_number);
            self->next_event = e_y_data;
            break;

        case PARCH_MSG_RR:
            assert(parch_msg_receive_sequence_number(msg) >= self->y_window);
            self->next_event = e_y_rr;
            break;

        case PARCH_MSG_RNR:
            self->next_event = e_y_rnr;
            break;

        case PARCH_MSG_CALL_REQUEST:
            assert(parch_msg_service_requested(msg) != NULL);
            assert(strlen(parch_msg_service_requested(msg)) > 0);
            self->next_event = e_y_call_request;
            break;

        case PARCH_MSG_CALL_ACCEPTED:
            self->next_event = e_y_call_accepted;
            break;

        case PARCH_MSG_CLEAR_REQUEST:
            self->next_event = e_y_clear_request;
            break;

        case PARCH_MSG_CLEAR_CONFIRMATION:
            self->next_event = e_y_clear_confirmation;
            break;

        case PARCH_MSG_RESET_REQUEST:
            self->next_event = e_y_reset_request;
            break;

        case PARCH_MSG_RESET_CONFIRMATION:
            self->next_event = e_y_reset_confirmation;
            break;
        case PARCH_MSG_DISCONNECT:
            self->next_event = e_y_disconnect;
            break;
    }
    int done = state_machine_dispatch_event(self);
    parch_msg_destroy(msg_p);
    return done;
}
