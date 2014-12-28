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

#include <glib.h>

/**
 * @file diag.h
 * @brief List of specific errors for diagnostic messages
 *
 * Diagnostic messages have a general 'cause' and a more specific
 * 'diagnostic'.
 */


typedef enum {
  d_packet_too_short,
  d_packet_on_unassigned_logical_channel,
  
  d_ok,

  // for c_unspecified
  d_unspecified, /**< for c_unspecified */

  // for c_client_originated
  d_client_originated, /**< for c_client_originated */

  // for c_number_busy
  d_no_logical_channel_available,
  d_number_busy, /**< for c_number_busy */

  // for c_call_collision
  d_call_collision, /**< for c_call_collision */
  
  // For c_system_error.  There are the 'errno' codes
  // returned when sendmsg fails
  d_system_error,   /**< for low-level errors, a generic error */
  d_eagain,  /**< for c_zmq_sendmsg_error, no available messages in non-blocking mode */
  d_enotsup, /**< for c_zmq_sendmsg_error, not supported by this socket type */
  d_efsm,    /**< for c_zmq_sendmsg_error, socket not in correct state */
  d_eterm,   /**< for c_zmq_sendmsg_error, socket was terminated */
  d_enotsock, /**< for c_zmq_sendmsg_error, socket is invalid */
  d_eintr,  /**< for c_zmq_sendmsg_error, operation was interrupted by signal */
  d_efault, /**< for c_zmq_sendmsg_error, message is invalid */

  // For c_malformed_message
  d_unidentifiable_packet,
  d_invalid_signature,
  d_unknown_version,
  d_unknown_message_type,
  d_invalid_lcn,
  d_invalid_q,     /**< for c_malformed_message */
  d_invalid_pr,  /**< for c_malformed_message, PR too large */
  d_invalid_ps,  /**< for c_malformed_message, PS too large */
  d_invalid_calling_address, /**< for c_malformed_message, address has zero length */
  d_invalid_called_address,
  d_invalid_hostname,
  d_invalid_packet_facility,
  d_invalid_throughput_facility,
  d_invalid_window_facility,
  d_invalid_directionality_facility,
  d_invalid_cause,
  d_invalid_diagnostic,
  d_data_too_short, /**< for c_malformed_message, data packet is empty */
  d_data_too_long, /**< for c_malformed_message, data packet is too big for the channel */

  // For c_invalid_facility_request
  d_invalid_packet_facility_negotiation, /**< for c_invalid_facility_request, the packet size negotiation broke the rules */
  d_invalid_window_facility_negotiation, /**< for c_invalid_facility_request, the window size negotiation broke the rules */
  d_invalid_throughput_facility_negotiation, /**< for c_invalid_facility_request, the throughput speed negotiation broke the rules */
  d_invalid_directionality_facility_negotiation, /**< for c_invalid_facility_request, the directionality negotiation broke the rules */

  // For c_invalid_forwarding_request
  d_callee_forwarding_not_allowed, /**< for c_invalid_forwarding_request, forwarding not allowed */
  d_caller_forwarding_not_allowed, /**< for c_invalid_forwarding_request, forwarding not allowed */

  // For c_access_barred
  d_output_barred, /**< for c_access_barred, sender not allowed to initiate calls */
  d_input_barred, 
  d_incoming_calls_barred, /**< for c_access_barred, receiver not allowed to receive calls */
  d_outgoing_calls_barred,

  // for c_address_in_use
  d_reserved_address, /**< for c_address_in_use, the "operator" address is reserved */
  d_address_in_use, /**< for c_address_in_use, the requested name is already in use */

  // for c_unknown_address
  d_unknown_client_address, /**< for c_unknown_address, the callee is not found */
  d_unknown_reserved_address, /**< for c_unknown_address,  the "operator" is not online */

  // for c_network_congestion
  d_no_connections_available, /**< for c_network_congestion, too many workers */
  d_no_channels_available, /**< for c_network_congestion, too many channels */

  // for procedure errors
  d_invalid_message_for_state_unitialized,
  d_invalid_message_for_state_client_restart_request,
  d_invalid_message_for_state_server_restart_request,
  d_invalid_message_for_state_initialized,
  d_invalid_message_for_state_ready, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_x_call_request, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_client_waiting,
  d_invalid_message_for_state_server_waiting,
  d_invalid_message_for_state_flow_control_ready,
  d_invalid_message_for_state_client_reset_request,  
  d_invalid_message_for_state_y_call_request, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_data_transfer, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_call_collision, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_x_clear_request, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_y_clear_request, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_x_reset_request, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_invalid_message_for_state_y_reset_request, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_ps_out_of_order, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_ps_not_in_window, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_pr_invalid_window_update, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_data_too_long_for_packet_facility, /**< for c_local_procedure_error or c_remote_procedure_error */
  d_improper_cause_code_from_client,
  
  // for quota_exceeded
  d_data_rate_exceeded, /**< for c_quota_exceeded, too many bytes-per-second */
  d_message_rate_exceeded, /**< for c_quota_exceeded, too many messages-per-second  */

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

gboolean diag_validate(diag_t D);

#endif
