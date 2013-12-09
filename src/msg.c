/*
    msg.c - helper functions for message passing

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

#include <czmq.h>
#include "msg.h"
#include "poll.h"
#include "joza_msg.h"
#include "cause.h"
#include "diag.h"
#include "worker.h"

// Deep diving into the ZeroMQ source says that for ZeroMQ 3.x,
// bytes 1 to 5 of the address would work as a unique ID for a
// given connection.
wkey_t msg_addr2key (const zframe_t *z)
{
    wkey_t x[1];

    memcpy(x, (char *) zframe_data((zframe_t *) z) + 1, sizeof(wkey_t));

    return x[0];
}

void diagnostic(void *sock, const zframe_t *A, const char *name, cause_t C, diag_t D)
{
    if (name)
        g_message("sending DIAGNOSTIC to %s, %s, %s", name, cause_name(C), diag_name(D));
    else
        g_message("sending DIAGNOSTIC %s, %s", cause_name(C), diag_name(D));
    joza_msg_send_addr_diagnostic(sock, A, C, D);
}

void call_request(void *sock, zframe_t *call_addr, zframe_t *return_addr, char *xname, char *yname,
                  packet_t pkt, seq_t window, tput_t tput, zframe_t *data)
{
    int ret;
    ret = joza_msg_send_addr_call_request(sock, call_addr, xname, yname, pkt, window, tput, data);
    if (ret == -1)
        diagnostic(sock, return_addr, xname, c_zmq_sendmsg_err, errno2diag());
}

void directory_request(void *sock, const zframe_t *A, const zhash_t *D)
{
    joza_msg_send_addr_directory (sock, A, D);
}

diag_t prevalidate_message (const joza_msg_t *msg)
{
    diag_t ret = d_unspecified;

    int id = joza_msg_const_id (msg);
    if (id == JOZA_MSG_DATA) {
        byte q = joza_msg_const_q(msg);
    }
    if (id == JOZA_MSG_DATA || id == JOZA_MSG_RR || id == JOZA_MSG_RNR) {
        uint16_t pr = joza_msg_const_pr(msg);
        if (pr > SEQ_MAX)
            ret = d_pr_too_large;
    }
    if (id == JOZA_MSG_DATA) {
        uint16_t ps = joza_msg_const_ps(msg);
        if (ps > SEQ_MAX)
            ret = d_ps_too_large;
    }
    if (id == JOZA_MSG_DATA) {
        zframe_t *data = joza_msg_const_data(msg);
        size_t data_len = zframe_size(data);
        if (data_len == 0)
            ret = d_data_too_short;
        if (data_len > packet_bytes(p_last))
            ret = d_data_too_long;
    }
    if (id == JOZA_MSG_CALL_REQUEST || id == JOZA_MSG_CALL_ACCEPTED) {
        zframe_t *data = joza_msg_const_data(msg);
        size_t data_len = zframe_size(data);
        if (data_len > packet_bytes(p_default))
            ret = d_data_too_long;
    }
    if (id == JOZA_MSG_CALL_REQUEST || id == JOZA_MSG_CALL_ACCEPTED || id == JOZA_MSG_CONNECT) {
        char *calling_address = joza_msg_const_calling_address(msg);

        if (strnlen_s(calling_address, NAME_LEN + 1) == 0)
            ret = d_calling_address_too_short;
        else if (strnlen_s(calling_address, NAME_LEN + 1) > NAME_LEN)
            ret = d_calling_address_too_long;
        else if (!safeascii(calling_address, NAME_LEN))
            ret = d_calling_address_format_invalid;
    }
    if (id == JOZA_MSG_CALL_REQUEST || id == JOZA_MSG_CALL_ACCEPTED) {
        char *called_address = joza_msg_const_called_address(msg);
        byte packet = joza_msg_const_packet(msg);
        uint16_t window = joza_msg_const_window(msg);
        byte throughput = joza_msg_const_throughput(msg);

        if (strnlen_s(called_address, NAME_LEN + 1) == 0)
            ret = d_called_address_too_short;
        else if (strnlen_s(called_address, NAME_LEN + 1) > NAME_LEN)
            ret = d_called_address_too_long;
        else if (!safeascii(called_address, NAME_LEN))
            ret = d_called_address_format_invalid;

        else if (packet_rngchk(packet) < 0)
            ret = d_packet_facility_too_small;
        else if (packet_rngchk(packet) > 0)
            ret = d_packet_facility_too_large;

        else if (seq_rngchk(window) < 0)
            ret = d_window_facility_too_small;
        else if (seq_rngchk(window) > 0)
            ret = d_window_facility_too_large;

        else if (tput_rngchk(throughput) < 0)
            ret = d_throughput_facility_too_small;
        else if (tput_rngchk(throughput) > 0)
            ret = d_throughput_facility_too_large;

    }
    if (id == JOZA_MSG_CLEAR_REQUEST || id == JOZA_MSG_RESET_REQUEST || id == JOZA_MSG_DIAGNOSTIC) {
        byte cause = joza_msg_const_cause(msg);
        byte diagnostic = joza_msg_const_diagnostic(msg);

        if (cause > CAUSE_MAX)
            ret = d_invalid_cause;
        else if (diagnostic > d_last)
            ret = d_invalid_diagnostic;
    }
    if (id == JOZA_MSG_CONNECT) {
        byte directionality = joza_msg_const_directionality(msg);
        if (iodir_validate(directionality) == 0)
            ret = d_invalid_directionality_facility;
    }
    if (id == JOZA_MSG_DIRECTORY) {
        zhash_t *workers = joza_msg_workers(msg);
    }
    return ret;
}
