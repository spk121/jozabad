/*
    demo_echo.h - a bot that echos the messages it receives

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
#include "joza_msg.h"
#include "diag.h"
#include "cause.h"
#include "iodir.h"
#include "packet.h"
#include "tput.h"

#include "initialize.h"
#include "call_setup.h"

int main(int argc, char** argv)
{
    int verbose = 1;
    char *broker = "tcp://localhost:5555";
    char *calling_address = "ECHO";
    iodir_t dir = io_outgoing_calls_barred;
    zctx_t *ctx = NULL;
    void *sock = NULL;
    tput_t thru;
    packet_t packet;
    uint16_t window;
    int ret;

    initialize (verbose, "echo", &ctx, &sock, broker, calling_address, dir);
    while (1) {
        // Act on connection request

        switch (id) {
        case DATA:
            // If PR allows it, echo the message back to the caller.
            // Update flow control params.
            break;
        case RR:
            // update flow control
            // enable echoing
            break;
        case RNR:
            // update flow control
            // disable echoing
            break;
        case RESET_REQUEST:
            // accept reset, resetting flow control params
            break;
        case CLEAR_REQUEST:
            // accept clear, shutting down
            break;
        }
    }
    return (0);
}

