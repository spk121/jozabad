/*
    msg.h - helper functions for ZeroMQ activities

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
 * @file msg.h
 * @author Mike Gran
 * @brief helper functions for joza_msg functions
 *
 * These functions add convenience or diagnostic information to
 * calls of the joza_msg primitives.
 */

#ifndef JOZA_MSG_H
#define JOZA_MSG_H

#include <czmq.h>
#include "mylimits.h"
#include "packet.h"
#include "seq.h"
#include "tput.h"
#include "mylimits.h"
#include "cause.h"
#include "diag.h"
#include "joza_msg.h"

/**
 * @brief Get a 32-bit hash integer from a ZeroMQ address frame
 *
 * Deep diving into the ZeroMQ source, it appears that the first
 * few bytes of an address frame are a unique integer ID.  This
 * extracts that integer ID.
 *
 * @param Z  A CZMQ frame that contains a ZeroMQ address
 * @return A unique 32-bit integer key for this address
 */
G_GNUC_INTERNAL
wkey_t msg_addr2key (const zframe_t *z);

/**
 * @brief Do basic sanity check of message
 *
 * This function does several basic range checks on the parameters
 * of a joza_msg.
 *
 * @param msg   A joza_msg
 * @return Returns d_unspecified if the message isn't invalid, or another d_* constant otherwise.
 */
G_GNUC_INTERNAL
diag_t prevalidate_message (joza_msg_t *msg);

void call_request(void *sock, zframe_t *call_addr, zframe_t *return_addr, char *xname, char *yname,
                  packet_t pkt, seq_t window, tput_t tput, zframe_t *data);
void diagnostic(void *sock, const zframe_t *A, const char *name, cause_t C, diag_t D);

/**
 * @brief send a directory hashtable to a worker.
 *
 * Typically in response to a DIRECTORY_REQUEST message from a worker,
 * this command sends a DIRECTORY hash table message to a worker.
 *
 * @param sock  A ZeroMQ socket
 * @param A     A CZMQ frame that contains a ZeroMQ address
 * @param D     A CZMQ hash table that contains worker name / status text pairs
 */
G_GNUC_INTERNAL
void directory_request(void *sock, const zframe_t *A, zhash_t *D);

#define RESET_REQUEST(A,C,D) \
    joza_msg_send_addr_reset_request(g_poll_sock, (A), (C), (D))
#define CLEAR_REQUEST(A,C,D) \
    joza_msg_send_addr_clear_request(g_poll_sock, (A), (C), (D))
#define DIAGNOSTIC(A,C,D) \
    diagnostic(A,C,D)

#define DISCONNECT_INDICATION(A) \
    joza_msg_send_addr_connect_indication(g_poll_sock, (A))
#define CONNECT_INDICATION(A) \
    joza_msg_send_addr_connect_indication(g_poll_sock, (A))
#define CALL_REQUEST call_request

#endif
