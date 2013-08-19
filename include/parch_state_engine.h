#ifndef __STATE_ENGINE_H_INCLUDED__
#define __STATE_ENGINE_H_INCLUDED__

#include "parch_broker.h"

#ifdef __cplusplus
extern "C" {
#endif

// data structures, macros, typedefs, functions

//  ---------------------------------------------------------------------------
//  TYPEDEFS
typedef enum _action_t action_t;
typedef enum _diagnostic_t diagnostic_t;
typedef enum _event_t event_t;
typedef enum _state_t state_t;

// These mostly come from TABLE 5-7/X.25

enum _clearing_cause_t {
    number_busy = 1,
    out_of_order = 9,
    remote_procedure_error = 17,
    // reverse_charging_acceptance_not_subscribed = 25,
    incompatible_destination = 33,
    // fast_select_acceptance_not_subscribed = 41,
    // ship_absent = 57,                  // LOL. This is in X.25, I swear.
    invalid_facility_request = 3,         // bad CONNECT or CALL_REQUEST parameter
    access_barred = 11,
    local_procedure_error = 19,
    network_congestion = 5,
    not_obtainable = 13,
    // roa_out_of_order = 21
};

// These mostly come from E.1/X.25.  Probably only a handful are still relevant.
enum _diagnostic_t {
    err_none,

    // No additional information
    err_invalid_ps,
    err_first = err_invalid_ps,
    err_invalid_pr,

    // Packet type invalid
    err_packet_type_invalid_for_state_s0,
    err_packet_type_invalid_for_state_s1,
    err_packet_type_invalid_for_state_s2,
    err_packet_type_invalid_for_state_s3,
    err_packet_type_invalid_for_state_s4,
    err_packet_type_invalid_for_state_s5,
    err_packet_type_invalid_for_state_s6,
    err_packet_type_invalid_for_state_s7,
    err_packet_type_invalid_for_state_s8,
    err_packet_type_invalid_for_state_s9,

    // Packet not allowed
    err_unidentifiable_packet,
    err_call_on_one_way_logical_channel,
    err_invalid_packet_type_on_a_permanent_virtual_circuit,
    err_packet_on_unassigned_logical_channel,
    err_reject_not_subscribed_to,
    err_packet_too_short,
    err_packet_too_long,
    err_invalid_general_format_identifier,
    // err_restart_packet_with_non_zero_logical_channel_number,
    err_packet_type_not_compatible_with_facility,
    // err_unauthorized_interrupt_confirmation,
    // err_unauthorized_interrupt,
    // err_unauthorized_reject,
    // err_toa_npi_address_subscription_facility_not_subscribed_to,

    // Time expired
    err_time_expired_for_y_call_request,
    err_time_expired_for_y_clear_request,
    err_time_expired_for_y_reset_request,
    // err_time_expired_for_y_restart_request,
    err_time_expired_for_call_deflection,

    // Call set-up and call clearing
    err_facility_code_not_allowed,
    err_facility_parameter_not_allowed,
    err_invalid_called_address,
    err_invalid_calling_address,
    err_incoming_call_barred,
    err_no_logical_channel_available,
    err_call_collision,
    err_duplicate_facility_request,
    err_non_zero_address_length,
    err_non_zero_facility_length,
    err_facility_not_provided_when_expected,
    err_invalid_itu_t_specified_x_facility,
    err_maximum_number_of_call_redirections_exceeded,

    // Miscellaneous
    err_improper_cause_code_from_x,
    err_not_aligned_octet,
    err_inconsistent_q_bit_setting,
    err_nui_problem,
    err_icrd_problem,

    // Not assigned
    err_not_assigned,

    // International problem
    err_remote_network_problem,
    err_international_protocol_problem,
    err_international_link_out_of_order,
    err_international_link_busy,
    err_transit_network_facility_problem,
    err_remote_network_facility_problem,
    err_international_routing_problem,
    err_temporary_routing_problem,
    err_unknown_called_dnic,
    err_maintenance_action,
    err_last = err_maintenance_action
};

//  Create a new state engine instance
CZMQ_EXPORT parch_state_engine_t *
    parch_state_engine_new(broker_t *broker, node_t * node, byte incoming_barred, byte outgoing_barred, byte throughput);


//  Destroy a state engine instance
CZMQ_EXPORT void
    parch_state_engine_destroy (parch_state_engine_t **self_p);

int
parch_state_engine_x_message (parch_state_engine_t *self, parch_msg_t *msg);


//  Self test of this class
void
    parch_state_engine_test (bool verbose);

#ifdef __cplusplus
}
#endif

#endif
