/*
    cause.h - general categories of errors

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

#ifndef JOZA_CAUSE_H
#define JOZA_CAUSE_H

// These are general error categories for DIAGNOSTIC, RESET, and CLEAR
// messages.

typedef enum {
    c_unspecified,

    // CLEAR and RESET messages from workers must have this cause
    c_worker_originated,

    // As a response to a CALL_REQUEST where the other worker is busy.
    c_number_busy,

    // As a response to a CALL_REQUEST where the other worker has just
    // connected to this worker
    c_call_collision,

    // For any diagnostic sent as a result of a zmq_sendmsg
    // returning non-zero.
    c_zmq_sendmsg_err,

    // For any message from a worker that has either
    // - address strings that don't meet the address format
    // - integer types that are out of range
    // - data frames that are too big or too small
    c_malformed_message,

    // As a response to any facility request negotiation where
    // a worker is trying to cheat.
    c_invalid_facility_request,

    // As a response to any fowarding request that can't be processed.
    c_invalid_forwarding_request,

    // As a response to a CALL_REQUEST to an output-only
    // channel, or from an input-only channel.
    c_access_barred,

    // As a response to a CONNECTION request where the desired
    // address is in used.
    c_address_in_use,

    // As a response to a CALL_REQUEST where the called address is
    // unknown
    c_unknown_address,

    // As a response when
    // - a CONNECTION request when all worker slots in use
    // - CALL_REQUEST when all channels are in use
    c_network_congestion,

    // As a response when
    // - a worker sends a message that is inappropriate for the current state
    c_local_procedure_error,
    c_remote_procedure_error,

    // As a response when
    // - the broker is
    c_broker_shutdown,

    // When the broker shuts down a worker for sending data too quickly
    c_quota_exceeded,

    c_last = c_quota_exceeded
} cause_t;

const char *cause_name(cause_t c);

#endif
