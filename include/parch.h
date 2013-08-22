/*  =========================================================================
    parch.h - public header file

    -------------------------------------------------------------------------
    Copyright (c) 2013 - Michael L. Gran - http://lonelycactus.com
    Copyright other contributors as noted in the AUTHORS file.

    This file is part of petulant-archer, A ZeroMQ-based networking
    library implementing the Switched Virtual Circuit pattern.

    http://github.com/spk121/petulant-archer

    This is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANT-
    ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
    Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not see http://www.gnu.org/licenses.
    =========================================================================
*/


#ifndef __PARCH_H_INCLUDED__
#define __PARCH_H_INCLUDED__

#define _GNU_SOURCE

// PARCH version macros for compile-time API detection
#define PARCH_VERSION_MAJOR 0
#define PARCH_VERSION_MINOR 1
#define PARCH_VERSION_PATCH 0

#define PARCH_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define PARCH_VERSION \
    PARCH_MAKE_VERSION(MYPROJ_VERSION_MAJOR, \
                        MYPROJ_VERSION_MINOR, \
                        MYPROJ_VERSION_PATCH)



#include <czmq.h>
#if CZMQ_VERSION < 10402
#   error "petulant-archer needs CZMQ/1.4.2 or later"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>
#include <stdbool.h>

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

    // My extensions
    err_incoming_data_barred,
    err_outgoing_data_barred,
    err_network_congestion,
    err_data_barred,
    err_throughput_out_of_range,
    err_packet_size_out_of_range,
    err_window_size_out_of_range,
    err_invalid_negotiation__incoming_data_barred,
    err_invalid_negotiation__outgoing_data_barred,
    err_invalid_negotiation__throughput,
    err_invalid_negotiation__window_size,
    err_invalid_negotiation__packet_size,
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
    err_last = err_maintenance_action,
};

typedef enum _diagnostic_t diagnostic_t;

//  Opaque class structures
typedef struct _parch_state_engine_t parch_state_engine_t;

#include "lib.h"

//  Classes in the API
#include "parch_msg.h"
#include "parch_msg2.h"
// #include "parch_common.h"
#include "parch_node.h"
#include "parch_state_engine.h"
#include "parch_broker.h"
#include "parch_throughput.h"

#ifdef __cplusplus
}
#endif

#endif
