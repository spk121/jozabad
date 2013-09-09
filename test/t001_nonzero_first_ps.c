#include "../libjoza/joza_lib.h"
#include "../libjoza/joza_msg.h"

static void initialize(int verbose, zctx_t *ctx, void *sock, char *broker,
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
    //throughput_t thru = t_2048kbps;
    //packet_t packet = p_4_Kbytes;
    //uint16_t window = WINDOW_MAX;

    initialize (verbose, ctx1, sock1, broker, calling_address1, dir);
    initialize (verbose, ctx2, sock2, broker, calling_address2, dir);
    return (EXIT_SUCCESS);
}

static void initialize(int verbose, zctx_t *ctx, void *sock, char *broker,
                       char *calling_address, direction_t dir)
{
    int ret;

    if (verbose)
        printf("connecting to socket %s\n", broker);

    ctx = zctx_new ();
    if (ctx == NULL) {
        if (verbose)
            printf("zctx_new() returned NULL\n");
        exit(1);
    }

    sock = zsocket_new(ctx, ZMQ_DEALER);
    if (sock == NULL) {
        if (verbose)
            printf("zsocket_new(%p, %d) returned NULL\n", ctx, ZMQ_DEALER);
        exit(1);
    }

    ret = zsocket_connect(sock, broker);
    if (ret != 0) {
        if (verbose)
            printf("zsocket_connect(%p, %p) returned %d\n", sock, broker, ret);
        exit(1);
    }

    if (verbose)
        printf("connecting to broker as %s, (%s)\n",
               calling_address,
               direction_name(dir));
    ret = joza_msg_send_connect(sock, calling_address, dir);
    if (ret != 0) {
        if (verbose)
            printf("joza_msg_send_connect (%p, %s, %s) returned %d\n",
                   sock,
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
        printf("waiting for connect indication\n");
    joza_msg_t *response = joza_msg_recv(sock);
    timeout1 = false;
    if (joza_msg_id(response) != JOZA_MSG_CONNECT_INDICATION) {
        if (verbose) {
            printf("did not receive connect indication\n");
            joza_msg_dump(response);
        }
        exit (1);
    }
    
}
