/*
 * File:   w_echo.c
 * Author: mike
 *
 * Created on August 30, 2013, 6:36 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <czmq.h>
#include "../include/msg.h"
#include "../include/direction.h"
#include "../include/throughput.h"
#include "../include/packet.h"

const char broker[] = "tcp://localhost:5555";
char *service_name = "echo";
static zctx_t *ctx;
static void *sock;

/*
 * This is a worker that echoes the messages it has been send
 */
int main(int argc, char** argv) {
    int verbose = 1;
    direction_t dir = direction_bidirectional;
    throughput_t thru = t_4800bps;
    packet_t packet;
    uint16_t window;
    msg_t *msg_in;
    char *sname;

    ctx = zctx_new ();

    zclock_log ("connecting to broker socket at %s...", broker);
    sock = zsocket_new (ctx, ZMQ_DEALER);
    zmq_connect (sock, broker);

    zclock_log ("connecting to broker as '%s', %s, %s...", broker, name(dir), name(thru));
    msg_send_connect(sock, service_name, dir, thru);

    // Wait for a connection indication
    while (true) {
        zclock_log ("waiting for a connection indication");
        msg_in = msg_recv(sock);
        assert(msg_in);
        msg_dump(msg_in);

        if (msg_id(msg_in) != MSG_CONNECT_INDICATION) {
            zclock_log ("received %s", msg_const_command(msg_in));
            msg_destroy(&msg_in);
        }
        else {
            dir = (direction_t) msg_directionality(msg_in);
            thru = (throughput_t)msg_throughput(msg_in);
            zclock_log ("received %s, %s, %s", msg_const_command(msg_in), name(dir), name(thru));
            msg_destroy(&msg_in);
            break;
        }
    }

    while (true) {
        zclock_log ("waiting for a call request");
        msg_in = msg_recv(sock);
        assert(msg_in);
        msg_dump(msg_in);

        if (msg_id(msg_in) != MSG_CALL_REQUEST) {
            zclock_log ("received %s", msg_const_command(msg_in));
            msg_destroy(&msg_in);
            continue;
        }
        else {
            const char *addr = msg_const_called_address(msg_in);
            dir = (direction_t) msg_directionality(msg_in);
            packet = (packet_t) msg_packet(msg_in);
            window = msg_window(msg_in);
            thru = (throughput_t)msg_throughput(msg_in);
            zclock_log ("received %s, %s, %s, %s, %d, %s", msg_const_command(msg_in), addr, name(dir), name(packet), window, name(thru));
            msg_destroy(&msg_in);

            zclock_log ("sending call accepted with %s, %s, %d, %s", name(dir), name(packet), window, name(thru));
            msg_send_call_accepted(sock, packet, window, thru, zframe_new(0, 0));

            break;
        }

    }
    sleep(5);
#if 0
        // Wait for a call request on my service
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_CALL_REQUEST);
        assert(streq(parch_msg_service(request), "echo"));


        // Respond that the call is accepted
        parch_msg_send_call_accepted(parch_node_client(session), 0, parch_msg_packet(request), parch_msg_window(request), parch_msg_throughput(request));

        parch_msg_destroy(&request);

        // Wait for a data packet
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_DATA);
        parch_msg_destroy(&request);

        // Respond with a data packet and an RR packet
        parch_msg_send_data(parch_node_client(session), 0, zframe_new("DATA BAR", 8));
        parch_msg_send_rr(parch_node_client(session),0);

        // Wait for receive a clear channel request
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_CLEAR_REQUEST);
        parch_msg_destroy(&request);

        // Respond with a clear confirmation
        parch_msg_send_clear_confirmation(parch_node_client(session), 0, 0);

        // Disconnect
#endif

    return (EXIT_SUCCESS);
}

