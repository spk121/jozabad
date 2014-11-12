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

    initialize (verbose, "peer X", &ctx1, &sock1, broker, calling_address1, "Alpha", dir);
    initialize (verbose, "peer Y", &ctx2, &sock2, broker, calling_address2, "Bravo", dir);
    call_setup (verbose, sock1, sock2, calling_address1, calling_address2, packet, window, thru);

    for (int i = 0; i < 2; i ++) {
        joza_msg_t *response;
        char *prefix = i ? calling_address2 : calling_address1;
        char *other_prefix = i ? calling_address1 : calling_address2;
        void *sock = i ? sock2 : sock1;
        void *other_sock = i ? sock1 : sock2;

        // Send a X RR message with PR = 0
        if (verbose)
            printf("%s: sending RR to broker with valid PR = 0\n", prefix);
        ret = joza_msg_send_rr(sock, 0);
        if (ret != 0) {
            if (verbose)
                printf("%s: joza_msg_send_rr (PR = 0) returned %d\n", prefix, ret);
            exit(1);
        }

        // Y should receive this RR message
        if (verbose)
            printf("%s: waiting for RR\n", other_prefix);
        response = joza_msg_recv(other_sock);
        if (joza_msg_id(response) != JOZA_MSG_RR || joza_msg_pr(response) != 0) {
            if (verbose) {
                printf("%s: did not receive RR PR = 0\n", i ? calling_address1 : calling_address2);
                joza_msg_dump(response);
            }
            exit (1);
        }

        // X shouldn't be able to set PR higher than the next PS it will
        // receive.  The first PS it will receive is zero, so it can't set PR
        // to higher than zero initially.

        // Send a X RR message with PR = 1
        if (verbose)
            printf("%s: sending RR to broker with invalid PR = 1\n", prefix);
        ret = joza_msg_send_rr(sock, 1);
        if (ret != 0) {
            if (verbose)
                printf("%s: joza_msg_send_rr (PR = 1) returned %d\n", prefix, ret);
            exit(1);
        }

        // This should receive an error
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
        } else if (joza_msg_diagnostic(response) != d_pr_invalid_window_update) {
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

