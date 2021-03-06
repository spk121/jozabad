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
        void *sock = i ? sock2 : sock1;
        void *other_sock = i ? sock1 : sock2;

        for (int j = SEQ_MIN; j <= SEQ_MAX; j ++) {

            // Send a X data message
            if (verbose)
                printf("%s: sending data to broker with valid PS %d\n", prefix, j);
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

            // send Y RR
            ret = joza_msg_send_rr(other_sock, j);
            if (ret != 0) {
                if (verbose)
                    printf("%s: joza_msg_send_rr (...) returned %d\n", prefix, ret);
                exit(1);
            }

            // Receive the Y RR on X
            response = joza_msg_recv(i ? sock2 : sock1);
            if (joza_msg_id(response) != JOZA_MSG_RR) {
                if (verbose) {
                    printf("%s: did not receive RR PR = %d\n", i ? calling_address2 : calling_address1,
                           joza_msg_pr(response));
                }
                exit (1);
            }
        }

        // Send a data message
        if (verbose)
            printf("%s: sending data to broker with valid PS 0\n", prefix);
        ret = joza_msg_send_data(sock, 0, 0, 0, zframe_new(data, 4));
        if (ret != 0) {
            if (verbose)
                printf("%s: joza_msg_send_data (PS=%d) returned %d\n", prefix, SEQ_MAX + 1, ret);
            exit(1);
        }

        // Receive the X data message on Y
        response = joza_msg_recv(i ? sock1 : sock2);
        if (joza_msg_id(response) != JOZA_MSG_DATA) {
            if (verbose) {
                printf("%s: did not receive data PS = 0\n", i ? calling_address1 : calling_address2);
            }
            exit (1);
        }
    }


    if (verbose)
        printf("SUCCESS\n");
    return (EXIT_SUCCESS);
}

