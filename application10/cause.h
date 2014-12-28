/*
    cause.h - general categories of errors

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

/**
 * @file cause.h
 * @brief List of error categories for diagnostic messages
 *
 * Diagnostic messages have a general 'cause' and a more specific
 * 'diagnostic'.
 */

#ifndef JOZA_CAUSE_H
#define JOZA_CAUSE_H

// These are general error categories for DIAGNOSTIC, RESET, and CLEAR
// messages.
/**
 * @brief The list of error categories.
 *
 * In DIAGNOSTIC, RESET, and CLEAR messages, an error category is supplied
 * to indicate the reason for the message.
 */

// In a client-originated CLEAR REQUEST, the client may set the cause to
// either 0 == c_client_originated, or 128 to 255, where only the top bit
// is significant from the point of view of the server.

// In a server-originated CLEAR REQUEST, the clearing causes are as
// follows,

typedef enum {
  c_clear__number_busy = 1,
  c_clear__invalid_facility_request = 3,
  c_clear__network_congestion = 5,
  c_clear__out_of_order = 9,
  c_clear__access_barred = 11,
  c_clear__not_obtainable = 13,
  c_clear__remote_procedure_error = 17,
  c_clear__local_procedure_error = 19,
  c_clear__rpoa_out_of_order = 21,
  c_clear__reverse_charging_not_accepted = 25,
  c_clear__incompatible_destination = 33,
  c_clear__fast_select_not_accepted = 41,
  c_clear__ship_absent = 57,
} server_clear_clearing_cause_t;

// In a client-originated RESET REQUEST, the clearing cause may be 0
// or may be 128 to 255, of which only the top bit is checked by the
// server.  The other 7 bits may be used by the client.

// In a server-originated RESET REQUEST, the clearing cause is one
// of the following

typedef enum {
  c_reset__out_of_order = 1,
  c_reset__remote_procedure_error = 3,
  c_reset__local_procedure_error = 5,
  c_reset__network_contestion = 7,
  c_reset__remote_client_operational = 9,
  c_reset__network_operational = 15,
  c_reset__incompatible_destination = 17,
  c_reset__network_out_of_order = 29,
} server_reset_clearing_cause_t;

// In a client-originated RESTART REQUEST, the clearing cause may be 0
// or may be 128 to 255, of which only the top bit is checked by the
// server.  The other 7 bits may be used by the client.


// In a server-originated RESTART REQUEST, the clearing cause is one
// of the following.
typedef enum {
  // These are the cause codes for RESTART packets
  c_restart__local_procedure_error = 1,
  c_restart__network_congestion = 3,
  c_restart__network_operational = 7,
  
  c_restart__registration_confirmed = 127,
  c_restart__client_restarting = 128,
} server_restart_clearing_cause_t;

#if 0
typedef enum {
  c_ok,
  c_unspecified,
  c_client_originated, /**<  CLEAR and RESET messages from workers must have this cause */
  c_number_busy,       /**< a CALL_REQUEST when the other worker is busy. */
  c_invalid_forwarding_request, /**< a forwarding request that can't be processed */
  c_access_barred,     /**< a CALL_REQUEST to an output-only or from an input-only channel. */
  c_local_procedure_error, /**< worker sends inapproprate message for the channel state */
  c_network_congestion, /**< no available worker slots or channels */
  
  c_remote_procedure_error, /**< other worker sent inappropriate message for the channel state */
    c_call_collision,    /**< a CALL_REQUEST where the other worker has just connected to this worker */
    c_zmq_sendmsg_err,   /**< the underlying zmq_sendmsg returned non-zero */
    c_malformed_message, /**< for any message that has out of range or malformed fields */
    c_invalid_facility_request, /**< a facility negotiation where a worker is trying to cheat */
    c_address_in_use,    /**< a CONNECTION request where the desired address is in use */
    c_unknown_address,    /**< a CALL_REQUEST where the called address is unknown */
    c_broker_shutdown,
    c_quota_exceeded,    /**< when a broker shuts down a worker or channel for too much data */
} cause_t;
#endif

typedef guint8 cause_t;

#define CAUSE_MAX (255)

/**
 * @brief Returns a string reprentation of a cause.
 *
 * @param C an cause
 * @return A null-terminated string. Do not free.
 */
const char *cause_name(cause_t c);

gboolean cause_validate(cause_t c);

#endif
