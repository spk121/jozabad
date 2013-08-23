/*  =========================================================================
    <name> - <description>

    -------------------------------------------------------------------------
    Copyright (c) <year> - <company name> - <website>
    Copyright other contributors as noted in the AUTHORS file.

    This file is part of <project name>, <descriptio>
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

typedef enum _action_t action_t;

enum _event_t {
    e_unspecified = 0,
    e_min = e_unspecified,

    // Broker or state-engine initiated events
    e_reset,
    e_clear,
    e_disconnect,

    // Node-initiated events
    e_x_connect,
    e_x_disconnect,
    e_x_call_request,
    e_x_call_accepted,
    e_x_clear_request,
    e_x_clear_confirmation,
    e_x_data,
    e_x_rr,
    e_x_rnr,
    e_x_reset_request,
    e_x_reset_confirmation,

    // Peer-initiated events
    e_y_disconnect,
    e_y_call_request,
    e_y_call_accepted,
    e_y_clear_request,
    e_y_clear_confirmation,
    e_y_data,
    e_y_rr,
    e_y_rnr,
    e_y_reset_request,
    e_y_reset_confirmation,

    e_max = e_y_reset_confirmation
};

typedef enum _event_t event_t;

enum _state_t {
    s_unspecified = 0,
    s_min = s_unspecified,
    s0_disconnected,
    s1_ready,
    s2_x_call,
    s3_y_call,
    s4_data,
    s5_collision,
    s6_x_clear,
    s7_y_clear,
    s8_x_reset,
    s9_y_reset,
    s_max = s9_y_reset,
};

typedef enum _state_t state_t;

struct _parch_state_engine_t {
    broker_t *broker; // loopback to the broker that contains this node
    node_t *node; // loopback to the node that contains this state engine
    state_t state; // current state in the state machine
    event_t event; // current event being processed by the state machine
    uint32_t event_id;  // count of events processed by this state engine
    uint32_t timeout_count;

    //
    uint8_t incoming_calls_barred; // when true, all incoming call request from peer are ignored
    uint8_t outgoing_calls_barred; // when true, all call requests from node are ignored.
    uint8_t throughput_index; // The rate at which this connection is throttled

    // Values for the current call negotiation
    uint8_t call_incoming_data_barred;
    uint8_t call_outgoing_data_barred;
    uint8_t call_throughput_index; // less than or equal to the ->throughput_index
    uint8_t call_packet_index;
    uint16_t call_window_size;

    // Coming up...
    event_t next_event; // next event to be processed by the state machine
    clearing_cause_t next_cause; // error category for next event, if required
    diagnostic_t next_diagnostic; // diagnostic for next event, if required

    // Flow control
    uint32_t x_sequence_number; // serial number of last DATA from X
    uint32_t y_sequence_number; // serial number of last DATA from Y
    uint32_t x_window; // serial number of last RR from X for Y
    uint32_t y_window; // serial number of last RR from Y for X
    int x_not_ready; // true if we received a RNR from X
    int y_not_ready; // true if we received a RNR from Y
    uint64_t _time;
    float channel_capacity_in_use;

    parch_msg_t *request; // current message being processed
    zconfig_t *config;
    int stopped;
};

struct _state_machine_table_element_t {
    action_t action;
    diagnostic_t diagnostic;
    state_t next;
};

struct _timeout_t {
    parch_state_engine_t *state_engine;
    uint32_t event_id;
    uint32_t iteration;
};

typedef struct _timeout_t timeout_t;

//  ---------------------------------------------------------------------------
//  MACROS

#define TIMEOUT_T11 (18)   // node must reply to our call request within TIMEOUT_T11
#define TIMEOUT_T13 (6)
#define TIMEOUT_T21 (20)   // peer must reply to our call request within TIMEOUT_T21


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
        assert (s->state <= s_max); \
        assert (s->event <= e_max); \
        assert (s->next_event <= e_max); \
        assert (s->x_window <= s->y_sequence_number); \
        assert (s->y_window <= s->x_sequence_number); \
        assert (s->x_not_ready == 0 || s->x_not_ready == 1); \
        assert (s->y_not_ready == 0 || s->y_not_ready == 1); \
        assert (s->request == 0 || parch_msg_id_is_valid(s->request)); \
        /* assert (s->request == 0 || (parch_msg_address(s->request) != 0)); */ \
        /* assert (s->reply == 0 || is_valid_id(parch_msg_id(s->reply))); */ \
        /* assert (s->reply == 0 || (parch_msg_address(s->reply) != 0)); */ \
} while (0);

//  ---------------------------------------------------------------------------
//  TYPEDEFS
typedef struct _state_machine_table_element_t state_machine_table_element_t;

//  ---------------------------------------------------------------------
//  State machine constants

static const char action_names[][21] = {
    [a_discard] = "discard",
    [a_reset] = "reset",
    [a_clear] = "clear",
    [a_disconnect] = "disconnect",

    [a_x_connect] = "x_connect",
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
    [err_packet_type_invalid_for_state_s9] = "invalid packet type for s9_y_reset state",
    [err_outgoing_data_barred] = "outgoing data barred",
    [err_data_barred] = "all data barred",
    [err_throughput_out_of_range] = "throughput is out of range",
    [err_packet_size_out_of_range] = "packet size is out of range",
    [err_window_size_out_of_range] = "window size is out of range",
    [err_invalid_negotiation__incoming_data_barred] = "invalid incoming data barred negotiation",
    [err_invalid_negotiation__outgoing_data_barred] = "invalid outgoing data barred negotiation",
    [err_invalid_negotiation__throughput] = "invalid throughput negotiation",
    [err_invalid_negotiation__window_size] = "invalid window size negotiation",
    [err_invalid_negotiation__packet_size] = "invalid packet size negotiation",
    [err_packet_too_short] = "packet too short",
    [err_packet_too_long] = "packet_too_long",
    [err_invalid_ps] = "invalid send sequence number",
    [err_network_congestion] = "channel capacity exceeded",
    [err_time_expired_for_x_call_request] = "timeout for x call request"
};

static const state_machine_table_element_t state_machine_table[s_max + 1][e_max + 1] = {
    /* S0 Initialize          */
    [s0_disconnected] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_discard, err_none, s0_disconnected},
        [e_clear] =
        {a_discard, err_none, s0_disconnected},
        [e_disconnect] =
        {a_discard, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_x_connect, err_none, s1_ready},
        [e_x_disconnect] =
        {a_discard, err_none, s0_disconnected},
        [e_x_call_request] =
        {a_discard, err_none, s0_disconnected},
        [e_x_call_accepted] =
        {a_discard, err_none, s0_disconnected},
        [e_x_clear_request] =
        {a_discard, err_none, s0_disconnected},
        [e_x_clear_confirmation] =
        {a_discard, err_none, s0_disconnected},
        [e_x_data] =
        {a_discard, err_none, s0_disconnected},
        [e_x_rr] =
        {a_discard, err_none, s0_disconnected},
        [e_x_rnr] =
        {a_discard, err_none, s0_disconnected},
        [e_x_reset_request] =
        {a_discard, err_none, s0_disconnected},
        [e_x_reset_confirmation] =
        {a_discard, err_none, s0_disconnected},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_discard, err_none, s0_disconnected},
        [e_y_call_request] =
        {a_discard, err_none, s0_disconnected},
        [e_y_call_accepted] =
        {a_discard, err_none, s0_disconnected},
        [e_y_clear_request] =
        {a_discard, err_none, s0_disconnected},
        [e_y_clear_confirmation] =
        {a_discard, err_none, s0_disconnected},
        [e_y_data] =
        {a_discard, err_none, s0_disconnected},
        [e_y_rr] =
        {a_discard, err_none, s0_disconnected},
        [e_y_rnr] =
        {a_discard, err_none, s0_disconnected},
        [e_y_reset_request] =
        {a_discard, err_none, s0_disconnected},
        [e_y_reset_confirmation] =
        {a_discard, err_none, s0_disconnected}
    },

    /* S1 READY                */
    [s1_ready] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_discard, err_none, s1_ready},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_discard, err_none, s1_ready},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s0_disconnected},
        [e_x_call_request] =
        {a_x_call_request, err_none, s2_x_call},
        [e_x_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s6_x_clear},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_x_data] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_x_rr] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_x_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_x_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_x_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_y_call_request, err_none, s3_y_call},
        [e_y_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_y_clear_request] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_y_data] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_y_rr] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_y_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_y_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_y_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear}
    },
    [s2_x_call] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_discard, err_none, s1_ready},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s6_x_clear},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_data] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_rr] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_x_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_y_call_request, err_none, s5_collision},
        [e_y_call_accepted] =
        {a_y_call_accepted, err_none, s4_data},
        [e_y_clear_request] =
        {a_y_clear_request, err_none, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_y_data] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_y_rr] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_y_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_y_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear},
        [e_y_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s2, s7_y_clear}
    },
    [s3_y_call] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_discard, err_none, s1_ready},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_discard, err_none, s5_collision},
        [e_x_call_accepted] =
        {a_x_call_accepted, err_none, s4_data},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s6_x_clear},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_x_data] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_x_rr] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_x_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_x_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_x_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_y_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_y_clear_request] =
        {a_y_clear_request, err_none, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_y_data] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_y_rr] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_y_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_y_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear},
        [e_y_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s3, s7_y_clear}
    },
    [s4_data] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_reset, err_none, s9_y_reset},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
        [e_x_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s6_x_clear},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
        [e_x_data] =
        {a_x_data, err_none, s4_data},
        [e_x_rr] =
        {a_x_rr, err_none, s4_data},
        [e_x_rnr] =
        {a_x_rnr, err_none, s4_data},
        [e_x_reset_request] =
        {a_x_reset, err_none, s8_x_reset},
        [e_x_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
        [e_y_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
        [e_y_clear_request] =
        {a_y_clear_request, err_none, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
        [e_y_data] =
        {a_y_data, err_none, s4_data},
        [e_y_rr] =
        {a_y_rr, err_none, s4_data},
        [e_y_rnr] =
        {a_y_rnr, err_none, s4_data},
        [e_y_reset_request] =
        {a_y_reset, err_none, s9_y_reset},
        [e_y_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s4, s7_y_clear},
    },
    [s5_collision] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_reset, err_none, s9_y_reset},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s6_x_clear},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_data] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_rr] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_x_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_y_call_accepted] =
        {a_y_call_accepted, err_none, s4_data},
        [e_y_clear_request] =
        {a_y_clear_request, err_none, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_y_data] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_y_rr] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_y_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_y_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},
        [e_y_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s5, s7_y_clear},

    },
    [s6_x_clear] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_reset, err_none, s9_y_reset},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_clear_request] =
        {a_discard, err_none, s6_x_clear},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_data] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_rr] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_x_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_discard, err_none, s6_x_clear},
        [e_y_call_accepted] =
        {a_discard, err_none, s6_x_clear},
        [e_y_clear_request] =
        {a_y_clear_request, err_none, s1_ready},
        [e_y_clear_confirmation] =
        {a_y_clear_confirmation, err_none, s1_ready},
        [e_y_data] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_y_rr] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_y_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_y_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
        [e_y_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s6, s7_y_clear},
    },
    /* S7 Y CLEAR                */
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_discard, err_none, s7_y_clear},
        [e_clear] =
        {a_discard, err_none, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_discard, err_none, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_discard, err_none, s7_y_clear},
        [e_x_call_accepted] =
        {a_discard, err_none, s7_y_clear},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s1_ready},
        [e_x_clear_confirmation] =
        {a_x_clear_confirmation, err_none, s1_ready},
        [e_x_data] =
        {a_discard, err_none, s7_y_clear},
        [e_x_rr] =
        {a_discard, err_none, s7_y_clear},
        [e_x_rnr] =
        {a_discard, err_none, s7_y_clear},
        [e_x_reset_request] =
        {a_discard, err_none, s7_y_clear},
        [e_x_reset_confirmation] =
        {a_discard, err_none, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_discard, err_none, s7_y_clear},
        [e_y_call_accepted] =
        {a_discard, err_none, s7_y_clear},
        [e_y_clear_request] =
        {a_discard, err_none, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_discard, err_none, s7_y_clear},
        [e_y_data] =
        {a_discard, err_none, s7_y_clear},
        [e_y_rr] =
        {a_discard, err_none, s7_y_clear},
        [e_y_rnr] =
        {a_discard, err_none, s7_y_clear},
        [e_y_reset_request] =
        {a_discard, err_none, s7_y_clear},
        [e_y_reset_confirmation] =
        {a_discard, err_none, s7_y_clear},
    },
    [s8_x_reset] =
    {
        // Broker or state-engine initiated events
        [e_reset] =
        {a_reset, err_none, s9_y_reset},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_x_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s1_ready},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_x_data] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_x_rr] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_x_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_x_reset_request] =
        {a_discard, err_none, s8_x_reset},
        [e_x_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},

        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_y_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_y_clear_request] =
        {a_y_clear_request, err_none, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s8, s7_y_clear},
        [e_y_data] =
        {a_discard, err_none, s8_x_reset},
        [e_y_rr] =
        {a_discard, err_none, s8_x_reset},
        [e_y_rnr] =
        {a_discard, err_none, s8_x_reset},
        [e_y_reset_request] =
        {a_y_reset, err_none, s4_data},
        [e_y_reset_confirmation] =
        {a_y_reset_confirmation, err_none, s4_data},
    },
    [s9_y_reset] =
    {
        [e_reset] =
        {a_discard, err_none, s9_y_reset},
        [e_clear] =
        {a_clear, err_packet_type_invalid_for_state_s1, s7_y_clear},
        [e_disconnect] =
        {a_disconnect, err_none, s0_disconnected},

        // Node-initiated events
        [e_x_connect] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_x_disconnect] =
        {a_x_disconnect, err_none, s_unspecified},
        [e_x_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_x_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_x_clear_request] =
        {a_x_clear_request, err_none, s6_x_clear},
        [e_x_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_x_data] =
        {a_discard, err_none, s9_y_reset},
        [e_x_rr] =
        {a_discard, err_none, s9_y_reset},
        [e_x_rnr] =
        {a_discard, err_none, s9_y_reset},
        [e_x_reset_request] =
        {a_x_reset, err_none, s4_data},
        [e_x_reset_confirmation] =
        {a_x_reset_confirmation, err_none, s4_data},



        // Peer-initiated events
        [e_y_disconnect] =
        {a_y_disconnect, err_none, s1_ready},
        [e_y_call_request] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_y_call_accepted] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_y_clear_request] =
        {a_y_clear_request, err_none, s7_y_clear},
        [e_y_clear_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_y_data] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_y_rr] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_y_rnr] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_y_reset_request] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
        [e_y_reset_confirmation] =
        {a_clear, err_packet_type_invalid_for_state_s9, s7_y_clear},
    }
};

static const char state_names[][16] = {
    [s0_disconnected] = "s0_disconnected",
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

static event_t
s_msg_id_x_next_event(int id);

static event_t
s_msg_id_y_next_event(int id);

static void
s_state_engine_clear_negotiation_parameters(parch_state_engine_t *self);

static void
s_state_engine_do_disconnect_internal(parch_state_engine_t *self);

static void
s_state_engine_do_clear(parch_state_engine_t *self, diagnostic_t diagnostic)
__attribute__((nonnull(1)));

static void
s_state_engine_do_disconnect(parch_state_engine_t *self, diagnostic_t diagnostic);

static void
s_state_engine_do_reset(parch_state_engine_t *self, diagnostic_t diagnostic);

static void
s_state_engine_do_x_call_accepted(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_x_call_request(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_x_clear_request(parch_state_engine_t * self);

static void
s_state_engine_do_x_clear_confirmation(parch_state_engine_t * self);

static void
s_state_engine_do_x_disconnect(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_x_data(parch_state_engine_t * self);

static void
s_state_engine_do_x_reset_confirmation(parch_state_engine_t * self);

static void
s_state_engine_do_x_reset_request(parch_state_engine_t * self);

static void
s_state_engine_do_x_rnr(parch_state_engine_t * self);

static void
s_state_engine_do_x_rr(parch_state_engine_t * self);

static void
s_state_engine_do_x_rr_internal(parch_state_engine_t *self, int not_ready);

static void
s_state_engine_do_y_call_accepted(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_y_call_request(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_y_clear_confirmation(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_y_clear_request(parch_state_engine_t * self);

static void
s_state_engine_do_y_data(parch_state_engine_t * self);

static void
s_state_engine_do_y_disconnect(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_y_reset_confirmation(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_y_reset_request(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_do_y_rnr(parch_state_engine_t * self);

static void
s_state_engine_do_y_rr(parch_state_engine_t * self);

static void
s_state_engine_do_y_rr_internal(parch_state_engine_t *self, int not_ready);

static bool
s_state_engine_has_peer(parch_state_engine_t *self);

static void
s_state_engine_log(parch_state_engine_t * self, const char *text)
__attribute__((nonnull(1, 2)));

static void
s_state_engine_send_msg_to_node_and_log(parch_state_engine_t *self, parch_msg_t **msg_p);

static void
s_state_engine_send_msg_to_peer_and_log(parch_state_engine_t *self, parch_msg_t **msg_p);

static void
s_state_engine_reset_flow_control(parch_state_engine_t * self)
__attribute__((nonnull(1)));

static void
s_state_engine_store_negotiation_parameters(parch_state_engine_t *self, parch_msg_t *r);

static int
s_state_engine_timeout_t11_callback (zloop_t *loop, zmq_pollitem_t *item __attribute__((unused)), void *arg);
static int
s_state_engine_timeout_t13_callback (zloop_t *loop, zmq_pollitem_t *item __attribute__((unused)), void *arg);
static int
s_state_engine_timeout_t21_callback (zloop_t *loop, zmq_pollitem_t *item __attribute__((unused)), void *arg);

static bool
s_state_engine_validate_negotiation_parameters(parch_state_engine_t *self, parch_msg_t *r, diagnostic_t *diagnostic);

static int
s_state_engine_y_message(parch_state_engine_t *self, parch_msg_t **msg_p);

static int
state_machine_dispatch(parch_state_engine_t *self, action_t action, diagnostic_t diagnostic, state_t next);

// ================================================================
// Lowest-level operations for state machine

static void
s_state_engine_clear_negotiation_parameters(parch_state_engine_t *self) {
    self->call_incoming_data_barred = 0;
    self->call_outgoing_data_barred = 0;
    self->call_throughput_index = 0;
    self->call_packet_index = 0;
    self->call_window_size = 0;
}

// This function is when the broker itself tells the nodes to clear the connection.  This
// happens when the broker decides that an error has occurred.

static void
s_state_engine_do_clear(parch_state_engine_t *self, diagnostic_t diagnostic) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    // Clear tell the node to close its connection.
    zframe_t *addr = parch_node_get_node_address(self->node);
    parch_msg_t *node_msg = parch_msg_new_clear_request_msg(addr, local_procedure_error, diagnostic);
    s_state_engine_send_msg_to_node_and_log(self, &node_msg);

    // Also, tell the peer to close its connection.
    if (s_state_engine_has_peer(self)) {
        parch_msg_t *peer_msg = parch_msg_new_clear_request_msg(addr, remote_procedure_error, diagnostic);
        s_state_engine_send_msg_to_peer_and_log(self, &peer_msg);
    }
            // The peer needs to reply to this message, else we'll timeout.
            timeout_t *t = (timeout_t *)zmalloc(sizeof(timeout_t));
            t->state_engine = self;
            t->event_id = self->event_id;
            t->iteration = self->timeout_count;
            parch_broker_add_timer (self->broker, TIMEOUT_T13, s_state_engine_timeout_t13_callback, t);
}

// This action is when the broker itself shuts this node down.

static void
s_state_engine_do_disconnect(parch_state_engine_t *self, diagnostic_t diagnostic) {
    zframe_t *addr = parch_node_get_node_address(self->node);
    parch_msg_t *msg = parch_msg_new_disconnect_indication_msg(addr, local_procedure_error, diagnostic);
    s_state_engine_send_msg_to_node_and_log(self, &msg);

    // Also, tell the peer that this node is disconnecting
    if (s_state_engine_has_peer(self)) {
        zframe_t *addr = parch_node_get_node_address(self->node);
        parch_msg_t *msg = parch_msg_new_disconnect_indication_msg(addr, remote_procedure_error, diagnostic);
        s_state_engine_send_msg_to_peer_and_log(self, &msg);
    }

    // Disconnect
    s_state_engine_do_disconnect_internal(self);
}

static void
s_state_engine_do_disconnect_internal(parch_state_engine_t *self) {
    s_state_engine_log(self, "disconnect");
    parch_node_disconnect_from_peer(self->node);
    s_state_engine_reset_flow_control(self);
    parch_node_disconnect_from_service(self->node);
    self->stopped = 1;
}

// This is the state-engine initiated reset request, where it is flagging a flow control problem
// and requires a reset.  This is different from the node-initiated reset request.

static void
s_state_engine_do_reset(parch_state_engine_t *self, diagnostic_t diagnostic) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    zframe_t *addr = parch_node_get_node_address(self->node);
    parch_msg_t *node_msg = parch_msg_new_reset_request_msg(addr, local_procedure_error, diagnostic);
    s_state_engine_send_msg_to_node_and_log(self, &node_msg);

    if (s_state_engine_has_peer(self)) {
        zframe_t *addr = parch_node_get_node_address(self->node);
        parch_msg_t *peer_msg = parch_msg_new_reset_request_msg(addr, remote_procedure_error, diagnostic);
        s_state_engine_send_msg_to_peer_and_log(self, &peer_msg);
    }
}

static void
s_state_engine_do_x_call_accepted(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y requested a call and X agreed
    // Just forward the CALL_ACCEPTED to Y
    parch_msg_t *reply = parch_msg_dup(self->request);

    // CALL NEGOTIATION, STEP 3

    // Here in STEP 3, we make sure that the node hasn't cheated
    // on negotiation by increasing any of the parameters invalidly.
    parch_msg_apply_defaults_to_connect_request(reply);
    diagnostic_t diagnostic;
    if (parch_msg_validate_connect_request(reply, &diagnostic) == false
            || s_state_engine_validate_negotiation_parameters(self, reply, &diagnostic) == false) {
        s_state_engine_log(self, diagnostic_messages[diagnostic]);
        self->state = s_unspecified;
        self->next_event = e_clear;
        self->next_cause = invalid_facility_request;
        self->next_diagnostic = diagnostic;
        parch_msg_destroy(&reply);
    } else {
        s_state_engine_send_msg_to_peer_and_log(self, &reply);
        s_state_engine_reset_flow_control(self);
    }
}

static void
s_state_engine_do_x_call_request(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Tell the broker to connect X to an idle node of the requested service.

    // If an idle node for that service is found, a virtual call is
    // made with the idle node becoming Y.  The call request is
    // forwarded on to the Y node.
    if (!parch_node_call_request(self->node, parch_msg_service(self->request))) {
        // If an idle node is not found, a clear request is sent to X.
        s_state_engine_log(self, "no peer available");
        self->state = s_unspecified;
        self->next_event = e_clear;
        self->next_cause = number_busy;
        self->next_diagnostic = err_no_logical_channel_available;
    } else {
        // CALL NEGOTIATION, STEP 1
        // When a call request passes through the state engine, it begins
        // the process of call negotiation.  The state engine may
        // downgrade some of the requested attributes of the connection
        // before sending it to the peer.
        diagnostic_t diagnostic;
        if (parch_msg_validate_connect_request(self->request, &diagnostic) == false) {
            self->state = s_unspecified;
            self->next_event = e_clear;
            self->next_cause = invalid_facility_request;
            self->next_diagnostic = diagnostic;
            s_state_engine_log(self, diagnostic_messages[diagnostic]);
        } else {
            parch_msg_t *peer_msg = parch_msg_dup(self->request);
            parch_msg_apply_defaults_to_connect_request(peer_msg);
            // The initiating node may not declare a throughput that is faster
            // than it can handle.
            uint8_t i = parch_throughput_index_throttle(parch_msg_throughput(peer_msg),
                    self->throughput_index);
            parch_msg_set_throughput(peer_msg, i); // We also store info about the call request.  We'll need it later steps.
            s_state_engine_store_negotiation_parameters(self, peer_msg);

            self->state = s2_x_call;
            s_state_engine_send_msg_to_peer_and_log(self, &peer_msg);

            // The peer needs to reply to this message, else we'll timeout.
            timeout_t *t = (timeout_t *)zmalloc(sizeof(timeout_t));
            t->state_engine = self;
            t->event_id = self->event_id;
            t->iteration = 0;
            parch_broker_add_timer (self->broker, TIMEOUT_T21, s_state_engine_timeout_t21_callback, t);
        }
    }
}

static void
s_state_engine_do_x_clear_confirmation(parch_state_engine_t * self) {
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
s_state_engine_do_x_clear_request(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // The X node has requested that the connection to Y be cleared.
    // The clear request is forwarded on to Y.
    parch_msg_t *reply = parch_msg_dup(self->request);
    s_state_engine_send_msg_to_peer_and_log(self, &reply);
}

static void
s_state_engine_do_x_data(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    assert(self->request);
    assert(parch_msg_id(self->request) == PARCH_MSG_DATA);

    bool ok = true;
    if (self->call_outgoing_data_barred) {
        ok = false;
        self->next_cause = access_barred;
        self->next_diagnostic = err_outgoing_data_barred;
    } else {
        size_t siz = zframe_size(parch_msg_data(self->request));
        size_t siz_max = parch_packet_bytes(self->call_packet_index);
        if (siz == 0) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_packet_too_short;
        } else if (siz > siz_max) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_packet_too_long;
        } else if (parch_msg_sequence(self->request) != self->x_sequence_number) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_invalid_ps;
        }
    }

    if (ok == false) {
        self->state = s_unspecified;
        s_state_engine_log(self, diagnostic_messages[self->next_diagnostic]);
    } else {
        parch_msg_t *msg = parch_msg_dup(self->request);
        s_state_engine_send_msg_to_peer_and_log(self, &msg);
        self->x_sequence_number++;
    }
}

static void
s_state_engine_do_x_disconnect(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Our node is quitting or dying.  Clean up as fast as possible.
    // Tell the peer we're going to die
    if (s_state_engine_has_peer(self)) {
        parch_msg_t *msg = parch_msg_new(PARCH_MSG_DISCONNECT);
        s_state_engine_send_msg_to_peer_and_log(self, &msg);
    }

    s_state_engine_do_disconnect_internal(self);
}

static void
s_state_engine_do_x_reset_confirmation(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y had requested that flow control be reset and X agreed.  Reset
    // the sequence numbers and flow control.

    parch_msg_t *r = parch_msg_dup(self->request);
    s_state_engine_send_msg_to_peer_and_log(self, &r);
    s_state_engine_reset_flow_control(self);
}

static void
s_state_engine_do_x_reset_request(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X has requested that data flow control be reset.  Forward the
    // message on to Y.
    parch_msg_t *msg = parch_msg_dup(self->request);
    s_state_engine_send_msg_to_peer_and_log(self, &msg);
}

static void
s_state_engine_do_x_rnr(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X has informed that it can't receive any data right now.  Store
    // info about the window and forward the message to Y.
    s_state_engine_do_x_rr_internal(self, 1);
}

static void
s_state_engine_do_x_rr(parch_state_engine_t * self) {
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
    uint32_t rs = parch_msg_sequence(self->request);

    // The node updates the range of packets it will accept.  That
    // range has to be greater than the last range, but, it can't
    // start later than the next packet expected from the peer.
    if (rs > self->x_window && rs <= self->y_sequence_number) {
        self->x_window = rs;
        self->x_not_ready = not_ready;
        parch_msg_t *msg = parch_msg_dup(self->request);
        s_state_engine_send_msg_to_peer_and_log(self, &msg);
    } else {
        // We're out of sync, so we reset the connection
        if (not_ready)
            s_state_engine_log(self, "bad RNR sequence id from node");
        else
            s_state_engine_log(self, "bad RR sequence id from node");
        self->next_event = e_reset;
        self->next_cause = local_procedure_error;
        self->next_diagnostic = err_invalid_pr;
    }
}

static void
s_state_engine_do_y_call_accepted(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X requested a call and Y agreed.

    // Forward the call request to the X node.
    parch_msg_t *reply = parch_msg_dup(self->request);

    // CALL NEGOTIATION, STEP 4

    // Swap the incoming data only and outgoing data only flags
    parch_msg_swap_incoming_and_outgoing(reply);

    // Here in STEP 4, we make sure that the peer hasn't cheated
    // on negotiation by increasing any of the parameters invalidly.
    parch_msg_apply_defaults_to_connect_request(reply);
    diagnostic_t diagnostic;
    if (parch_msg_validate_connect_request(reply, &diagnostic) == false
            || s_state_engine_validate_negotiation_parameters(self, reply, &diagnostic) == false) {
        self->state = s_unspecified;
        self->next_event = e_y_clear_request;
        self->next_cause = invalid_facility_request;
        self->next_diagnostic = diagnostic;
        s_state_engine_log(self, diagnostic_messages[diagnostic]);
        parch_msg_destroy(&reply);
    } else {
        // Make sure it has the return address
        parch_msg_set_address(reply, parch_node_get_node_address(self->node));
        s_state_engine_send_msg_to_node_and_log(self, &reply);
        s_state_engine_reset_flow_control(self);
    }
}

static void
s_state_engine_do_y_call_request(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y had requested that the broker connect it to an idle node of
    // the requested service, and the broker had determined that this
    // node is and idle node of that service.  The broker had also
    // connected this node in a virtual circuit.

    // Forward the call request to the X node.
    parch_msg_t *r = parch_msg_dup(self->request);

    // CALL NEGOTIATION, STEP 2
    // The state engine may
    // downgrade some of the requested attributes of the connection
    // before sending it to the peer.

    // Swap the incoming data only and outgoing data only flags
    s_state_engine_clear_negotiation_parameters(self);
    parch_msg_swap_incoming_and_outgoing(r);

    // Here in STEP 2, we reduce the throughput if this node state engine
    // requires it
    uint8_t i = parch_throughput_index_throttle(parch_msg_throughput(r),
            self->throughput_index);
    parch_msg_set_throughput(r, i);

    diagnostic_t diagnostic;
    if (parch_msg_validate_connect_request(r, &diagnostic) == false) {
        self->state = s_unspecified;
        self->next_event = e_y_clear_request;
        self->next_cause = invalid_facility_request;
        self->next_diagnostic = diagnostic;
        s_state_engine_log(self, diagnostic_messages[diagnostic]);
        parch_msg_destroy(&r);
    } else {
        // Store the negotiation values so far for step 3.
        s_state_engine_store_negotiation_parameters(self, r);

        // Make sure it has the return address
        parch_msg_set_address(r, parch_node_get_node_address(self->node));
        s_state_engine_send_msg_to_node_and_log(self, &r);

            // The node needs to reply to this message, else we'll timeout.
            timeout_t *t = (timeout_t *)zmalloc(sizeof(timeout_t));
            t->state_engine = self;
            t->event_id = self->event_id;
            t->iteration = 0;
            parch_broker_add_timer (self->broker, TIMEOUT_T11, s_state_engine_timeout_t11_callback, t);
    }
}

static void
s_state_engine_do_y_clear_request(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // The Y node has requested that the connection to X be cleared.
    // The clear request is forwarded on to X.
    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    s_state_engine_send_msg_to_node_and_log(self, &r);
    s_state_engine_reset_flow_control(self);
}

static void
s_state_engine_do_y_clear_confirmation(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has agreed to clear the channel.  So, tell the broker to
    // close the connection to Y and end the virtual call.  Then reset
    // flow control.

    parch_msg_t *r = parch_msg_dup(self->request);

    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    s_state_engine_send_msg_to_node_and_log(self, &r);

    // Disconnect from peer
    s_state_engine_log(self, "clearing");
    parch_node_disconnect_from_peer(self->node);
    s_state_engine_reset_flow_control(self);
}

static void
s_state_engine_do_y_reset_request(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has requested that data flow control be reset.  Forward the
    // message on to X.
    // Forward the call request to the X node.
    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    s_state_engine_send_msg_to_node_and_log(self, &r);
}

static void
s_state_engine_do_y_reset_confirmation(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    parch_msg_t *r = parch_msg_dup(self->request);
    // Make sure it has the return address
    parch_msg_set_address(r, parch_node_get_node_address(self->node));
    s_state_engine_send_msg_to_node_and_log(self, &r);
    s_state_engine_reset_flow_control(self);
}

static void
s_state_engine_do_y_data(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has sent data for X.  Verify that the DATA packet has the
    // right sequence number and that it is within X's transmission
    // window.  If it is, forward it to X.  If it isn't, reject it.

    assert(self->request);
    assert(parch_msg_id(self->request) == PARCH_MSG_DATA);
    parch_msg_t *msg = parch_msg_dup(self->request);

    bool ok = true;
    if (self->call_incoming_data_barred) {
        ok = false;
        self->next_cause = access_barred;
        self->next_diagnostic = err_incoming_data_barred;
    } else if (zframe_size(parch_msg_data(msg)) == 0) {
        ok = false;
        self->next_cause = local_procedure_error;
        self->next_diagnostic = err_packet_too_short;
    } else if (zframe_size(parch_msg_data(msg)) > parch_packet_bytes(self->call_packet_index)) {
        ok = false;
        self->next_cause = local_procedure_error;
        self->next_diagnostic = err_packet_too_long;
    } else if ((parch_msg_sequence(msg) != self->y_sequence_number)
            || (parch_msg_sequence(msg) < self->x_window || parch_msg_sequence(msg) > self->x_window + self->call_window_size)) {
        ok = false;
        self->next_cause = local_procedure_error;
        self->next_diagnostic = err_invalid_ps;
    }

    if (ok == false) {
        self->state = s_unspecified;
        self->next_event = e_reset;
        s_state_engine_log(self, diagnostic_messages[err_incoming_data_barred]);
        parch_msg_destroy(&msg);
    } else {
        int64_t now = zclock_time();
        int64_t delta = now - self->_time;

        // A leaky bucky with a 5 second time drain time
        self->channel_capacity_in_use -= (float) delta / 5000.0f;
        if (self->channel_capacity_in_use < 0.0)
            self->channel_capacity_in_use = 0.0;
        // bytes * (8 bits / byte) * (sec / bit) / (5 sec)
        self->channel_capacity_in_use += (float) (zframe_size(parch_msg_data(msg)) * 8) / (5.0 * parch_throughput_bps(self->throughput_index));

        if (self->channel_capacity_in_use > 2.0) {
            self->state = s_unspecified;
            self->next_event = e_reset;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_network_congestion;
            s_state_engine_log(self, diagnostic_messages[err_network_congestion]);
            parch_msg_destroy(&msg);
        }

        parch_msg_set_address(msg, parch_node_get_node_address(self->node));
        s_state_engine_send_msg_to_node_and_log(self, &msg);
        self->y_sequence_number++;
    }
}

static void
s_state_engine_do_y_disconnect(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // Y has requested a disconnect.
    // Forward this on to the node so it knows that its peer has disappeared.
    parch_msg_t *r = parch_msg_dup(self->request);
    s_state_engine_reset_flow_control(self);
    s_state_engine_send_msg_to_node_and_log(self, &r);
}

static void
s_state_engine_do_y_rnr(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    s_state_engine_do_y_rr_internal(self, 1);
    // Y has informed that it can't receive any data right now.  Store
    // info about the window and forward the message to X.
}

static void
s_state_engine_do_y_rr(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    s_state_engine_do_y_rr_internal(self, 0);
    // Y had updated its allowed transmission window.  Store info
    // about the window and then forward the message to X.
}

static void
s_state_engine_do_y_rr_internal(parch_state_engine_t *self, int not_ready) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // X had updated its allowed transmission window.  Store info
    // about the window and then forward the message to Y.
    uint32_t rs = parch_msg_sequence(self->request);

    // The node updates the range of packets it will accept.  That
    // range has to be greater than the last range, but, it can't
    // start later than the next packet expected from the peer.
    if (rs > self->y_window && rs <= self->x_sequence_number) {
        self->y_window = rs;
        self->y_not_ready = not_ready;
        parch_msg_t *msg = parch_msg_dup(self->request);
        parch_msg_set_address(msg, parch_node_get_node_address(self->node));
        s_state_engine_send_msg_to_node_and_log(self, &msg);
    } else {
        // We're out of sync, so we reset the connection
        if (not_ready)
            s_state_engine_log(self, "bad RNR sequence id from peer");
        else
            s_state_engine_log(self, "bad RR sequence id from peer");
        self->next_event = e_reset;
        self->next_cause = local_procedure_error;
        self->next_diagnostic = err_invalid_pr;
    }
}

static bool
s_state_engine_has_peer(parch_state_engine_t *self) {
    return (parch_node_get_peer(self->node) != NULL);
}

static void
s_state_engine_log(parch_state_engine_t * self, const char *text) {
    if (parch_broker_get_verbose(self->broker)) {
        char *self_addr;
        if (parch_node_get_node_address(self->node) != NULL)
            self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        else
            self_addr = strdup("UNKNOWN");
        zclock_log("I: [%s] state engine -- %s",
                self_addr,
                text);
        free(self_addr);
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
            d = (diagnostic_t) parch_msg_diagnostic(msg);
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
    node_t * peer = parch_node_get_peer(self->node);

    // Log the sending of the message
    if (parch_broker_get_verbose(self->broker)) {
        diagnostic_t d = err_none;
        char *self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        char *peer_addr = zframe_strhex(parch_node_get_node_address(peer));
        if (parch_msg_id(msg) == PARCH_MSG_CLEAR_REQUEST) {
            d = (diagnostic_t) parch_msg_diagnostic(msg);
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
s_state_engine_reset_flow_control(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    self->x_sequence_number = 0;
    self->y_sequence_number = 0;
    self->x_window = 0;
    self->y_window = 0;
    self->x_not_ready = 0;
    self->y_not_ready = 0;
}

static void
s_state_engine_store_negotiation_parameters(parch_state_engine_t *self, parch_msg_t *r) {
    self->call_incoming_data_barred = parch_msg_incoming(r);
    self->call_outgoing_data_barred = parch_msg_outgoing(r);
    self->call_throughput_index = parch_msg_throughput(r);
    self->call_packet_index = parch_msg_packet(r);
    self->call_window_size = parch_msg_window(r);
}

// This is the timeout timer for when we send CALL_REQUEST to the node
static int
s_state_engine_timeout_t11_callback (zloop_t *loop __attribute__((unused)), zmq_pollitem_t *item __attribute__((unused)), void *arg) {
    timeout_t *t = (timeout_t *) arg;
    if (t->state_engine->event_id == t->event_id) {
        // We're still stuck on this event
        state_machine_dispatch(t->state_engine, a_clear, err_time_expired_for_x_call_request, s7_y_clear);
    }
    return 0;
}

// This is the timeout timer for when we send CLEAR_REQUEST to the node
static int
s_state_engine_timeout_t13_callback (zloop_t *loop __attribute__((unused)), zmq_pollitem_t *item __attribute__((unused)), void *arg) {
    timeout_t *t = (timeout_t *) arg;
    if (t->state_engine->event_id == t->event_id) {
        // We're still stuck on this event
        if (t->iteration == 0)
                state_machine_dispatch(t->state_engine, a_clear, err_time_expired_for_x_call_request, s7_y_clear);
        else
                state_machine_dispatch(t->state_engine, a_disconnect, err_none, s0_disconnected);
        t->state_engine->timeout_count ++;
    }
    return 0;
}

// This is the timeout timer for when we send CALL_REQUEST to the peer
static int
s_state_engine_timeout_t21_callback (zloop_t *loop __attribute__((unused)), zmq_pollitem_t *item __attribute__((unused)), void *arg) {
    timeout_t *t = (timeout_t *) arg;
    if (t->state_engine->event_id == t->event_id) {
        // We're still stuck on this event
        state_machine_dispatch(t->state_engine, a_clear, err_time_expired_for_x_call_request, s7_y_clear);
    }
    return 0;
}

#if 0
// This is the timeout timer for when we send RESET_REQUEST to the peer
static int
s_state_engine_timeout_t22_callback (zloop_t *loop __attribute__((unused)), zmq_pollitem_t *item __attribute__((unused)), void *arg) {
    timeout_t *t = (timeout_t *) arg;
    if (t->state_engine->event_id == t->event_id) {
        // We're still stuck on this event
        if (t->state_engine->n_timeout == 0)
            state_machine_dispatch(t->state_engine, a_reset, err_time_expired_for_x_call_request, s9_y_reset);
        else
            state_machine_dispatch(t->state_engine, a_clear, err_time_expired_for_x_call_request, s7_y_clear);
    }
    return 0;
}
#endif

// During the negotiation process, each step is only allowed to make things more restrictive.
// Can't make things less restrictive.

static bool
s_state_engine_validate_negotiation_parameters(parch_state_engine_t *self, parch_msg_t *msg, diagnostic_t *diagnostic) {
    bool valid = true;
    *diagnostic = err_none;
    if (self->call_incoming_data_barred == 1 && parch_msg_incoming(msg) == 0) {
        // The node's not allow to remove the incoming calls barred flag
        valid = false;
        *diagnostic = err_invalid_negotiation__incoming_data_barred;
    } else if (self->call_outgoing_data_barred == 1 && parch_msg_outgoing(msg) == 0) {
        // The node's not allow to remove the outgoing calls barred flag
        valid = false;
        *diagnostic = err_invalid_negotiation__outgoing_data_barred;
    } else if (parch_msg_incoming(msg) == 1 && parch_msg_outgoing(msg) == 1) {
        // A channel can't bar traffic in both directions
        valid = false;
        *diagnostic = err_data_barred;
    } else if (!parch_throughput_index_negotiate(parch_msg_throughput(msg), self->call_throughput_index).ok) {
        // The node is not allowed to increase the throughput or set it to one of the
        valid = false;
        *diagnostic = err_invalid_negotiation__throughput;
    } else if (!parch_window_negotiate(parch_msg_window(msg), self->call_window_size).ok) {

        valid = false;
        *diagnostic = err_invalid_negotiation__window_size;
    } else if (!parch_packet_index_negotiate(parch_msg_packet(msg), self->call_packet_index).ok) {
        // The node only can change the packet size to be between the requested value and 128.
        valid = false;
        *diagnostic = err_invalid_negotiation__packet_size;
    }
    return valid;
}

// ================================================================
// State machine actions










#if 0

static void
state_engine_do_s0_initialize_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    // We've been in S0 for a while without registering.  Disconnect
}

static void
state_engine_do_s2_x_call_waiting_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s3_y_call_waiting_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s6_x_clear_request_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s7_y_clear_request_first_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s7_y_clear_request_second_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s8_x_reset_request_first_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s8_x_reset_request_second_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s9_y_reset_request_first_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}

static void
state_engine_do_s9_y_reset_request_second_timeout(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

}
#endif

static void
state_engine_config_self(parch_state_engine_t * self __attribute__((unused))) {
    // state engine is still partly uninitialized here.
    // Can't do STATE_ENGINE_VALIDITY_CHECKS(self);
}

static int
state_machine_dispatch(parch_state_engine_t *self, action_t action, diagnostic_t diagnostic, state_t next) {
    STATE_ENGINE_VALIDITY_CHECKS(self);

    self->event = self->next_event;
    self->next_event = e_unspecified;
    self->next_diagnostic = err_none;

    state_t state_orig = self->state;
    char *node_addr = zframe_strhex(parch_node_get_node_address(self->node));
    if (parch_broker_get_verbose(self->broker))
        zclock_log("I: [%s] state engine -- doing '%s' action in '%s'",
            node_addr,
            action_names[action], state_names[self->state]);
    free(node_addr);
    usleep(1);
    self->event_id ++;
    switch (action) {
        case a_unspecified:
        case a_discard:
            break;
        case a_reset:
            s_state_engine_do_reset(self, diagnostic);
            break;
        case a_clear:
            assert(next == s7_y_clear);
            s_state_engine_do_clear(self, diagnostic);
            break;
        case a_disconnect:
            s_state_engine_do_disconnect(self, diagnostic);
            break;
        case a_x_connect:
            // x_connect is handled by the broker.
            break;
        case a_x_disconnect:
            s_state_engine_do_x_disconnect(self);
            break;
        case a_x_call_request:
            if (self->outgoing_calls_barred == 0)
                s_state_engine_do_x_call_request(self);
            else
                s_state_engine_log(self, "x call request discarded, outgoing calls barred");
            break;
        case a_x_call_accepted:
            s_state_engine_do_x_call_accepted(self);
            break;
        case a_x_clear_request:
            s_state_engine_do_x_clear_request(self);
            break;
        case a_x_clear_confirmation:
            s_state_engine_do_x_clear_confirmation(self);
            break;
        case a_x_data:
            s_state_engine_do_x_data(self);
            break;
        case a_x_rr:
            s_state_engine_do_x_rr(self);
            break;
        case a_x_rnr:
            s_state_engine_do_x_rnr(self);
            break;
        case a_x_reset:
            s_state_engine_do_x_reset_request(self);
            break;
        case a_x_reset_confirmation:
            s_state_engine_do_x_reset_confirmation(self);
            break;

        case a_y_disconnect:
            s_state_engine_do_y_disconnect(self);
            break;
        case a_y_call_request:
            if (self->incoming_calls_barred == 0)
                s_state_engine_do_y_call_request(self);
            else
                s_state_engine_log(self, "y call request discarded, incoming calls barred");
            break;
        case a_y_call_accepted:
            s_state_engine_do_y_call_accepted(self);
            break;
        case a_y_clear_request:
            s_state_engine_do_y_clear_request(self);
            break;
        case a_y_clear_confirmation:
            s_state_engine_do_y_clear_confirmation(self);
            break;
        case a_y_data:
            s_state_engine_do_y_data(self);
            break;
        case a_y_rr:
            s_state_engine_do_y_rr(self);
            break;
        case a_y_rnr:
            s_state_engine_do_y_rnr(self);
            break;
        case a_y_reset:
            s_state_engine_do_y_reset_request(self);
            break;
        case a_y_reset_confirmation:
            s_state_engine_do_y_reset_confirmation(self);
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
    if (next != s_unspecified)
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
state_machine_dispatch_event(parch_state_engine_t * self) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    assert(self->next_event >= e_x_connect && self->next_event <= e_max);

    int done = 0;
    while (self->next_event != e_unspecified && done == 0) {
        state_t s = self->state;
        event_t e = self->next_event;
        diagnostic_t d = self->next_diagnostic;
        const state_machine_table_element_t * elt = &state_machine_table[s][e];
        if (d != err_none) {
            // Some events, like reset and clear events, override the state machine's generic
            // diagnostic message with something more specific.
            done = state_machine_dispatch(self, elt->action, d, elt->next);
            self->next_diagnostic = err_none;
        } else
            // But usually, we just have a diagnostic message based on the state of the table.
            done = state_machine_dispatch(self, elt->action, elt->diagnostic, elt->next);

    }
    return done;
}

parch_state_engine_t *
parch_state_engine_new(broker_t *broker, node_t * node, byte incoming_barred, byte outgoing_barred, byte throughput) {
    parch_state_engine_t *self = (parch_state_engine_t *) zmalloc(sizeof (parch_state_engine_t));
    self->broker = broker;
    self->node = node;
    self->state = s0_disconnected;
    self->event = e_unspecified;
    self->event_id = 0;
    self->next_event = e_unspecified;
    self->request = NULL;
    self->incoming_calls_barred = incoming_barred;
    self->outgoing_calls_barred = outgoing_barred;
    self->throughput_index = throughput;
    self->_time = zclock_time();
    self->channel_capacity_in_use = 0.0;
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
    self->request = parch_msg_dup(msg);
    self->next_event = s_msg_id_x_next_event(parch_msg_id(msg));
    int done = state_machine_dispatch_event(self);

    STATE_ENGINE_VALIDITY_CHECKS(self);
    return done;
}

static int
s_state_engine_y_message(parch_state_engine_t *self, parch_msg_t **msg_p) {
    STATE_ENGINE_VALIDITY_CHECKS(self);
    assert(msg_p);
    assert(msg_p[0]);
    parch_msg_t *msg = msg_p[0];

    if (parch_broker_get_verbose(self->broker)) {
        char * const self_addr = zframe_strhex(parch_node_get_node_address(self->node));
        zclock_log("I: [%s] socket -- received %s msg from peer", self_addr, parch_msg_command(msg));
        free(self_addr);
        parch_msg_dump(msg);
    }

    self->request = parch_msg_dup(msg);
    self->next_event = s_msg_id_y_next_event(parch_msg_id(msg));
    int done = state_machine_dispatch_event(self);
    parch_msg_destroy(msg_p);
    return done;
}

static event_t
s_msg_id_x_next_event(int id) {
    event_t ret;
    switch (id) {
        case PARCH_MSG_DATA:
            ret = e_x_data;
            break;

        case PARCH_MSG_RR:
            ret = e_x_rr;
            break;

        case PARCH_MSG_RNR:
            ret = e_x_rnr;
            break;

        case PARCH_MSG_CALL_REQUEST:
            ret = e_x_call_request;
            break;

        case PARCH_MSG_CALL_ACCEPTED:
            ret = e_x_call_accepted;
            break;

        case PARCH_MSG_CLEAR_REQUEST:
            ret = e_x_clear_request;
            break;

        case PARCH_MSG_CLEAR_CONFIRMATION:
            ret = e_x_clear_confirmation;
            break;

        case PARCH_MSG_RESET_REQUEST:
            ret = e_x_reset_request;
            break;

        case PARCH_MSG_RESET_CONFIRMATION:
            ret = e_x_reset_confirmation;
            break;

        case PARCH_MSG_CONNECT:
            ret = e_x_connect;
            break;

        case PARCH_MSG_DISCONNECT:
            ret = e_x_disconnect;
            break;
    }
    return ret;
}

static event_t
s_msg_id_y_next_event(int id) {
    event_t ret;
    switch (id) {
        case PARCH_MSG_DATA:
            ret = e_y_data;
            break;

        case PARCH_MSG_RR:
            ret = e_y_rr;
            break;

        case PARCH_MSG_RNR:
            ret = e_y_rnr;
            break;

        case PARCH_MSG_CALL_REQUEST:
            ret = e_y_call_request;
            break;

        case PARCH_MSG_CALL_ACCEPTED:
            ret = e_y_call_accepted;
            break;

        case PARCH_MSG_CLEAR_REQUEST:
            ret = e_y_clear_request;
            break;

        case PARCH_MSG_CLEAR_CONFIRMATION:
            ret = e_y_clear_confirmation;
            break;

        case PARCH_MSG_RESET_REQUEST:
            ret = e_y_reset_request;
            break;

        case PARCH_MSG_RESET_CONFIRMATION:
            ret = e_y_reset_confirmation;
            break;
        case PARCH_MSG_DISCONNECT:
            ret = e_y_disconnect;
            break;
    }
    return ret;
}

