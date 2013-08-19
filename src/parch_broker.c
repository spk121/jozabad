#include <stdio.h>
#include <stdlib.h>

//  The broker class defines a single broker instance
#include <czmq.h>
#if CZMQ_VERSION < 10402
#error "petulant-archer needs CZMQ/1.4.2 or later"
#endif

#include "../include/parch.h"
#include "parch_msg.h"

#define THROUGHPUT_CLASS_MIN 3
#define THROUGHPUT_CLASS_MAX 45

struct _broker_t {
    zctx_t *ctx; //  Our context
    void *socket; //  Socket for external messages to nodes
    int verbose; //  Print activity to stdout
    char *endpoint; //  Broker binds to this endpoint
    zloop_t *loop; //  This is the main loop
    zmq_pollitem_t *poll_input;
    zhash_t *nodes; //  Hash of known nodes
    zhash_t *services; //  Hash of known services
    zlist_t *waiting; //  List of nodes trying to call other nodes
};

typedef struct {
    broker_t *broker; //  Broker instance
    char *name; //  Service name
    zlist_t *idle; //  Idle nodes
    zlist_t *busy; //  Connected nodes
    size_t workers; //  How many workers we have
} service_t;

struct _node_t {
    // state machine
    broker_t *broker; // loopback pointer to the broker that services this node
    service_t *service; // loopback pointer to the service
    zframe_t *node_address; // pointer to the ZeroMQ address of this node
    char *service_name;
    node_t *peer; // If a call is active, this is the callee.
    parch_state_engine_t *engine;
};


static parch_msg_t *
s_broker_msg_recv(broker_t *self);
static void
s_broker_destroy(broker_t **self_p);
static broker_t *
s_broker_new(int verbose, char *endpoint);

//  The service class defines a single service instance

static void
s_service_destroy(void *argument);
static void
s_node_set_service(node_t *node);

void *
parch_broker_get_socket(broker_t *self) {
    return self->socket;
}

int
parch_broker_get_verbose(broker_t *self) {
    return self->verbose;
}

node_t *
parch_node_get_peer(node_t * const self) {
    assert(self);
    return self->peer;
}

zframe_t *
parch_node_get_node_address(node_t const * const self) {
    return self->node_address;
}

parch_state_engine_t *
parch_node_get_state_engine(node_t *self) {
    assert(self);
    return self->engine;
}

void
parch_node_set_service_name(node_t *self, char *sname) {
    assert(self);
    assert(sname);
    if (self->service_name)
        free(self->service_name);
    self->service_name = strdup(sname);
}

char *
parch_node_get_service_name(node_t *self) {
    return self->service_name;
}

void
parch_node_update_service_name_with_broker(node_t *node) {
    assert(node);
    assert(node->service_name);

    if (streq(node->service_name, node->service->name))
        return;

    if (node->service) {
        zlist_remove(node->service->idle, node);
        zlist_remove(node->service->busy, node);
        node->service->workers = zlist_size(node->service->idle) + zlist_size(node->service->busy);
    }
    s_node_set_service(node);
    return;
}

//  Here is the implementation of the methods that work on a service.
//  Lazy constructor that locates a service by name, or creates a new
//  service if there is no service already with that name.

static service_t *
s_broker_require_service(broker_t *self, const char const *service_name) {
    assert(service_name);
    char *name = strdup(service_name);

    service_t *service =
            (service_t *) zhash_lookup(self->services, name);
    if (service == NULL) {
        service = (service_t *) zmalloc(sizeof (service_t));
        service->broker = self;
        service->name = name;
        service->idle = zlist_new();
        service->busy = zlist_new();
        zhash_insert(self->services, name, service);
        zhash_freefn(self->services, name, s_service_destroy);
        if (self->verbose)
            zclock_log("I: broker -- added service '%s'", name);
    } else
        free(name);

    return service;
}

//  Service destructor is called automatically whenever the service is
//  removed from broker->services.

static void
s_service_destroy(void *argument) {
    service_t *service = (service_t *) argument;
    zlist_destroy(&service->idle);
    zlist_destroy(&service->busy);
    free(service->name);
    free(service);
}

//  The join method finds an idle worker of a given name.  It

static node_t *
s_service_get_node(service_t *self) {
    assert(self);

    // s_broker_purge (self->broker);
    if (zlist_size(self->idle) == 0)
        return (node_t *) 0;

    node_t *worker = (node_t *) zlist_pop(self->idle);
    zlist_remove(self->idle, worker);
    zlist_append(self->busy, worker);
    return worker;
}

static void
s_node_set_service(node_t *node) {
    //  Attach node to service and mark as idle
    node->service = s_broker_require_service(node->broker, node->service_name);
    zlist_append(node->service->idle, node);
    node->service->workers++;
    if (node->broker->verbose) {
        char * const node_addr = zframe_strhex(node->node_address);
        zclock_log("I: [%s] node -- attached to service '%s'", node_addr,
                node->service_name);
        free(node_addr);
    }
}


#if 0
//  The dispatch method sends request to the worker.

static void
s_service_dispatch(service_t *self) {
    assert(self);

    s_broker_purge(self->broker);
    if (zlist_size(self->waiting) == 0)
        return;

    while (zlist_size(self->requests) > 0) {
        worker_t *worker = (worker_t*) zlist_pop(self->waiting);
        zlist_remove(self->waiting, worker);
        zmsg_t *msg = (zmsg_t*) zlist_pop(self->requests);
        s_worker_send(worker, MDPW_REQUEST, NULL, msg);
        //  Workers are scheduled in the round-robin fashion
        zlist_append(self->waiting, worker);
        zmsg_destroy(&msg);
    }
}

static void
s_service_ready() {
    //  Attach node to service and mark as idle
    node->service = s_service_require(self, service_frame);
    zlist_append(self->waiting, node);
    zlist_append(node->service->waiting, node);
    node->service->workers++;
    node->expiry = zclock_time() + HEARTBEAT_EXPIRY;
    s_service_dispatch(node->service);
    zframe_destroy(&service_frame);
    zclock_log("worker created");
}
#endif

static inline char *
s_msg_address_strhex(parch_msg_t *msg) {
    return zframe_strhex(parch_msg_address(msg));
}

static inline char *
s_msg_service_name_dup(parch_msg_t *msg) {
    return strdup(parch_msg_service(msg));
}

static inline zframe_t *
s_msg_address_dup(parch_msg_t *msg) {
    return zframe_dup(parch_msg_address(msg));
}

static void
s_broker_register_node(broker_t *self, parch_msg_t *msg) {
    char *key = s_msg_address_strhex(msg);
    char *sname = parch_msg_service(msg);
    uint8_t incoming = parch_msg_incoming(msg);
    uint8_t outgoing = parch_msg_outgoing(msg);
    uint8_t throughput = parch_msg_throughput(msg);

    if (sname == 0 || strlen(sname) == 0 || !is_safe_ascii(sname)) {
        // A connection has to have a service name
        zframe_t *addr = parch_msg_address(msg);
        parch_msg_t *node_msg = parch_msg_new_disconnect_indication_msg(addr,
                local_procedure_error,
                err_invalid_calling_address);
        parch_msg_send(&node_msg, self->socket);
        if (self->verbose)
            zclock_log("I: [%s] broker -- connect rejected: bad address", key);

    } else if ((incoming && outgoing)
            || ((throughput != 0) && (throughput < THROUGHPUT_CLASS_MIN || throughput > THROUGHPUT_CLASS_MAX))) {
        // FIXME -- THROUGHPUT_CLASS_MAX should be a config option
        // A service that prevents both incoming and outgoing messages is not allowed.
        // Also, a valid throughput must be specified.
        zframe_t *addr = parch_msg_address(msg);
        parch_msg_t *node_msg = parch_msg_new_disconnect_indication_msg(addr,
                local_procedure_error,
                err_facility_parameter_not_allowed);
        parch_msg_send(&node_msg, self->socket);
        if (self->verbose)
            zclock_log("I: [%s] broker -- connect rejected: bad facility parameter", key);
    } else {
        // Make a state engine for the node
        // and register the node's service
        node_t *val = (node_t *) zmalloc(sizeof (node_t));
        assert(val);
        val->broker = self;
        val->node_address = zframe_dup(parch_msg_address(msg));
        val->service_name = strdup(parch_msg_service(msg));
        s_node_set_service(val);
        val->engine = parch_state_engine_new(self, val, incoming, outgoing, throughput);
        assert(val->engine);
        int rc = zhash_insert(self->nodes, key, val);
        assert(rc != -1);
        parch_msg_t *node_msg = parch_msg_new(PARCH_MSG_CONNECT_INDICATION);
        parch_msg_set_address(node_msg, parch_msg_address(msg));
        parch_msg_send(&node_msg, self->socket);
    }
    free(key);
}

void
parch_simple_node_destroy(node_t **self_p) {
    node_t *self = self_p[0];
    if (self->broker)
        self->broker = NULL;
    if (self->node_address) {
        zframe_destroy(&self->node_address);
        self->node_address = NULL;
    }
    if (self->service_name) {
        free(self->service_name);
        self->service_name = NULL;
    }
    free(self);
}

int
s_socket_event(zloop_t *loop __attribute__((unused)), zmq_pollitem_t *item, void *arg) {
    broker_t *self = (broker_t *) arg;
    assert(self);

    if (!(item->revents & ZMQ_POLLIN))
        return 0;

    parch_msg_t *msg = s_broker_msg_recv(self);
    if (msg == NULL) {
        zclock_log("I: received malformed msg");
        return 0;
    }

    char *identity = s_msg_address_strhex(msg);

    // Create a state machine for this message, if necessary
    if (zhash_lookup(self->nodes, identity) == NULL) {
        if (parch_msg_id(msg) == PARCH_MSG_CONNECT)
            s_broker_register_node(self, msg);
        else {
            parch_msg_send_clear_request(parch_msg_address(msg), local_procedure_error, err_packet_on_unassigned_logical_channel);
            if (self->verbose) {
                char * const msg_addr = s_msg_address_strhex(msg);
                zclock_log("I: [%s] socket -- '%s' from unregistered socket was ignored", msg_addr, parch_msg_command(msg));
                free(msg_addr);
                parch_msg_dump(msg);
            }
            goto cleanup;
        }
    }
    if (self->verbose) {
        char * const msg_addr = s_msg_address_strhex(msg);
        zclock_log("I: [%s] socket -- received '%s' from node", msg_addr, parch_msg_command(msg));
        free(msg_addr);
        parch_msg_dump(msg);
    }
    // Ask the state machine what to do with this message
    node_t *node_val = (node_t *) zhash_lookup(self->nodes, identity);


    parch_state_engine_t *engine = node_val->engine;
    int done = parch_state_engine_x_message(engine, msg);

    if (done) {
        parch_state_engine_destroy(&node_val->engine);
        zhash_delete(self->nodes, identity);
        parch_simple_node_destroy(&node_val);
    }
cleanup:
    parch_msg_destroy(&msg);
    free(identity);
    return 0;
}

static void
s_broker_destroy(broker_t **self_p) {
    assert(self_p);
    if (*self_p) {
        broker_t *self = *self_p;
        zctx_destroy(&self->ctx);
        zhash_destroy(&self->nodes);
        if (self->poll_input) {
            free(self->poll_input);
            self->poll_input = NULL;
        }
        free(self);
        *self_p = NULL;
    }
}

static broker_t *
s_broker_new(int verbose, char *endpoint) {
    broker_t *self = (broker_t *) zmalloc(sizeof (broker_t));

    //  Initialize broker state
    self->ctx = zctx_new();
    assert(self->ctx);

    self->verbose = verbose;
    self->nodes = zhash_new();
    self->services = zhash_new();

    self->socket = zsocket_new(self->ctx, ZMQ_ROUTER);
    assert(self->socket);
    zsocket_bind(self->socket, endpoint);

    self->loop = zloop_new();
    assert(self->loop);
    zloop_set_verbose(self->loop, self->verbose);


    self->poll_input = (zmq_pollitem_t *) zmalloc(sizeof (zmq_pollitem_t));
    assert(self->poll_input);
    self->poll_input->socket = self->socket;
    self->poll_input->fd = 0;
    self->poll_input->events = ZMQ_POLLIN;

    int rc = zloop_poller(self->loop, self->poll_input, s_socket_event, self);
    assert(rc != -1);

    zloop_start(self->loop);

    zclock_log("I: SVC broker/0.1 is active at %s", endpoint);
    return self;
}

static parch_msg_t *
s_broker_msg_recv(broker_t *self) {
    parch_msg_t *msg;
    msg = parch_msg_recv(self->socket);
    return msg;
}

static void
s_node_mark_busy(node_t *self) {
    zlist_remove(self->service->idle, self);
    // If it is in the busy list, move it to the bottom
    zlist_remove(self->service->busy, self);
    zlist_append(self->service->busy, self);
    self->service->workers = zlist_size(self->service->idle) + zlist_size(self->service->busy);
}

static void
s_node_mark_idle(node_t *self) {
    zlist_remove(self->service->idle, self);
    zlist_remove(self->service->busy, self);
    zlist_append(self->service->idle, self);
    self->service->workers = zlist_size(self->service->idle) + zlist_size(self->service->busy);
}

int
parch_node_call_request(node_t *self, char const * const service_name) {
    s_node_mark_busy(self);
    service_t *service =
            (service_t *) zhash_lookup(self->broker->services, service_name);
    if (service == NULL || zlist_size(service->idle) == 0) {
        s_node_mark_idle(self);
        if (self->broker->verbose) {
            char *self_addr = zframe_strhex(self->node_address);
            if (service == NULL)
                zclock_log("I: [%s] broker -- service '%s' unknown",
                    self_addr,
                    service_name);
            else
                zclock_log("I: [%s] broker -- no available peers for service '%s'",
                    self_addr,
                    service_name);

            free(self_addr);
        }
        return 0;
    }

    node_t *peer = s_service_get_node(service);
    self->peer = peer;
    if (self->broker->verbose) {
        char *self_addr = zframe_strhex(self->node_address);
        char *service_addr = zframe_strhex(peer->node_address);
        zclock_log("I: [%s] broker -- connecting to peer [%s], service '%s'",
                self_addr,
                service_addr,
                service_name);
        free(self_addr);
        free(service_addr);
    }

    peer->peer = self;
    if (self->broker->verbose) {
        char *self_addr = zframe_strhex(self->node_address);
        char *service_addr = zframe_strhex(peer->node_address);
        zclock_log("I: [%s] broker -- connecting to peer [%s], service '%s'",
                service_addr,
                self_addr,
                service_name);
        free(self_addr);
        free(service_addr);
    }
    s_node_mark_busy(peer);
    return 1;
}

void
parch_node_disconnect_from_service(node_t *self) {
    s_node_mark_idle(self);
    zlist_remove(self->service->idle, self);
    self->service->workers = zlist_size(self->service->idle) + zlist_size(self->service->busy);
    if (self->service->workers == 0) {
        zhash_delete(self->broker->services, self->service_name);
        if (self->broker->verbose)
            zclock_log("I: broker -- removed service '%s'", self->service_name);
    }
    free(self->service);
    self->service = NULL;
}

void
parch_node_disconnect_from_peer(node_t *self) {
    assert(self != NULL);

    if (self->peer == NULL) {
        if (self->broker->verbose) {
            char *self_addr = zframe_strhex(self->node_address);
            zclock_log("I: [%s] broker -- can't disconnect from peer because there isn't one",
                    self_addr);
            free(self_addr);
        }
        return;
    }

    node_t *peer = self->peer;
    self->peer = NULL;
    if (self->broker->verbose) {
        char *self_addr = zframe_strhex(self->node_address);
        char *service_addr = zframe_strhex(peer->node_address);
        zclock_log("I: [%s] broker -- disconnecting from peer [%s]",
                self_addr,
                service_addr);
        free(self_addr);
        free(service_addr);
    }

    peer->peer = NULL;
    if (self->broker->verbose) {
        char *self_addr = zframe_strhex(self->node_address);
        char *service_addr = zframe_strhex(peer->node_address);
        zclock_log("I: [%s] broker -- disconnecting from peer [%s]",
                service_addr,
                self_addr);
        free(self_addr);
        free(service_addr);
    }
}

int main(int argc __attribute__((unused)), char *argv [] __attribute__((unused))) {
    int verbose = 1;
    broker_t *self = s_broker_new(verbose, "tcp://*:5555");
    s_broker_destroy(&self);
    return EXIT_SUCCESS;
}
