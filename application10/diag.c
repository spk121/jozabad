/*
    diag.c - diagnostics for diagnostic messages

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

#include <errno.h>
#include <zmq.h>
#include "diag.h"

#if 0
static const char diag2_names[128][45] = {
  [d_packet_too_short] = "PACKET TOO SHORT",
  [d_packet_too_long] = "PACKET TOO LONG"
};
#endif

static const char diag_names[d_last + 1][80] = {
  [d_ok] = "D_OK",

  // For c_unspecified
  [d_unspecified] = "D_UNSPECIFIED",

  // For c_worker_originated
  [d_client_originated] = "D_CLIENT_ORIGINATED",

  // For c_number_busy
  [d_no_logical_channel_available] = "D_NO_LOGICAL_CHANNEL_AVAILABLE",
  [d_number_busy] = "D_NUMBER_BUSY",

  // For c_call_collision
  [d_call_collision] = "D_CALL_COLLISION",

  // For c_system_error
  [d_system_error] = "D_SYSTEM_ERROR",
  [d_eagain] = "D_EAGAIN",
  [d_enotsup] = "D_ENOTSUP",
  [d_efsm] = "D_EFSM",
  [d_eterm] = "D_ETERM",
  [d_enotsock] = "D_ENOTSOCK",
  [d_eintr] = "D_EINTR",
  [d_efault] = "D_EFAULT",

  // For c_malformed_message
  [d_unidentifiable_packet] = "D_UNIDENTIFABLE_PACKET",
  [d_invalid_signature] = "D_INVALID_SIGNATURE",
  [d_invalid_envelope] = "D_INVALID_ENVELOPE",
  [d_unknown_version] = "D_UNKNOWN_VERSION",
  [d_unknown_message_type] = "D_UNKNOWN_MESSAGE_TYPE",
  [d_invalid_lcn] = "D_INVALID_LCN",
  [d_invalid_q] = "D_INVALID_Q",
  [d_invalid_pr] = "D_INVALID_PR",
  [d_invalid_ps] = "D_INVALID_PS",
  [d_invalid_calling_address] = "D_INVALID_CALLING_ADDRESS",
  [d_invalid_called_address] = "D_INVALID_CALLED_ADDRESS",
  [d_invalid_hostname] = "D_INVALID_HOSTNAME",
  [d_invalid_packet_facility] = "D_INVALID_PACKET_FACILITY",
  [d_invalid_window_facility] = "D_INVALID_WINDOW_FACILITY",
  [d_invalid_throughput_facility] = "D_INVALID_THROUGHPUT_FACILITY",
  [d_invalid_directionality_facility] = "D_INVALID_DIRECTIONALITY_FACILITY",
  [d_invalid_cause] = "D_INVALID_CAUSE",
  [d_invalid_diagnostic] = "D_INVALID_DIAGNOSTIC",
  [d_data_too_short] = "D_DATA_TOO_SHORT",
  [d_data_too_long] = "D_DATA_TOO_LONG",

  // For c_invalid_facility_request
  [d_invalid_packet_facility_negotiation] = "D_INVALID_PACKET_FACILITY_NEGOTIATION",
  [d_invalid_window_facility_negotiation] = "D_INVALID_WINDOW_FACILITY_NEGOTIATION",
  [d_invalid_throughput_facility_negotiation] = "D_INVALID_THROUGHPUT_FACILITY_NEGOTIATION",
  [d_invalid_directionality_facility_negotiation] = "D_INVALID_DIRECTIONALITY_FACILITY_NEGOTIATION",

  // For c_invalid_forwarding_request
  [d_callee_forwarding_not_allowed] = "D_CALLEE_FORWARDING_NOT_ALLOWED",
  [d_caller_forwarding_not_allowed] = "D_CALLER_FORWARDING_NOT_ALLOWED",

  // For c_access_barred
  [d_output_barred] = "D_OUTPUT_BARRED",
  [d_input_barred] = "D_INPUT_BARRED",
  [d_incoming_calls_barred] = "D_INCOMING_CALLS_BARRED",
  [d_outgoing_calls_barred] = "D_OUTGOING_CALLS_BARRED",

  // for c_address_in_use
  [d_reserved_address] = "D_RESERVED_ADDRESS",
  [d_address_in_use] = "D_ADDRESS_IN_USE",

  // for c_unknown_address
  [d_unknown_client_address] = "D_UNKNOWN_CLIENT_ADDRESS",
  [d_unknown_reserved_address] = "D_UNKNOWN_RESERVED_ADDRESS",

  // For c_network_congestion
  [d_no_connections_available] = "D_NO_CONNECTIONS_AVAILABLE",
  [d_no_channels_available] = "D_NO_CHANNELS_AVAILABLE",

  // For procedure errors
  [d_invalid_message_for_state_unitialized] = "D_INVALID_MESSAGE_FOR_STATE_UNITIALIZED",
  [d_invalid_message_for_state_client_restart_request] = "D_INVALID_MESSAGE_FOR_STATE_CLIENT_RESTART_REQUEST", 
  [d_invalid_message_for_state_server_restart_request] = "D_INVALID_MESSAGE_FOR_STATE_SERVER_RESTART_REQUEST", 
  [d_invalid_message_for_state_initialized] = "D_INVALID_MESSAGE_FOR_STATE_INITIALIZED",
  [d_invalid_message_for_state_ready] = "D_INVALID_MESSAGE_FOR_STATE_READY",
  [d_invalid_message_for_state_x_call_request] = "D_INVALID_MESSAGE_FOR_STATE_X_CALL_REQUEST",
  [d_invalid_message_for_state_y_call_request] = "D_INVALID_MESSAGE_FOR_STATE_Y_CALL_REQUEST",
  [d_invalid_message_for_state_data_transfer] = "D_INVALID_MESSAGE_FOR_STATE_DATA_TRANSFER",
  [d_invalid_message_for_state_call_collision] = "D_INVAlID_MESSAGE_FOR_STATE_CALL_COLLISION",
  //[d_invalid_message_for_state_x_clear_request] = "D_INVALID_MESSAGE_FOR_STATE_X_CLEAR_REQUEST",
  //[d_invalid_message_for_state_y_clear_request] = "D_INVALID_MESSAGE_FOR_STATE_Y_CLEAR_REQUEST",
  //[d_invalid_message_for_state_x_reset_request] = "D_INVALID_MESSAGE_FOR_STATE_X_RESET_REQUEST",
  //[d_invalid_message_for_state_y_reset_request] = "D_INVALID_MESSAGE_FOR_STATE_Y_RESET_REQUEST",
  [d_ps_out_of_order] = "D_PS_OUT_OF_ORDER",
  [d_ps_not_in_window] = "D_PS_NOT_IN_WINDOW",
  [d_pr_invalid_window_update] = "D_PR_INVALID_WINDOW_UPDATE",
  [d_data_too_long_for_packet_facility] = "D_DATA_TOO_LONG_FOR_PACKET_FACILITY",
  [d_improper_cause_code_from_client] = "D_IMPROPER_CAUSE_CODE_FROM_CLIENT",

  // for c_quota_exceeded
  [d_data_rate_exceeded] = "D_DATA_RATE_EXCEEDED",
  [d_message_rate_exceeded] = "D_MESSAGE_RATE_EXCEEDED",
};

diag_t errno2diag()
{
    if (errno == EAGAIN)
        return d_eagain;
    else if (errno == ENOTSUP)
        return d_enotsup;
    else if (errno == EFSM)
        return d_efsm;
    else if (errno == ETERM)
        return d_eterm;
    else if (errno == ENOTSOCK)
        return d_enotsock;
    else if (errno == EINTR)
        return d_eintr;
    else if (errno == EFAULT)
        return d_efault;
    else
        return d_system_error;
}

const char *diag_name(diag_t D)
{
    return diag_names[D];
}

gboolean diag_validate(diag_t D)
{
  if (D < 0 || D > d_last)
    return FALSE;
  return TRUE;
}
