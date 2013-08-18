/*  =========================================================================
    parch_node.c - node API

    -------------------------------------------------------------------------
    Copyright (c) 2013 - Michael L. Gran - http://lonelycactus.com
    Copyright other contributors as noted in the AUTHORS file.

    This file is part of petulant-archer, A ZeroMQ-based networking
    library implementing the Switched Virtual Circuit pattern.

    http://github.com/spk121/petulant-archer

    This is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANT-
    ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
    Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not see http://www.gnu.org/licenses.
    =========================================================================
*/

#include "../include/parch.h"
// #include "../include/parch_common.h"
//#include "../include/parch_node.h"

//  Structure of our class

struct _parch_node_t {
    zctx_t *ctx;                // Our context
    char *broker;
    void *client;               // Socket to broker
    char *service;              //
    int verbose;                // print activity to stdout
};

//  ----------------------------------------------------------------
//  Connect or reconnect to broker

void s_parch_node_connect_to_broker (parch_node_t *self)
{
    if (self->client)
        zsocket_destroy (self->ctx, self->client);
    self->client = zsocket_new (self->ctx, ZMQ_DEALER);
    int ret = zmq_connect (self->client, self->broker);
    if (ret == 0) {
        if (self->verbose)
            zclock_log ("I: connecting to broker at %s...", self->broker);
    }
    else if (ret == -1) {
        zclock_log ("I: failed to connect to broker at %s, %s", self->broker, strerror(errno));
    }
}

//  ----------------------------------------------------------------
//  Constructor

parch_node_t *
parch_node_new (char *broker, char *service, int verbose)
{
    assert (broker);

    parch_node_t *self = (parch_node_t *) zmalloc (sizeof (parch_node_t));
    self->ctx = zctx_new ();
    self->broker = strdup (broker);
    self->service = strdup (service);
    self->verbose = verbose;
    s_parch_node_connect_to_broker (self);
    return self;
}

//  ----------------------------------------------------------------
//  Destructor

void
parch_node_destroy (parch_node_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        parch_node_t *self = *self_p;
        zctx_destroy (&self->ctx);
        free (self->broker);
        free (self->service);
        free (self);
        *self_p = NULL;
    }
}

//  ---------------------------------------------------------------------
//  Set node socket option

int
parch_node_setsockopt (parch_node_t *self, int option, const void *optval, size_t optvallen)
{
    assert (self);
    assert (self->client);
    return zmq_setsockopt (self->client, option, optval, optvallen);
}


//  ---------------------------------------------------------------------
//  Get node socket option

int
parch_node_getsockopt (parch_node_t *self, 	int option, void *optval, size_t *optvallen)
{
    assert (self);
    assert (self->client);
    return zmq_getsockopt (self->client, option, optval, optvallen);
}

const char *
parch_node_service (parch_node_t *self)
{
    assert (self);
    assert (self->service);
    return self->service;
}

void *
parch_node_client (parch_node_t *self)
{
    assert (self);
    assert (self->client);
    return self->client;
}


//  ----------------------------------------------------------------
//  Here is the send method. It sends a request to the broker.
//  It takes ownership of the request message, and destroys it when sent.
#if 0
void
parch_node_send (parch_node_t *self, zmsg_t **request_p)
{
    assert (self);
    assert (request_p);
    zmsg_t *request = *request_p;

    //  Prefix request with protocol frames
    //  Frame 1: empty frame (delimiter)
    //  Frame 2: "PARCHxy" (seven bytes, PARCH x.y)
    //  Frame 3: Address (sending to broker)
    zmsg_pushstr (request, PARCH_NODE_TO_BROKER);
    zmsg_pushstr (request, PARCH_HEADER);
    zmsg_pushstr (request, "");
    if (self->verbose) {
        zclock_log ("I: send request:");
        zmsg_dump (request);
    }
    zmsg_send (request_p, self->client);
}
#endif
int
parch_node_poll (parch_node_t *self, int timeout_ms)
{
    zmq_pollitem_t items[] = { {self->client, 0, ZMQ_POLLIN, 0} };

    /* this will block for 10msec, ZMQ_POLL_MSEC is for compatibility for v2.2 */
    int rc = zmq_poll(items, 1, timeout_ms * ZMQ_POLL_MSEC );
    if (rc == -1)
        return 0;

    return items [0].revents & ZMQ_POLLIN;
}

//  ----------------------------------------------------------------
//  Receive report from the broker.
//  The caller is responsible for destroying the received message.
//  If service is not NULL, it is filled in with a pointer
//  to service string. It is caller's responsibility to free it.
#if 0
zmsg_t *
parch_node_recv (parch_node_t *self, char **service_p)
{
    assert (self);

    zmsg_t *msg = zmsg_recv (self->client);
    if (msg == NULL)
        //  Interrupt
        return NULL;

    if (self->verbose) {
        zclock_log ("I: received reply:");
        zmsg_dump (msg);
    }

    //  Message format:
    //  Frame 1: empty frame (delimiter)
    //  Frame 2: "PARCHxy" (sevel bytes, PARCH x.y)
    //  Frame 3: Address (sending to node)
    //  Frame 4..n: Application frames

    //  We would handle malformed replies better in real code
    assert (zmsg_size (msg) >= 5);

    zframe_t *empty = zmsg_pop (msg);
    assert (zframe_streq (empty, ""));
    zframe_destroy (&empty);

    zframe_t *header = zmsg_pop (msg);
    assert (zframe_streq (header, PARCH_HEADER));
    zframe_destroy (&header);

    zframe_t *address = zmsg_pop (msg);
    assert (zframe_streq (address, PARCH_BROKER_TO_NODE));
    zframe_destroy (&address);

    zframe_t *service = zmsg_pop (msg);
    if (service_p)
        *service_p = zframe_strdup (service);
    zframe_destroy (&service);

    return msg;     //  Success
}
#endif
