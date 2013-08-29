#include "../include/channel.h"
#include "../include/connections.h"
#include "../include/lib.h"
#include "../include/log.h"

static zctx_t *ctx;
void *sock;
static zloop_t *loop;
static zmq_pollitem_t poll_input = {NULL, 0, ZMQ_POLLIN, 0};

static int
s_recv(zloop_t *loop, zmq_pollitem_t *item, void *arg);

void poll_init(bool verbose, char *endpoint) {
    //  Initialize broker state
    ctx = zctx_new_or_die();
    sock = zsocket_new_or_die(ctx, ZMQ_ROUTER);
    zsocket_bind(sock, endpoint);
    loop = zloop_new_or_die();
    zloop_set_verbose(loop, verbose);
    poll_input.socket = sock;
    int rc = zloop_poller(loop, &poll_input, s_recv, NULL);
    assert(rc != -1);
}

static int
s_recv(zloop_t *loop, zmq_pollitem_t *item, void *arg) {
    if (!(item->revents & ZMQ_POLLIN))
        return 0;

    msg_t *msg;
    msg = msg_recv(sock);
    if (msg == NULL) {
        WARN("W: received malformed message");
        return 0;
    }

	ConnectionStore connection_store;

    char *key = zframe_strhex (msg_address(msg));
    INFO("received %s from %s", msg_command(msg), dname(key));
    Channel* channel = channel_store.find_channel(key);
	Connection* worker = connection_store.find_worker(key);

    if (channel != NULL) {
        // Step 1: if msg comes from a worker in a logical channel, we let the
        // state engine dispatch it.
		INFO("handling off %s to channel %s/%s", parch_msg_command(msg),
			connection_store.find_worker(channel->x_key)->dname
			connection_store.find_worker(channel->y_key)->dname);
		
        channel_store.channels[channel_id] = channel_dispatch (channel_store.channels[channel_id], msg);
        if (channel_store.channels[channel_id].state == state_ready) {
            channel_store_remove_channel (&channel_store, channel_id);
        }
    }
    else if (connection_id != 0) {
        // Step 2: if msg comes from a worker that is in the worker directory, we let the
        // worker directory dispatch it.
        NOTE("handling off %s to connection %s", parch_msg_command(msg), dname(connection_store.connections[connection_id].key));
        connection_dispatch (&connection_store, key, msg);
    }
    else {
        // Step 3: if msg comes from an unknown directory, we dispatch it here
        // The only message we can dispatch is CONNECT_REQUEST
        NOTE("handling off %s to top-level", parch_msg_command(msg));
        if (parch_msg_id(msg) == PARCH_MSG_CONNECT)
            connection_connect (&connection_store, key, msg);
    }

    return 0;
}

void poll_start (void) {
    // Takes over control of the thread.
    zloop_start(loop);
}

#if 0
void (*msg_handler)(parch_msg_t msg);

struct _parch_msg_t {
    zframe_t *address;          //  Address of peer if any
    int id;                     //  parch_msg message ID
    byte *needle;               //  Read/write pointer for serialization
    byte *ceiling;              //  Valid upper limit for read pointer
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
msg_s4_x_data (zframe_t *addr, uint32_t sequence, zframe_t *data,
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
    }
    else if (state == s5_collision)
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
        size_t siz = zframe_size(parch_msg_data(self->request));
        size_t siz_max = parch_packet_bytes(self->call_packet_index);
        if (siz == 0) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_packet_too_short;
        } else if (siz > siz_max) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_packet_too_long;
        } else if (parch_msg_sequence(self->request) != self->x_sequence_number) {
            ok = false;
            self->next_cause = local_procedure_error;
            self->next_diagnostic = err_invalid_ps;
        }
    }

    if (ok == false) {
        self->state = s_unspecified;
        s_state_engine_log(self, diagnostic_messages[self->next_diagnostic]);
    } else {
        parch_msg_t *msg = parch_msg_dup(self->request);
        s_state_engine_send_msg_to_peer_and_log(self, &msg);
        self->x_sequence_number++;
    }
}

#endif

static int
poll_get_msg_from_socket(zloop_t *loop __attribute__((unused)), zmq_pollitem_t *item, void *arg) {
    if (!(item->revents & ZMQ_POLLIN))
        return 0;

    parch_msg_t *msg;
    msg = parch_msg_recv(sock);
    if (msg == NULL) {
        zclock_log("I: received malformed message");
        return 0;
    }
    parch_msg_t m;
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
    parch_msg_destroy(&msg);

    // When we get a message from node, the address is the address back to the node
    if (m.id == PARCH_MSG_CONNECT)
        msg_handle_connect(m.address, m.protocol, m.version, m.service, m.incoming, m.outgoing, m.throughput);
    else if (m.id == PARCH_MSG_CALL_REQUEST)
        msg_handle_call_request (m.address, m.service, m.outgoing, m.incoming, m.packet, m.window, m.throughput);
#if 0
    if (m.id == PARCH_MSG_DATA)
        msg_handle_data (m.address, m.sequence, m.data);
    else if (m.id == PARCH_MSG_RR)
        msg_handle_rr (m.address, m.sequence);
    else if (m.id == PARCH_MSG_RNR)
        msg_handle_rnr (m.address, m.sequence);
    else if (m.id == PARCH_MSG_CALL_REQUEST)
        msg_handle_call_request (m.address, m.service, m.outgoing, m.incoming, m.packet, m.window, m.throughput);
    else if (m.id == PARCH_MSG_CALL_ACCEPTED)
        msg_handle_call_ACCEPTED(m.address, m.service, m.outgoing, m.incoming, m.packet, m.window, m.throughput);
    else if (m.id == PARCH_MSG_CLEAR_REQUEST)
        msg_handle_clear_request(m.address, m.cause, m.diagnostic);
    else if (m.id == PARCH_MSG_CLEAR_CONFIRMATION)
        msg_handle_clear_confirmation(m.address, m.cause, m.diagnostic);
    else if (m.id == PARCH_MSG_RESET_REQUEST)
        msg_handle_reset_request(m.address, m.cause, m.diagnostic);
    else if (m.id == PARCH_MSG_RESET_CONFIRMATION)
        msg_handle_reset_confirmation(m.address, m.cause, m.diagnostic);

    else if (m.id == PARCH_MSG_CONNECT_INDICATION)
        msg_handle_connect(m.address, m.incoming, m.outgoing, m.throughput);
    else if (m.id == PARCH_MSG_DISCONNECT)
        msg_handle_disconnect(m.address);
    else if (m.id == PARCH_MSG_DISCONNECT_INDICATION)
        msg_handle_disconnect_indication(m.address, m.cause, m.diagnostic);
    else
        ;
#endif
    return 0;
}




void
add_timer(size_t delay_in_sec, zloop_fn handler, void *arg) {
    zloop_timer (loop, delay_in_sec * 1000, 1, handler, arg);
}

void
send_msg_and_free(parch_msg_t **ppmsg)
{
    parch_msg_send(ppmsg, sock);
}

static void
finalize_poll(void) {

    zloop_poller_end(loop, &poll_input);
    zloop_destroy(&loop);
    zsocket_destroy(ctx, sock);
    zctx_destroy(&ctx);
}

static char *negotiate_connection_name (const char *service) {
    // FIXME do validity checking
    return strdup(service);
}

static byte negotiate_connection_throughput (byte throughput) {
    if (throughput == 0)
        return 7;
    return throughput;
}

static byte negotiate_connection_incoming (byte throughput) {
    return throughput;
}

static byte negotiate_connection_outgoing (byte throughput) {
    return throughput;
}


void
msg_handle_connect(zframe_t *address, const char *protocol, byte version, const char *service,
        byte incoming, byte outgoing, byte throughput) {
    char *key = zframe_strhex(address);

    // If this address already has a connection, send a diagnostic message back.

    // Otherwise, add worker to the worker directory.
    char *service2 = negotiate_connection_name (service);
    char throughput2 = negotiate_connection_throughput(throughput);
    byte incoming2 = negotiate_connection_incoming(incoming);
    byte outgoing2 = negotiate_connection_incoming(outgoing);
    add_to_worker_directory(key, address, service2, throughput2, incoming2, outgoing2);

     parch_msg_t *self = parch_msg_new (PARCH_MSG_CONNECT_INDICATION);
     parch_msg_set_address(self, address);
   parch_msg_set_incoming (self, incoming2);
    parch_msg_set_outgoing (self, outgoing2);
    parch_msg_set_throughput (self, throughput2);
    return parch_msg_send (&self, sock);
}


void
msg_handle_call_request(zframe_t *address, const char *service, byte outgoing, byte incoming, byte packet,
                uint16_t window, byte throughput) {
    // Check to see if there is a logical channel for this address.

    // If there is, get the state

    // The state plus



    // Find a service with the given name
    int index = find_service(service, address);

    // throttle facility request
    packet2 = func(packet);  // throttle
    int channel_number = open_logical_channel (char *x_key, char *y_key, incoming, outgoing, throughput_index, packet_index, window_size);

         parch_msg_t *self = parch_msg_new (PARCH_MSG_CALL_REQUEST);
     parch_msg_set_address(self, y_address(channels[channel_number]))
   parch_msg_set_incoming (self, incoming2);
    parch_msg_set_outgoing (self, outgoing2);
    parch_msg_set_throughput (self, throughput2);
    return parch_msg_send (&self, sock);
}

#endif