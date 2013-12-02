/*
    diag.h - diagnostic information

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
    d_worker_originated = 1,

    // For c_number_busy,
    d_number_busy = 2,

    // For c_call_collision
    d_call_collision = 3,

    // For c_zmq_sendmsg_error.  There are the 'errno' codes
    // returned when zmq_sendmsg fails
    d_zmq_error = 4,
    d_zmq_eagain = 5,
    d_zmq_enotsup = 6,
    d_zmq_efsm = 7,
    d_zmq_eterm = 8,
    d_zmq_enotsock = 9,
    d_zmq_eintr = 10,
    d_zmq_efault = 11,

    // For c_malformed_message
    d_invalid_q = 12,
    d_pr_too_large = 13,
    d_ps_too_large = 14,
    d_invalid_data_frame = 15,
    d_calling_address_too_short = 16,
    d_calling_address_too_long = 17,
    d_calling_address_format_invalid = 18,
    d_called_address_too_short = 19,
    d_called_address_too_long = 20,
    d_called_address_format_invalid = 21,
    d_packet_facility_too_small = 22,
    d_packet_facility_too_large = 23,
    d_window_facility_too_small = 24,
    d_window_facility_too_large = 25,
    d_throughput_facility_too_small = 26,
    d_throughput_facility_too_large = 27,
    d_invalid_cause = 28,
    d_invalid_diagnostic = 29,
    d_invalid_q_too_large = 30,
    d_invalid_pr_too_small = 31,
    d_invalid_pr_too_large = 32,
    d_data_too_short = 33,
    d_data_too_long = 34,
    d_invalid_directionality_facility = 35,

    // For c_invalid_facility_request
    d_invalid_packet_facility_negotiation = 36,
    d_invalid_window_facility_negotiation = 37,
    d_invalid_throughput_facility_negotiation = 38,
    d_invalid_directionality_negotiation = 39,

    // For c_invalid_forwarding_request
    d_callee_forwarding_not_allowed = 40,
    d_caller_forwarding_not_allowed = 41,

    // For c_access_barred
    d_output_barred = 42,
    d_input_barred = 43,

    // for c_address_in_use
    d_reserved_address = 44,         /* the "operator" address is reserved */
    d_address_in_use = 45,

    // for c_unknown_address
    d_unknown_worker_address = 46,
    d_unknown_reserved_address = 47, /* the "operator" is not online */

    // for c_network_congestion
    d_no_connections_available = 48,
    d_no_channels_available = 49,

    // for procedure errors
    d_invalid_message_for_state_ready = 50,
    d_invalid_message_for_state_x_call_request = 51,
    d_invalid_message_for_state_y_call_request = 52,
    d_invalid_message_for_state_data_transfer = 53,
    d_invalid_message_for_state_call_collision = 54,
    d_invalid_message_for_state_x_clear_request = 55,
    d_invalid_message_for_state_y_clear_request = 56,
    d_invalid_message_for_state_x_reset_request = 57,
    d_invalid_message_for_state_y_reset_request = 58,
    d_ps_out_of_order = 59,
    d_ps_not_in_window = 60,
    d_pr_invalid_window_update = 61,
    d_data_too_long_for_packet_facility = 62,

    // for quota_exceeded
    d_data_rate_exceeded = 63,
    d_message_rate_exceeded = 64,

    d_last = d_message_rate_exceeded
} diag_t;

diag_t errno2diag();
const char *diag_name(diag_t D);


#endif
