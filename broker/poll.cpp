#include <czmq.h>

#include "../libjoza/joza_lib.h"
#include "../libjoza/joza_msg.h"

#include "connections.h"
#include "dname.h"
#include "lib.h"
#include "log.h"
#include "poll.h"

static zctx_t *ctx;
void *sock;
static zloop_t *loop;
static zmq_pollitem_t poll_input = {NULL, 0, ZMQ_POLLIN, 0};

static int
    s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *arg);
static int
    s_process_(joza_msg_t *msg);

void poll_init(bool verbose, const char *endpoint) {
    //  Initialize broker state
    ctx = zctx_new_or_die();
    sock = zsocket_new_or_die(ctx, ZMQ_ROUTER);
    NOTE("binding ROUTER socket to %s", endpoint);
    zsocket_bind(sock, endpoint);
    loop = zloop_new_or_die();
    zloop_set_verbose(loop, verbose);
    poll_input.socket = sock;
    int rc = zloop_poller(loop, &poll_input, s_recv_, NULL);
    assert(rc != -1);
}

// This is the main callback that gets called whenever a
// message is received from the ZeroMQ loop.
static int
s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *arg) {
    joza_msg_t *msg = NULL;
    int ret = 0;

    if (item->revents & ZMQ_POLLIN) {
        msg = joza_msg_recv(sock);
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
s_process_(joza_msg_t *msg) {
    char *key;
    Channel* channel;
    Connection* worker;
    bool more = false;

    key = zframe_strhex(joza_msg_address(msg));
    do_more:

    worker = NULL;
    channel = NULL;
    worker = find_worker(key);
    if (worker != NULL) {
        channel = find_channel(key);
    }
    
    // If this worker is connected and part of a virtual call, the
    // call's state machine processes the message.
    if (channel != NULL) {
        INFO("handling off %s from worker %s to channel %s/%s",
             joza_msg_const_command(msg),
             find_dname(key),
             find_dname(channel->x_key),
             find_dname(channel->y_key));

        channel->dispatch(msg);
        if (channel->state == state_ready) {
            TRACE("channel %p/%p is ready for removal",
                  find_dname(channel->x_key),
                  find_dname(channel->y_key));
            remove_channel(channel);
        }
    }
    
    // If this worker is connected, but, not part of a call, the
    // connection handler handles the message.
    else if (worker != NULL) {
        INFO("handing off %s from worker %s to connection %s",
             joza_msg_const_command(msg),
             find_dname(key),
             find_dname(key));
        more = connection_dispatch(key, msg);
        if (more)
            goto do_more;
    }

    // If this worker is heretofore unknown, we handle it here, but, only
    // if is a connection request.
    else {
        if (joza_msg_id(msg) == JOZA_MSG_CONNECT) {
            INFO("handling off %s to top-level", joza_msg_const_command(msg));
            add_connection(key, joza_msg_const_address(msg),
                           joza_msg_const_calling_address(msg),
                           (direction_t) joza_msg_const_directionality(msg));
        } else {
            INFO("ignored %s", joza_msg_const_command(msg));
        }
    }
    return 0;
}


void poll_start(void) {
    // Takes over control of the thread.
    NOTE("starting main loop");
    zloop_start(loop);
}

#if 0
void (*joza_msg_handler)(joza_joza_msg_t msg);

struct _joza_joza_msg_t {
    zframe_t *address; //  Address of peer if any
    int id; //  msg message ID
    byte *needle; //  Read/write pointer for serialization
    byte *ceiling; //  Valid upper limit for read pointer
    uint32_t sequence;
    zframe_t *data;
    char *service;
    byte outgoing;
    byte incoming;
    byte packet;
    uint16_t window;
    byte throughput;
    byte cause;
    byte diagnostic;
    char *protocol;
    byte version;
};

#if 0

diagnostic_t
joza_msg_s4_x_data(zframe_t *addr, uint32_t sequence, zframe_t *data,
                   bool outgoing_data_barred, uint32_t x_sequence) {
    if (state == s0_disconnected)
        return err_packet_type_invalid_for_state_s0;
    else if (state == s1_ready)
        return err_packet_type_invalid_for_state_s1;
    else if (state == s2_x_call)
        return err_packet_type_invalid_for_state_s2;
    else if (state == s3_y_call)
        return err_packet_type_invalid_for_state_s3;
    else if (state == s4_data) {
    } else if (state == s5_collision)
        return err_packet_type_invalid_for_state_s5;
    else if (state == s6_x_clear)
        return err_packet_type_invalid_for_state_s6;
    else if (state == s7_y_clear)
        return err_packet_type_invalid_for_state_s7;
    else if (state == s8_x_reset)
        return err_packet_type_invalid_for_state_s8;
    else if (state == s9_y_reset)
        return err_packet_type_invalid_for_state_s9;
    bool ok = true;
    if (self->call_outgoing_data_barred) {
        ok = false;
        self->next_cause = access_barred;
        self->next_diagnostic = err_outgoing_data_barred;
    } else {
        size_t siz = zframe_size(joza_msg_data(self->request));
        size_t siz_max = parch_packet_bytes(self->call_packet_index);
        if (siz == 0) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_packet_too_short;
        } else if (siz > siz_max) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_packet_too_long;
        } else if (joza_msg_sequence(self->request) != self->x_sequence_number) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_invalid_ps;
        }
    }

    if (ok == false) {
        self->state = s_unspecified;
        s_state_engine_log(self, diagnostic_messages[self->next_diagnostic]);
    } else {
        joza_joza_msg_t *msg = joza_msg_dup(self->request);
        s_state_engine_send_joza_joza_msg_to_peer_and_log(self, &msg);
        self->x_sequence_number++;
    }
}

#endif

static int
poll_get_joza_msg_from_socket(zloop_t *loop __attribute__((unused)), zmq_pollitem_t *item, void *arg) {
    if (!(item->revents & ZMQ_POLLIN))
        return 0;

    joza_joza_msg_t *msg;
    msg = joza_msg_recv(sock);
    if (msg == NULL) {
        zclock_log("I: received malformed message");
        return 0;
    }
    joza_joza_msg_t m;
    if (msg->address)
        m.address = zframe_dup(msg->address);
    else
        m.address = 0;
    m.cause = msg->cause;
    if (msg->data)
        m.data = zframe_dup(msg->data);
    else
        m.data = 0;
    m.diagnostic = msg->diagnostic;
    m.id = msg->id;
    m.incoming = msg->incoming;
    m.outgoing = msg->outgoing;
    m.packet = msg->packet;
    if (msg->protocol)
        m.protocol = strdup(msg->protocol);
    else
        m.protocol = NULL;
    m.sequence = msg->sequence;
    m.service = strdup(msg->service);
    m.throughput = msg->throughput;
    m.version = msg->version;
    m.window = msg->window;
    joza_msg_destroy(&msg);

    // When we get a message from node, the address is the address back to the node
    if (m.id == JOZA_MSG_CONNECT)
        joza_msg_handle_connect(m.address, m.protocol, m.version, m.service, m.incoming, m.outgoing, m.throughput);
    else if (m.id == JOZA_MSG_CALL_REQUEST)
        joza_msg_handle_call_request(m.address, m.service, m.outgoing, m.incoming, m.packet, m.window, m.throughput);
#if 0
    if (m.id == JOZA_MSG_DATA)
        joza_msg_handle_data(m.address, m.sequence, m.data);
    else if (m.id == JOZA_MSG_RR)
        joza_msg_handle_rr(m.address, m.sequence);
    else if (m.id == JOZA_MSG_RNR)
        joza_msg_handle_rnr(m.address, m.sequence);
    else if (m.id == JOZA_MSG_CALL_REQUEST)
        joza_msg_handle_call_request(m.address, m.service, m.outgoing, m.incoming, m.packet, m.window, m.throughput);
    else if (m.id == JOZA_MSG_CALL_ACCEPTED)
        joza_msg_handle_call_ACCEPTED(m.address, m.service, m.outgoing, m.incoming, m.packet, m.window, m.throughput);
    else if (m.id == JOZA_MSG_CLEAR_REQUEST)
        joza_msg_handle_clear_request(m.address, m.cause, m.diagnostic);
    else if (m.id == JOZA_MSG_CLEAR_CONFIRMATION)
        joza_msg_handle_clear_confirmation(m.address, m.cause, m.diagnostic);
    else if (m.id == JOZA_MSG_RESET_REQUEST)
        joza_msg_handle_reset_request(m.address, m.cause, m.diagnostic);
    else if (m.id == JOZA_MSG_RESET_CONFIRMATION)
        joza_msg_handle_reset_confirmation(m.address, m.cause, m.diagnostic);

    else if (m.id == JOZA_MSG_CONNECT_INDICATION)
        joza_msg_handle_connect(m.address, m.incoming, m.outgoing, m.throughput);
    else if (m.id == JOZA_MSG_DISCONNECT)
        joza_msg_handle_disconnect(m.address);
    else if (m.id == JOZA_MSG_DISCONNECT_INDICATION)
        joza_msg_handle_disconnect_indication(m.address, m.cause, m.diagnostic);
    else
        ;
#endif
    return 0;
}

void
add_timer(size_t delay_in_sec, zloop_fn handler, void *arg) {
    zloop_timer(loop, delay_in_sec * 1000, 1, handler, arg);
}

void
send_joza_msg_and_free(joza_joza_msg_t **ppmsg) {
    joza_msg_send(ppmsg, sock);
}

static void
finalize_poll(void) {

    zloop_poller_end(loop, &poll_input);
    zloop_destroy(&loop);
    zsocket_destroy(ctx, sock);
    zctx_destroy(&ctx);
}

static char *negotiate_connection_name(const char *service) {
    // FIXME do validity checking
    return strdup(service);
}

static byte negotiate_connection_throughput(byte throughput) {
    if (throughput == 0)
        return 7;
    return throughput;
}

static byte negotiate_connection_incoming(byte throughput) {
    return throughput;
}

static byte negotiate_connection_outgoing(byte throughput) {
    return throughput;
}

void
joza_msg_handle_connect(zframe_t *address, const char *protocol, byte version, const char *service,
                        byte incoming, byte outgoing, byte throughput) {
    char *key = zframe_strhex(address);

    // If this address already has a connection, send a diagnostic message back.

    // Otherwise, add worker to the worker directory.
    char *service2 = negotiate_connection_name(service);
    char throughput2 = negotiate_connection_throughput(throughput);
    byte incoming2 = negotiate_connection_incoming(incoming);
    byte outgoing2 = negotiate_connection_incoming(outgoing);
    add_to_worker_directory(key, address, service2, throughput2, incoming2, outgoing2);

    joza_joza_msg_t *self = joza_msg_new(JOZA_MSG_CONNECT_INDICATION);
    joza_msg_set_address(self, address);
    joza_msg_set_incoming(self, incoming2);
    joza_msg_set_outgoing(self, outgoing2);
    joza_msg_set_throughput(self, throughput2);
    return joza_msg_send(&self, sock);
}

void
joza_msg_handle_call_request(zframe_t *address, const char *service, byte outgoing, byte incoming, byte packet,
                             uint16_t window, byte throughput) {
    // Check to see if there is a logical channel for this address.

    // If there is, get the state

    // The state plus



    // Find a service with the given name
    int index = find_service(service, address);

    // throttle facility request
    packet2 = func(packet); // throttle
    int channel_number = open_logical_channel(char *x_key, char *y_key, incoming, outgoing, throughput_index, packet_index, window_size);

    joza_joza_msg_t *self = joza_msg_new(JOZA_MSG_CALL_REQUEST);
    joza_msg_set_address(self, y_address(channels[channel_number]))
        joza_msg_set_incoming(self, incoming2);
    joza_msg_set_outgoing(self, outgoing2);
    joza_msg_set_throughput(self, throughput2);
    return joza_msg_send(&self, sock);
}

#endif
