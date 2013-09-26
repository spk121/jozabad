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
    char *calling_address1 = "ADAM";
    char *calling_address2 = "EVE";
    iodir_t dir = io_bidirectional;
    zctx_t *ctx1 = NULL;
    zctx_t *ctx2 = NULL;
    void *sock1 = NULL;
    void *sock2 = NULL;
    tput_t thru = t_2048kbps;
    packet_t packet = p_4_Kbytes;
    uint16_t window = 2;
    int ret;
    char *data = "DATA";

    initialize (verbose, "peer X", &ctx1, &sock1, broker, calling_address1, dir);

    initialize (verbose, "peer Y", &ctx2, &sock2, broker, calling_address2, dir);

    call_setup (verbose, sock1, sock2, calling_address1, calling_address2, packet, window, thru);

    for (int i = 0; i < 2; i ++) {
        joza_msg_t *response;
        char *prefix = i ? calling_address2 : calling_address1;
        void *sock = i ? sock2 : sock1;
        // Valid first data message.  It should not receive an error
        if (verbose)
            printf("%s: sending data to broker with good initial PS = 0\n", prefix);
        ret = joza_msg_send_data(sock, 0, 0, 0, zframe_new(data, 4));
        if (ret != 0) {
            if (verbose)
                printf("%s: joza_msg_send_data (...) returned %d\n", prefix, ret);
            exit(1);
        }

        // Receive the data message on the other size
        response = joza_msg_recv(i ? sock1 : sock2);
        if (joza_msg_id(response) != JOZA_MSG_DATA) {
            if (verbose) {
                printf("%s: did not receive data\n", i ? calling_address1 : calling_address2);
            }
            exit (1);
        } else {
            if (verbose) {
                printf("%s: received data\n", i ? calling_address1 : calling_address2);
            }
        }

        if (verbose)
            printf("%s: sending data to broker with discontinuous PS = 2\n", prefix);
        ret = joza_msg_send_data(sock, 0, 0, 2, zframe_new(data, 4));
        if (ret != 0) {
            if (verbose)
                printf("%s: joza_msg_send_data (...) returned %d\n", prefix, ret);
            exit(1);
        }

        if (verbose)
            printf("%s: waiting for diagnostic\n", prefix);
        response = joza_msg_recv(sock);
        if (joza_msg_id(response) != JOZA_MSG_DIAGNOSTIC) {
            if (verbose) {
                printf("%s: did not receive diagnostic\n", prefix);
                joza_msg_dump(response);
            }
            exit (1);
        } else if(joza_msg_cause(response) != c_local_procedure_error) {
            if (verbose)
                printf("%s: did not received correct cause (%d/%d)\n",
                       prefix,
                       joza_msg_cause(response),
                       joza_msg_diagnostic(response));
            exit(1);
        } else if (joza_msg_diagnostic(response) != d_ps_out_of_order) {
            if (verbose)
                printf("%s: did not received correct diagnostic (%d/%d)\n",
                       prefix,
                       joza_msg_cause(response),
                       joza_msg_diagnostic(response));
            exit(1);
        }
    }


    if (verbose)
        printf("SUCCESS\n");
    return (EXIT_SUCCESS);
}

