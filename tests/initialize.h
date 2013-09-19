#ifndef JOZA_TESTS_INITIALIZE_H
#define JOZA_TESTS_INITIALIZE_H

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
    //timeout1 = true;
    // signal (SIGALRM, catch_alarm);
    //alarm(10);

    if (verbose)
        printf("%s: waiting for connect indication\n", preface);
    joza_msg_t *response = joza_msg_recv(*sock);
    //timeout1 = false;
    if (joza_msg_id(response) != JOZA_MSG_CONNECT_INDICATION) {
        if (verbose) {
            printf("%s: did not receive connect indication\n", preface);
            joza_msg_dump(response);
        }
        exit (1);
    }   
}


#endif
