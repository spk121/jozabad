/*
    diag.h - diagnostic information

    Copyright 2013, 2014 Michael L. Gran <spk121@yahoo.com>

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

/**
 * @file diag.h
 * @brief List of specific errors for diagnostic messages
 *
 * Diagnostic messages have a general 'cause' and a more specific
 * 'diagnostic'.
 */

typedef enum {
    d_unspecified = 0, /**< for c_unspecified */
    d_worker_originated = 1, /**< for c_worker_originated */
    d_number_busy = 2, /**< for c_number_busy */
    d_call_collision = 3, /**< for c_call_collision */

    // For c_zmq_sendmsg_error.  There are the 'errno' codes
    // returned when zmq_sendmsg fails
    d_zmq_error = 4,   /**< for c_zmq_sendmsg_error, a generic error */
    d_zmq_eagain = 5,  /**< for c_zmq_sendmsg_error, no available messages in non-blocking mode */
    d_zmq_enotsup = 6, /**< for c_zmq_sendmsg_error, not supported by this socket type */
    d_zmq_efsm = 7,    /**< for c_zmq_sendmsg_error, socket not in correct state */
    d_zmq_eterm = 8,   /**< for c_zmq_sendmsg_error, socket was terminated */
    d_zmq_enotsock = 9, /**< for c_zmq_sendmsg_error, socket is invalid */
    d_zmq_eintr = 10,  /**< for c_zmq_sendmsg_error, operation was interrupted by signal */
    d_zmq_efault = 11, /**< for c_zmq_sendmsg_error, message is invalid */

    // For c_malformed_message
    d_invalid_q = 12,     /**< for c_malformed_message, UNUSED: bad data Q */
    d_pr_too_large = 13,  /**< for c_malformed_message, PR too large */
    d_ps_too_large = 14,  /**< for c_malformed_message, PS too large */
    d_invalid_data_frame = 15,  /**< for c_malformed_message, UNUSED */
    d_calling_address_too_short = 16, /**< for c_malformed_message, address has zero length */
    d_calling_address_too_long = 17,  /**< for c_malformed_message, address is too long */
    d_calling_address_format_invalid = 18,  /**< for c_malformed_message, address has disallowed characters */
    d_called_address_too_short = 19,  /**< for c_malformed_message, address has zero length */
    d_called_address_too_long = 20,  /**< for c_malformed_message, address is too long */
    d_called_address_format_invalid = 21, /**< for c_malformed_message, address has disallowed characters */
    d_packet_facility_too_small = 22, /**< for c_malformed_message, the packet size enum is too small */
    d_packet_facility_too_large = 23 /**< for c_malformed_message, the packet size enum is too large */
                                  d_window_facility_too_small = 24, /**< for c_malformed_message, the sequence window is too small */
                                  d_window_facility_too_large = 25, /**< for c_malformed_message, the sequence window is too large */
                                  d_throughput_facility_too_small = 26, /**< for c_malformed_message, the throughput speed  enum is too small */
                                  d_throughput_facility_too_large = 27, /**< for c_malformed_message, the throughput speed  enum is too large */
                                  d_invalid_cause = 28, /**< for c_malformed_message, a diagnostic message's cause is invalid */
                                  d_invalid_diagnostic = 29, /**< for c_malformed_message, a diagnostic message's diagnostic is invalid */
                                  d_invalid_q_too_large = 30, /**< for c_malformed_message, the data's Q is too large */
                                  d_invalid_pr_too_small = 31, /**< for c_malformed_message, PR too small */
                                  d_invalid_pr_too_large = 32, /**< for c_malformed_message, PS too small */
                                  d_data_too_short = 33, /**< for c_malformed_message, data packet is empty */
                                  d_data_too_long = 34, /**< for c_malformed_message, data packet is too big for the channel */
                                  d_invalid_directionality_facility = 35, /**< for c_malformed_message, the directionality is invalid */

                                  // For c_invalid_facility_request
                                  d_invalid_packet_facility_negotiation = 36, /**< for c_invalid_facility_request, the packet size negotiation broke the rules */
                                  d_invalid_window_facility_negotiation = 37, /**< for c_invalid_facility_request, the window size negotiation broke the rules */
                                  d_invalid_throughput_facility_negotiation = 38, /**< for c_invalid_facility_request, the throughput speed negotiation broke the rules */
                                  d_invalid_directionality_negotiation = 39, /**< for c_invalid_facility_request, the directionality negotiation broke the rules */

                                  // For c_invalid_forwarding_request
                                  d_callee_forwarding_not_allowed = 40, /**< for c_invalid_forwarding_request, forwarding not allowed */
                                  d_caller_forwarding_not_allowed = 41, /**< for c_invalid_forwarding_request, forwarding not allowed */

                                  // For c_access_barred
                                  d_output_barred = 42, /**< for c_access_barred, sender not allowed to initiate calls */
                                  d_input_barred = 43, /**< for c_access_barred, receiver not allowed to receive calls */

                                  // for c_address_in_use
                                  d_reserved_address = 44, /**< for c_address_in_use, the "operator" address is reserved */
                                  d_address_in_use = 45, /**< for c_address_in_use, the requested name is already in use */

                                  // for c_unknown_address
                                  d_unknown_worker_address = 46, /**< for c_unknown_address, the callee is not found */
                                  d_unknown_reserved_address = 47, /**< for c_unknown_address,  the "operator" is not online */

                                  // for c_network_congestion
                                  d_no_connections_available = 48, /**< for c_network_congestion, too many workers */
                                  d_no_channels_available = 49, /**< for c_network_congestion, too many channels */

                                  // for procedure errors
                                  d_invalid_message_for_state_ready = 50, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_x_call_request = 51, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_y_call_request = 52, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_data_transfer = 53, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_call_collision = 54, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_x_clear_request = 55, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_y_clear_request = 56, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_x_reset_request = 57, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_invalid_message_for_state_y_reset_request = 58, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_ps_out_of_order = 59, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_ps_not_in_window = 60, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_pr_invalid_window_update = 61, /**< for c_local_procedure_error or c_remote_procedure_error */
                                  d_data_too_long_for_packet_facility = 62, /**< for c_local_procedure_error or c_remote_procedure_error */

                                  // for quota_exceeded
                                  d_data_rate_exceeded = 63, /**< for c_quota_exceeded, too many bytes-per-second */
                                  d_message_rate_exceeded = 64, /**< for c_quota_exceeded, too many messages-per-second  */

                                  d_last = d_message_rate_exceeded
} diag_t;

/**
 * @brief Returns diagnostic matching most recent c_zmq_sendmsg_error
 *
 * It converts the most recent errno to a diagnostic enum.
 *
 * @return A diagnostic
 */
diag_t errno2diag();

/**
 * @brief Returns a string name of a given diagnostic.
 *
 * @param D a diagnostic enum value
 * @return A constant string. Caller must not free.
 */
const char *diag_name(diag_t D);


#endif
