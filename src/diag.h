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
    d_unspecified = 0,

    // For c_worker_originated,
    d_worker_originated = 10,

    // For c_number_busy,
    d_number_busy = 20,

    // For c_call_collision
    d_call_collision = 30,

    // For c_zmq_sendmsg_error.  There are the 'errno' codes
    // returned when zmq_sendmsg fails
    d_zmq_error = 40,
    d_zmq_eagain = 41,
    d_zmq_enotsup = 42,
    d_zmq_efsm = 43,
    d_zmq_eterm = 44,
    d_zmq_enotsock = 45,
    d_zmq_eintr = 46,
    d_zmq_efault = 47,

    // For c_malformed_message
    d_invalid_q = 50,
    d_pr_too_large = 51,
    d_ps_too_large = 52,
    d_invalid_data_frame = 53,
    d_calling_address_too_short = 54,
    d_calling_address_too_long = 55,
    d_calling_address_format_invalid = 56,
    d_called_address_too_short = 57,
    d_called_address_too_long = 58,
    d_called_address_format_invalid = 59,
    d_packet_facility_too_small = 60,
    d_packet_facility_too_large = 61,
    d_window_facility_too_small = 62,
    d_window_facility_too_large = 63,
    d_throughput_facility_too_small = 64,
    d_throughput_facility_too_large = 65,
    d_invalid_cause = 66,
    d_invalid_diagnostic = 67,
    d_invalid_q_too_large = 68,
    d_invalid_pr_too_small = 69,
    d_invalid_pr_too_large = 70,
    d_data_too_short = 71,
    d_data_too_long = 72,
    d_invalid_directionality_facility = 73,

    // For c_invalid_facility_request
    d_invalid_packet_facility_negotiation = 80,
    d_invalid_window_facility_negotiation = 81,
    d_invalid_throughput_facility_negotiation = 82,
    d_invalid_directionality_negotiation = 83,

    // For c_invalid_forwarding_request
    d_callee_forwarding_not_allowed = 90,
    d_caller_forwarding_not_allowed = 91,

    // For c_access_barred
    d_output_barred = 100,
    d_input_barred = 101,

    // for c_address_in_use
    d_reserved_address = 110,         /* the "operator" address is reserved */
    d_address_in_use = 111,

    // for c_unknown_address
    d_unknown_worker_address = 120,
    d_unknown_reserved_address = 121, /* the "operator" is not online */

    // for c_network_congestion
    d_no_connections_available = 130,
    d_no_channels_available = 131,

    // for procedure errors
    d_invalid_message_for_state_ready = 140,
    d_invalid_message_for_state_x_call_request = 141,
    d_invalid_message_for_state_y_call_request = 142,
    d_invalid_message_for_state_data_transfer = 143,
    d_invalid_message_for_state_call_collision = 144,
    d_invalid_message_for_state_x_clear_request = 145,
    d_invalid_message_for_state_y_clear_request = 146,
    d_invalid_message_for_state_x_reset_request = 147,
    d_invalid_message_for_state_y_reset_request = 148,
    d_ps_out_of_order = 149,
    d_ps_not_in_window = 150,
    d_pr_invalid_window_update = 151,
    d_data_too_long_for_packet_facility = 152,

    // for quota_exceeded
    d_data_rate_exceeded = 160,
    d_message_rate_exceeded = 161

} diag_t;

diag_t errno2diag();

#endif
