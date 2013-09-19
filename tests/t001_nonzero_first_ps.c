#include "joza_msg.h"
#include "diag.h"
#include "cause.h"
#include "iodir.h"
#include "packet.h"
#include "tput.h"

static void initialize(int verbose, const char *preface, zctx_t **ctx, void **sock, char *broker,
                       char *calling_address, iodir_t dir);
static void call_setup (int verbose, void *sock1, void *sock2, char *name1, char *name2,
                 packet_t packet, uint16_t window, tput_t thru);

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

    // OK, finally we're ready for the test.  The first data message sent should
    // have a PS of Zero.  If it doesn't it should receive an error.
    if (verbose)
        printf("peer X: sending data to broker with PS = 2\n");
    ret = joza_msg_send_data(sock1, 0, 0, 2, zframe_new(data, 4));
    if (ret != 0) {
        if (verbose)
            printf("peer X: joza_msg_send_data (...) returned %d\n", ret);
        exit(1);
    }

    if (verbose)
        printf("peer X: waiting for diagnostic\n");
    joza_msg_t *response = joza_msg_recv(sock1);
    if (joza_msg_id(response) != JOZA_MSG_DIAGNOSTIC) {
        if (verbose) {
            printf("peer X: did not receive diagnostic\n");
            joza_msg_dump(response);
        }
        exit (1);
    }   
    else if(joza_msg_cause(response) != c_local_procedure_error
            || joza_msg_diagnostic(response) != d_ps_out_of_order) {
        if (verbose)
            printf("peer X: did not received correct diagnostic (%d/%d)", joza_msg_cause(response),
                   joza_msg_diagnostic(response));
        exit(1);
    }

    if (verbose)
        printf("SUCCESS\n");
    return (EXIT_SUCCESS);
}

static void initialize(int verbose, const char *preface, zctx_t **ctx, void **sock, char *broker,
                       char *calling_address, iodir_t dir)
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
               iodir_name(dir));
    ret = joza_msg_send_connect(*sock, calling_address, dir);
    if (ret != 0) {
        if (verbose)
            printf("%s: joza_msg_send_connect (%p, %s, %s) returned %d\n", preface,
                   *sock,
                   calling_address,
                   iodir_name(dir),
                   ret);
        exit(1);
    }

    // Broker must respond within timeout
    timeout1 = true;
    signal (SIGALRM, catch_alarm);
    //alarm(10);

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


void call_setup (int verbose, void *sock1, void *sock2, char *name1, char *name2,
                 packet_t packet, uint16_t window, tput_t thru)
{
    int ret;
       
    if (verbose)
        printf("peer X: requesting a virtual call\n");
    ret = joza_msg_send_call_request(sock1, name1, name2,
                                     packet, window, thru, zframe_new (0,0));

    if (verbose)
        printf("peer Y: waiting for a call request message\n");
    joza_msg_t *response = joza_msg_recv(sock2);
    if (joza_msg_id(response) != JOZA_MSG_CALL_REQUEST) {
        if (verbose) {
            printf("peer Y: did not receive call request from X\n");
            joza_msg_dump(response);
        }
        exit (1);
    }   

    if (verbose)
        printf("peer Y: sending call accepted message\n");
    joza_msg_send_call_accepted(sock2, name1, name2,
                                packet, window, thru, zframe_new(0,0));

    joza_msg_destroy(&response);
    
    if (verbose)
        printf("peer X: waiting for a call accepted message\n");
    response = joza_msg_recv(sock1);
    if (joza_msg_id(response) != JOZA_MSG_CALL_ACCEPTED) {
        if (verbose) {
            printf("peer X: did not receive call request from Y\n");
            joza_msg_dump(response);
        }
        exit (1);
    } 
}

