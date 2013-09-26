#include "joza_msg.h"
#include "diag.h"
#include "cause.h"
#include "iodir.h"
#include "packet.h"
#include "tput.h"
#include "seq.h"

#include "initialize.h"
#include "call_setup.h"

int main(int argc, char** argv)
{
    int verbose = getenv("JOZA_VERBOSE_TEST");
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
        void *other_sock = i ? sock1 : sock2;

        for (int j = 0; j < window; j ++) {
            // Send a X data message
            if (verbose)
                printf("%s: sending data to broker with PS %d in window\n", prefix, j);
            ret = joza_msg_send_data(sock, 0, 0, j, zframe_new(data, 4));
            if (ret != 0) {
                if (verbose)
                    printf("%s: joza_msg_send_data (PS=%d) returned %d\n", prefix, j, ret);
                exit(1);
            }

            // Receive the X data message on Y
            response = joza_msg_recv(i ? sock1 : sock2);
            if (joza_msg_id(response) != JOZA_MSG_DATA) {
                if (verbose) {
                    printf("%s: did not receive data PS = %d\n", i ? calling_address1 : calling_address2, j);
                }
                exit (1);
            }
        }

        if (verbose)
            printf("%s: sending data to broker with PS %d *not* in window\n", prefix, window);
        ret = joza_msg_send_data(sock, 0, 0, window, zframe_new(data, 4));
        if (ret != 0) {
            if (verbose)
                printf("%s: joza_msg_send_data (PS=3) returned %d\n", prefix, ret);
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
                printf("%s: did not received correct cause (%s/%s)\n",
                       prefix,
                       cause_name(joza_msg_cause(response)),
                       diag_name(joza_msg_diagnostic(response)));
            exit(1);
        } else if (joza_msg_diagnostic(response) != d_ps_not_in_window) {
            if (verbose)
                printf("%s: did not received correct diagnostic (%s/%s)\n",
                       prefix,
                       cause_name(joza_msg_cause(response)),
                       diag_name(joza_msg_diagnostic(response)));
            exit(1);
        }
    }

    if (verbose)
        printf("SUCCESS\n");
    return (EXIT_SUCCESS);
}

