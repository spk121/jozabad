#include "../libjoza/joza_lib.h"
#include "../libjoza/joza_msg.h"

static void initialize(int verbose, const char *preface, zctx_t **ctx, void **sock, char *broker,
                       char *calling_address, direction_t dir);

static int timeout1;

void
catch_alarm (int sig)
{
    if (timeout1) {
        printf("timeout\n");
        fflush(stdout);
        exit(1);
    }
}

int main(int argc, char** argv) {
    int verbose = 1;
    char *broker = "tcp://localhost:5555";
    char *calling_address1 = "100-0001";
    char *calling_address2 = "100-0002";
    direction_t dir = direction_bidirectional;
    zctx_t *ctx1 = NULL;
    zctx_t *ctx2 = NULL;
    void *sock1 = NULL;
    void *sock2 = NULL;
    throughput_t thru = t_2048kbps;
    packet_t packet = p_4_Kbytes;
    uint16_t window = WINDOW_MAX;
    int ret;

    initialize (verbose, "peer X", &ctx1, &sock1, broker, calling_address1, dir);
    initialize (verbose, "peer Y", &ctx2, &sock2, broker, calling_address2, dir);
    if (verbose)
        printf("peer X: requesting a virtual call\n");
    ret = joza_msg_send_call_request(sock1, calling_address1, calling_address2,
                                     packet, window, thru, zframe_new (0,0));

    if (verbose)
        printf("peer Y: waiting for a call request message\n");
    // Broker must respond within timeout
    joza_msg_t *response = joza_msg_recv(sock2);
    if (joza_msg_id(response) != JOZA_MSG_CALL_REQUEST) {
        if (verbose) {
            printf("peer Y: did not receive call request from X\n");
            joza_msg_dump(response);
        }
        exit (1);
    }   
    
    return (EXIT_SUCCESS);
}

static void initialize(int verbose, const char *preface, zctx_t **ctx, void **sock, char *broker,
                       char *calling_address, direction_t dir)
{
    int ret;

    if (verbose)
        printf("%s: connecting to socket %s\n", preface, broker);

    *ctx = zctx_new ();
    if (*ctx == NULL) {
        if (verbose)
            printf("%s: zctx_new() returned NULL\n", preface);
        exit(1);
    }

    *sock = zsocket_new(*ctx, ZMQ_DEALER);
    if (*sock == NULL) {
        if (verbose)
            printf("%s: zsocket_new(%p, %d) returned NULL\n", preface, *ctx, ZMQ_DEALER);
        exit(1);
    }

    ret = zsocket_connect(*sock, broker);
    if (ret != 0) {
        if (verbose)
            printf("%s: zsocket_connect(%p, %p) returned %d\n", preface, *sock, broker, ret);
        exit(1);
    }

    if (verbose)
        printf("%s: connecting to broker as %s, (%s)\n",
               preface,
               calling_address,
               direction_name(dir));
    ret = joza_msg_send_connect(*sock, calling_address, dir);
    if (ret != 0) {
        if (verbose)
            printf("%s: joza_msg_send_connect (%p, %s, %s) returned %d\n", preface,
                   *sock,
                   calling_address,
                   direction_name(dir),
                   ret);
        exit(1);
    }

    // Broker must respond within timeout
    timeout1 = true;
    signal (SIGALRM, catch_alarm);
    alarm(10);

    if (verbose)
        printf("%s: waiting for connect indication\n", preface);
    joza_msg_t *response = joza_msg_recv(*sock);
    timeout1 = false;
    if (joza_msg_id(response) != JOZA_MSG_CONNECT_INDICATION) {
        if (verbose) {
            printf("%s: did not receive connect indication\n", preface);
            joza_msg_dump(response);
        }
        exit (1);
    }   
}
