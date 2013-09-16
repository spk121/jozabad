#include <czmq.h>

#include "../libjoza/joza_msg.h"

//#include "connections.h"
#include "lib.h"
#include "log.h"
#include "poll.h"
#include "worker.h"
#include "channel.h"

void *g_poll_sock;

static zctx_t *ctx;
static zloop_t *loop;
static zmq_pollitem_t poll_input = {NULL, 0, ZMQ_POLLIN, 0};

static int s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *arg);
static int s_process_(joza_msg_t *msg);

zctx_t *
zctx_new_or_die (void)
{
    zctx_t *c = zctx_new();
    if (c == NULL) {
        zclock_log("failed to create a ZeroMQ context");
        exit(1);
    }
    return c;
}

void *
zsocket_new_or_die(zctx_t *ctx, int type)
{
    void *sock = zsocket_new(ctx, type);
    if (sock == NULL) {
        zclock_log("failed to create a new ZeroMQ socket");
        exit(1);
    }
    return sock;
}

zloop_t *
zloop_new_or_die(void)
{
    zloop_t *L = zloop_new();
    if (L == NULL) {
        zclock_log("failed to create a new ZeroMQ main loop");
        exit (1);
    }
    return L;
}


void poll_init(bool_t verbose, const char *endpoint)
{
    //  Initialize broker state
    ctx = zctx_new_or_die();
    g_poll_sock = zsocket_new_or_die(ctx, ZMQ_ROUTER);
    NOTE("binding ROUTER socket to %s", endpoint);
    zsocket_bind(g_poll_sock, endpoint);
    loop = zloop_new_or_die();
    zloop_set_verbose(loop, verbose);
    poll_input.socket = g_poll_sock;
    int rc = zloop_poller(loop, &poll_input, s_recv_, NULL);
    assert(rc != -1);
}

// This is the main callback that gets called whenever a
// message is received from the ZeroMQ loop.
static int
s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    joza_msg_t *msg = NULL;
    int ret = 0;

    if (item->revents & ZMQ_POLLIN) {
        msg = joza_msg_recv(g_poll_sock);
        if (msg != NULL) {
            // Process the valid message
            ret = s_process_(msg);
        }
    }

    return ret;
}

// This is entry point for message processing.  Every message
// being processed start here.
static int
s_process_(joza_msg_t *msg)
{
    uint32_t key;
    bool_index_t bi_worker;
    bool_t more = FALSE;
    role_t role = READY;

    key = addr2hash(joza_msg_address(msg));
do_more:

    bi_worker = get_worker(key);
    if (bi_worker.flag == TRUE) {
        role = w_role[bi_worker.index];
    }

    // If this worker is connected and part of a virtual call, the
    // call's state machine processes the message.
    if (role == X_CALLER || role == Y_CALLEE) {
        channel_dispatch_by_lcn(msg, w_lcn[bi_worker.index], role);
    }

    // If this worker is connected, but, not part of a call, the
    // connection handler handles the message.
    else if (bi_worker.flag == TRUE) {
        more = worker_dispatch_by_idx(msg, bi_worker.index);
        if (more)
            goto do_more;
    }

    // If this worker is heretofore unknown, we handle it here.  The
    // only message we handle from unconnected workers are connection
    // requests.
    else {
        const char *cmdname = joza_msg_const_command(msg);

        if (joza_msg_id(msg) == JOZA_MSG_CONNECT) {
            INFO("handling %s from unconnected worker", cmdname);
            add_worker(joza_msg_const_address(msg),
                       joza_msg_const_calling_address(msg),
                       (iodir_t) joza_msg_const_directionality(msg));
        } else {
            INFO("ignoring %s from unconnected worker", cmdname);
        }
    }
    return 0;
}


void poll_start(void)
{
    // Takes over control of the thread.
    NOTE("starting main loop");
    zloop_start(loop);
}

