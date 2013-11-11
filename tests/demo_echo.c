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
#include "mylimits.h"

#include "initialize.h"

int main(int argc, char** argv)
{
    int verbose = 1;
    char *broker = "tcp://localhost:5555";
    char *calling_address = "ECHO";
    iodir_t dir = io_outgoing_calls_barred;
    zctx_t *ctx = NULL;
    void *sock = NULL;
    int enabled = 0, connected = 0;
    unsigned xpr = 0, xps = 0, ypr = 0, yps = 0, window = 0;
    joza_msg_t *msg;

    initialize (verbose, "echo", &ctx, &sock, broker, calling_address, dir);
    while (1) {
        // Act on connection request

        if (verbose)
            printf("%s: waiting for a call request message\n", calling_address);
        msg = joza_msg_recv(sock);
        if (joza_msg_id(msg) != JOZA_MSG_CALL_REQUEST)
            continue;
        joza_msg_dump(msg);
        window = joza_msg_window(msg);
        // joza_msg_send_call_accepted(&msg, sock);
        joza_msg_send_call_accepted (sock,
                                     joza_msg_calling_address(msg),
                                     joza_msg_called_address(msg),
                                     joza_msg_packet(msg),
                                     joza_msg_window(msg),
                                     joza_msg_throughput(msg),
                                     zframe_new(0,0));

        connected = 1;
        xpr = 0;
        xps = 0;
        ypr = 0;
        yps = 0;

        while (connected) {
            msg = joza_msg_recv(sock);
            joza_msg_dump(msg);
            switch (joza_msg_id(msg)) {
            case JOZA_MSG_DATA:
                // If PR allows it, echo the message back to the caller.
                ypr = joza_msg_pr(msg);
                yps = joza_msg_ps(msg);
                xpr = yps;      /* Always allow new data */
                if (enabled && xps < ypr + window) {
                    // Echo Y's msg back to him
                    joza_msg_send_data(sock,
                                       joza_msg_q(msg),
                                       xpr,
                                       xps,
                                       joza_msg_data(msg));
                    xps ++;
                    if (xps > WINDOW_MAX)
                        xps = 0;
                }
                else {
                    joza_msg_send_rr(sock, xpr);
                }
                break;
            case JOZA_MSG_RR:
                // update flow control
                // enable echoing
                enabled = 1;
                ypr = joza_msg_pr(msg);
                break;
            case JOZA_MSG_RNR:
                // update flow control
                // disable echoing
                enabled = 0;
                ypr = joza_msg_pr(msg);
                break;
            case JOZA_MSG_RESET_REQUEST:
                // accept reset, resetting flow control params
                joza_msg_send_reset_confirmation(sock);
                xpr = 0;
                xps = 0;
                ypr = 0;
                yps = 0;
                break;
            case JOZA_MSG_CLEAR_REQUEST:
                // accept clear, shutting down
                joza_msg_send_clear_confirmation(sock);
                connected = 0;
                break;
            }
            joza_msg_destroy(&msg);
        }
    }
    return (0);
}

