/*
    cause.c - causes for diagnostic messages

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

#include <glib.h>
#include "cause.h"

#define CAUSE_NAME_MAX_LEN (30)

static const char cause_names[CAUSE_MAX + 1][CAUSE_NAME_MAX_LEN] = {
    "C_UNSPECIFIED",
    "C_WORKER_ORIGINATED",
    "C_NUMBER_BUSY",
    "C_CALL_COLLISION",
    "C_ZMQ_SENDMSG_ERR",
    "C_MALFORMED_MSG",
    "C_INVALID_FACILITY_REQUEST",
    "C_INVALID_FORWARDING_REQUEST",
    "C_ACCESS_BARRED",
    "C_ADDRESS_IN_USE",
    "C_UNKNOWN_ADDRESS",
    "C_NETWORK_CONGESTION",
    "C_LOCAL_PROCEDURE_ERROR",
    "C_REMOTE_PROCEDURE_ERROR",
    "C_BROKER_SHUTDOWN",
    "C_QUOTA_EXCEEDED"
};

const char *cause_name(cause_t c)
{
    g_assert_cmpint(c, <=, CAUSE_MAX);

    return &(cause_names[c][0]);
}

