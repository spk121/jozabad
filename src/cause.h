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
typedef enum {
    c_unspecified,
    c_worker_originated, /**<  CLEAR and RESET messages from workers must have this cause */
    c_number_busy,       /**< a CALL_REQUEST when the other worker is busy. */
    c_call_collision,    /**< a CALL_REQUEST where the other worker has just connected to this worker */
    c_zmq_sendmsg_err,   /**< the underlying zmq_sendmsg returned non-zero */
    c_malformed_message, /**< for any message that has out of range or malformed fields */
    c_invalid_facility_request, /**< a facility negotiation where a worker is trying to cheat */
    c_invalid_forwarding_request, /**< a forwarding request that can't be processed */
    c_access_barred,     /**< a CALL_REQUEST to an output-only or from an input-only channel. */
    c_address_in_use,    /**< a CONNECTION request where the desired address is in use */
    c_unknown_address,    /**< a CALL_REQUEST where the called address is unknown */
    c_network_congestion, /**< no available worker slots or channels */
    c_local_procedure_error, /**< worker sends inapproprate message for the channel state */
    c_remote_procedure_error, /**< other worker sent inappropriate message for the channel state */
    c_broker_shutdown,
    c_quota_exceeded,    /**< when a broker shuts down a worker or channel for too much data */
} cause_t;

#define CAUSE_MAX (c_quota_exceeded)

/**
 * @brief Returns a string reprentation of a cause.
 *
 * @param C an cause
 * @return A null-terminated string. Do not free.
 */
const char *cause_name(cause_t c);

#endif
