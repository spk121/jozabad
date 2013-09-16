/*
    diagnostic.h - diagnostic information

    Copyright 2013 Michael L. Gran <spk121@yahoo.com>

    This file is part of Jozabad.

    Jozabad is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jozabad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jozabad.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef JOZA_DIAG_H
#define JOZA_DIAG_H

typedef enum {
    // For c_unspecified
    d_unspecified,

    // For c_worker_originated,
    d_worker_originated,

    // For c_number_busy,
    d_number_busy,

    // For c_call_collision
    d_call_collision,

    // For c_zmq_sendmsg_error.  There are the 'errno' codes
    // returned when zmq_sendmsg fails
    d_zmq_error,
    d_zmq_eagain,
    d_zmq_enotsup,
    d_zmq_efsm,
    d_zmq_eterm,
    d_zmq_enotsock,
    d_zmq_eintr,
    d_zmq_efault,

    // For c_malformed_message
    d_invalid_q,
    d_pr_too_large,
    d_ps_too_large,
    d_invalid_data_frame,
    d_calling_address_too_short,
    d_calling_address_too_long,
    d_calling_address_format_invalid,
    d_called_address_too_short,
    d_called_address_too_long,
    d_called_address_format_invalid,
    d_packet_facility_too_small,
    d_packet_facility_too_large,
    d_window_facility_too_small,
    d_window_facility_too_large,
    d_throughput_facility_too_small,
    d_throughput_facility_too_large,
    d_invalid_cause,
    d_invalid_diagnostic,
    d_invalid_q_too_large,
    d_invalid_pr_too_small,
    d_invalid_pr_too_large,
    d_data_too_short,
    d_data_too_long,
    d_invalid_directionality_facility,
    
    // For c_invalid_facility_request
    d_invalid_packet_facility_negotiation,
    d_invalid_window_facility_negotiation,
    d_invalid_throughput_facility_negotiation,
    d_invalid_directionality_negotiation,

    // For c_invalid_forwarding_request
    d_callee_forwarding_not_allowed,
    d_caller_forwarding_not_allowed,

    // For c_access_barred
    d_output_barred,
    d_input_barred,

    // for c_address_in_use
    d_reserved_address,         /* the "operator" address is reserved */
    d_address_in_use,

    // for c_unknown_address
    d_unknown_worker_address,
    d_unknown_reserved_address, /* the "operator" is not online */

    // for c_network_congestion
    d_no_connections_availabie,
    d_no_channels_available,

    // for procedure errors
    d_invalid_message_for_state_ready,
    d_invalid_message_for_state_x_call_request,
    d_invalid_message_for_state_y_call_request,
    d_invalid_message_for_state_data_transfer,
    d_invalid_message_for_state_call_collision,
    d_invalid_message_for_state_x_clear_request,
    d_invalid_message_for_state_y_clear_request,
    d_invalid_message_for_state_x_reset_request,
    d_invalid_message_for_state_y_reset_request,
    d_ps_out_of_order,
    d_ps_not_in_window,
    d_pr_invalid_window_update,
    d_data_too_long_for_packet_facility,

    // for quota_exceeded
    d_data_rate_exceeded,
    d_message_rate_exceeded

} diag_t;

diag_t errno2diag();

#endif
