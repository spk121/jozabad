/*
    diag.c - diagnostics for diagnostic messages

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

#include <errno.h>
#include <zmq.h>
#include "diag.h"

static const char diag_names[65][45] = {
    /* 0 */ "D_UNSPECIFIED",
    "D_WORKER_ORIGINATED",
    "D_NUMBER_BUSY",
    "D_CALL_COLLISION",
    "D_ZMQ_ERROR",
    "D_ZMQ_EAGAIN",
    "D_ZMQ_ENOTSUP",
    "D_ZMQ_EFSM",
    "D_ZMQ_ETERM",
    "D_ZMQ_ENOTSOCK",
    /* 10 */ "D_ZMQ_EINTR",
    "D_ZMQ_EFAULT",
    "D_INVALID_Q",
    "D_PR_TOO_LARGE",
    "D_PS_TOO_LARGE",
    "D_INVALID_DATA_FRAME",
    "D_CALLING_ADDRESS_TOO_SHORT",
    "D_CALLING_ADDRESS_TOO_LONG",
    "D_CALLING_ADDRESS_FORMAT_INVALID",
    "D_CALLED_ADDRESS_TOO_SHORT",
    /* 20 */ "D_CALLED_ADDRESS_TOO_LONG",
    "D_CALLED_ADDRESS_FORMAT_INVALID",
    "D_PACKET_FACILITY_TOO_SMALL",
    "D_PACKET_FACILITY_TOO_LARGE",
    "D_WINDOW_FACILITY_TOO_SMALL",
    "D_WINDOW_FACILITY_TOO_LARGE",
    "D_THROUGHPUT_FACILITY_TOO_SMALL",
    "D_THROUGHPUT_FACILITY_TOO_LARGE",
    "D_INVALID_CAUSE",
    "D_INVALID_DIAGNOSTIC",
    /* 30 */ "D_INVALID_Q_TOO_LARGE",
    "D_INVALID_PR_TOO_SMALL",
    "D_INVALID_PR_TOO_LARGE",
    "D_DATA_TOO_SHORT",
    "D_DATA_TOO_LONG",
    "D_INVALID_DIRECTIONAL_FACILITY",
    "D_INVALID_PACKET_FACILITY_NEGOTIATION",
    "D_INVALID_WINDOW_FACILITY_NEGOTIATION",
    "D_INVALID_DIRECTIONALITY_NEGOTIATION",
    /* 40 */ "D_CALLEE_FORWARDING_NOT_ALLOWED",
    "D_CALLER_FORWARDING_NOT_ALLOWED",
    "D_OUTPUT_BARRED",
    "D_INPUT_BARRED",
    "D_RESERVED_ADDRESS",
    "D_ADDRESS_IN_USE",
    "D_UNKNOWN_WORKER_ADDRESS",
    "D_UNKNOWN_RESERVED_ADDRESS",
    "D_NO_CONNECTIONS_AVAILABLE",
    "D_NO_CHANNELS_AVAILABLE",
    /* 50 */ "D_INVALID_MESSAGE_FOR_STATE_READY",
    "D_INVALID_MESSAGE_FOR_STATE_X_CALL_REQUEST",
    "D_INVALID_MESSAGE_FOR_STATE_Y_CALL_REQUEST",
    "D_INVALID_MESSAGE_FOR_STATE_DATA_TRANSFER",
    "D_INVAlID_MESSAGE_FOR_STATE_CALL_COLLISION",
    "D_INVALID_MESSAGE_FOR_STATE_X_CLEAR_REQUEST",
    "D_INVALID_MESSAGE_FOR_STATE_Y_CLEAR_REQUEST",
    "D_INVALID_MESSAGE_FOR_STATE_X_RESET_REQUEST",
    "D_INVALID_MESSAGE_FOR_STATE_Y_RESET_REQUEST",
    "D_PS_OUT_OF_ORDER",
    /* 60 */ "D_PS_NOT_IN_WINDOW",
    "D_PR_INVALID_WINDOW_UPDATE",
    "D_DATA_TOO_LONG_FOR_PACKET_FACILITY",
    "D_DATA_RATE_EXCEEDED",
    "D_MESSAGE_RATE_EXCEEDED"
};

diag_t errno2diag()
{
    if (errno == EAGAIN)
        return d_zmq_eagain;
    else if (errno == ENOTSUP)
        return d_zmq_enotsup;
    else if (errno == EFSM)
        return d_zmq_efsm;
    else if (errno == ETERM)
        return d_zmq_eterm;
    else if (errno == ENOTSOCK)
        return d_zmq_enotsock;
    else if (errno == EINTR)
        return d_zmq_eintr;
    else if (errno == EFAULT)
        return d_zmq_efault;
    else
        return d_zmq_error;
}

const char *diag_name(diag_t D)
{
    return &(diag_names[D][0]);
}
