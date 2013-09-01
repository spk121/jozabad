#include <string>
#include <memory>
#include "../include/connections.h"
#include "../include/msg.h"
#include "../include/log.h"
#include "../include/poll.h"
#include "../include/throughput.h"
#include "../include/window.h"
#include "../include/action.h"

connection_store_t connection_store;
channel_store_t channel_store;
extern throughput_t opt_throughput;
extern void *sock;

// The DNAMES are used in log messages, because hex addresses are hard
// to keep straight.
static int iter = 0;
static char dnames[DNAME_COUNT][DNAME_LENGTH_MAX] = {

    "Anthony", "Brock", "Casey", "Dorotha", "Estelle", "Flora",
    "Garry", "Hector", "Iago", "Jasmin", "Kenji", "Laura", "Margot",
    "Nanette", "Orpheus", "Paul", "Quentin", "Rasheeda", "Sage",
    "Tuan", "Ursula", "Vonnie", "William", "Xander", "Yukiko",
    "Zachariah",

    "Arlene", "Bethann", "Camellia", "Danuta", "Emmaline", "Floretta",
    "Gayle", "Hollie", "Isaac", "Jeane", "Kathrine", "Laurie",
    "Marianela", "Nichole", "Oleta", "Priscilla", "Quartus", "Rana",
    "Shara", "Toni", "Ulysses", "Vallie", "Walt", "Xerxes",
    "Yehoyada", "Zephaniah",

    "Ava", "Beula", "Carol", "Daren", "Enriqueta", "Francoise",
    "Georgann", "Haggai", "Ira", "Jeanetta", "Keenan", "Leia",
    "Marie", "Noah", "Obadiah", "Peter", "Quirinius", "Rebecca",
    "Sharyn", "Trish", "Uzziah", "Velvet", "Wayland", "Xerces",
    "Yessica", "Zero",

    "Adam", "Bessie", "Carlos", "Donte", "Errol", "Frederic",
    "Giuseppe", "Hai", "Irene", "Jeannette", "Keesha", "Lelah",
    "Matt", "Nestor", "Oleta", "Pamela", "Queenie", "Reynalda",
    "Sherill", "Thomas", "Uriel", "Victor", "Weston", "Xibiah",
    "Yoav", "Zara"

};

Connection::Connection(zframe_t *a, const char *n, throughput_t t, direction_t d) {
    assert(validate(n));
    address = zframe_dup(a);
    id = iter++;
    strcpy(name, n);
    throughput = t;
    direction = d;
}

Connection::~Connection() {
    zframe_destroy(&address);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CONNECTION STORE
//////////////////////////////////////////////////////////////////////////////////////////////////

char const *
dname(uint16_t id) {
    return dnames[id % DNAME_COUNT];
}

const char*
find_dname(const char *key) {
    if(connection_store.count(key) == 0)
        return key;
    else {
        Connection *c = connection_store.at(key);
        return (dname(c->id));
    }
}

Connection*
find_worker(const char *key) {
    Connection *c = NULL;
    TRACE("enter find_worker(key = %p '%s')", key, key);
    if (connection_store.count(key) == 0) {
        NOTE("connection store has no workers");
        return NULL;
    }
    c = connection_store.at(key);
    if (c == NULL) {
        NOTE("worker %s not found in connection store", key);
    } else {
        NOTE("worker %s found in connection store as %s", key, dname(c->id));
    }
    TRACE("exiting find_worker(key = %p '%s') = %p", key, key, c);
    return c;
}

void add_connection(const char *key, msg_t *msg) {
    TRACE("add_connection(key = %p '%s', msg = %p)", key, key, msg);
    int c = connection_store.count(key);
    if (c == 0) {
        zframe_t *address = msg_address(msg);
        char *serv = msg_service(msg);
        throughput_t throughput = (throughput_t) msg_throughput(msg);
        direction_t direction = (direction_t) msg_directionality(msg);

        // CONNECTION NEGOTIATION -- the broker may the throughput requested by the worker.
        if (throughput != throttle(throughput, opt_throughput))
            NOTE("connection negotiation throttles requested throughput - %s -> %s", name(throughput), name(opt_throughput));
        throughput = throttle(throughput, opt_throughput);

        connection_store.insert(pair<string, Connection *>(key, new Connection(address, serv, throughput, direction)));
        INFO("adding %s (%s, %s) as connection %s", key, name(throughput), name(direction), dname(connection_store.at(key)->id));
        NOTE("%d connections exist", connection_store.size());

        msg_t *reply = msg_new(MSG_CONNECT_INDICATION);
        msg_set_throughput(reply, throughput);
        msg_set_directionality(reply, direction);
        msg_set_service(reply, serv);
        connection_msg_send(key, &reply);
    } else {
        ERR("can't add %s as new connection. Connection already exists as %d.", key, dname(connection_store.at(key)->id));
        abort();
    }
}

Connection*
find_worker_by_name(const char* name) {
    TRACE("find_worker_by_name(name = %p '%s')", name, name);
    Connection* connection = NULL;
    for (auto it = connection_store.begin();
            it != connection_store.end();
            ++it) {
        if (strcmp(it->second->name, name) == 0)
            connection = it->second;
    }
    return connection;
}

void
connection_msg_send(const char *key, msg_t **msg) {
    Connection *w = find_worker(key);
    msg_set_address(*msg, w->address);
    INFO("sending '%s' to %s", msg_const_command(*msg), dname(w->id));
    msg_send(msg, sock);
}

void
connection_disconnect(const char *key) {
    TRACE("connection_disconnect(key = %p '%s'", key, key);
    Connection* c = find_worker(key);
    if (c != NULL) {
        INFO("removing connection %s", dname(c->id));
       connection_store.erase(key);
        NOTE("%d connections remain", connection_store.size());
    } else {
        WARN("failed to disconnection non-existent connection %s", key);
    }
}

bool
connection_dispatch(const char *key, msg_t *msg) {
    // This dispatcher is for messages from already-registered workers
    // that aren't connected to peers.  Thus the only valid messages
    // are DISCONNECT and CALL_REQUEST
    bool more = false;
    int id = msg_id(msg);
    if (id == MSG_DISCONNECT) {
        connection_disconnect(key);
    } else if (id == MSG_CALL_REQUEST) {
        // Look for a peer key
        Connection* x = find_worker(key);
        Connection* y = find_worker_by_name(msg_service(msg));
        if (y == 0) {
            // No available worker found.  Send a clear request.
        } else {
            INFO("adding channel %s/%s", dname(x->id), dname(y->id));
            char *y_key = zframe_strhex(y->address);
            add_channel(key, y_key);
            free(y_key);
            more = true;
        }
    }
    return more;
}

Channel::Channel(const char *key_x, const char *key_y) {
    x_key = strdup(key_x);
    y_key = strdup(key_y);
    state = state_ready;
    flow = init();
    packet_size_index = p_last;
    window_size = WINDOW_MAX;
    throughput_index = t_last;
}

Channel::~Channel() {
    free(x_key);
    free(y_key);
}

void Channel::dispatch(const msg_t *msg) {
    state_t state_orig;
    char* msg_key;
    bool is_y;
    action_t a;

    state_orig = state;
    msg_key = zframe_strhex((zframe_t*) msg_const_address(msg));
    is_y = strcmp(msg_key, y_key) == 0;
    a = find_action(state, msg, is_y);
    INFO("%s/%s dispatching %s in %s", find_dname(x_key), find_dname(y_key), name(a), name(state));
    free(msg_key);
    msg_key = NULL;
    switch (a) {
        case a_unspecified:
            abort();
            break;
        case a_discard:
            break;
        case a_reset:
            do_reset((diagnostic_t)msg_const_diagnostic(msg));
            break;
        case a_clear:
            do_clear((diagnostic_t)msg_const_diagnostic(msg));
            break;
        case a_disconnect:
            do_disconnect((diagnostic_t)msg_const_diagnostic(msg));
            break;

        case a_x_connect:
            // Connection happens before the channel exists
            abort();
            break;
        case a_x_disconnect:
            do_x_disconnect((diagnostic_t)msg_const_diagnostic(msg));
            break;
        case a_x_call_request:
            do_x_call_request(msg_const_service(msg),
                    (packet_t) msg_const_packet(msg),
                    msg_const_window(msg),
                    (throughput_t) msg_const_throughput(msg));
            break;
        case a_x_call_accepted:
            // Should never happen, since X always connects before Y
            abort();
            break;
        case a_x_call_collision:
            do_x_call_collision();
            break;
        case a_x_clear_request:
            do_x_clear_request((diagnostic_t) msg_const_diagnostic(msg));
            break;
        case a_x_clear_confirmation:
            do_x_clear_confirmation((diagnostic_t) msg_const_diagnostic(msg));
            break;
        case a_x_data:
            do_x_data(msg_const_sequence(msg), msg_const_data(msg));
            break;
        case a_x_rr:
            do_x_rr(msg_const_sequence(msg));
            break;
        case a_x_rnr:
            do_x_rnr(msg_const_sequence(msg));
            break;
        case a_x_reset:
            do_x_reset((diagnostic_t) msg_const_diagnostic(msg));
            break;
        case a_x_reset_confirmation:
            do_x_reset_confirmation((diagnostic_t) msg_const_diagnostic(msg));
            break;

            // case a_y_connect:
        case a_y_disconnect:
            do_y_disconnect();
            break;
        case a_y_call_request:
            // Should never happen, since X connects before Y be definition.
            abort();
            break;
        case a_y_call_accepted:
            do_y_call_accepted();
            break;
        case a_y_call_collision:
            do_y_call_collision();
            break;
        case a_y_clear_request:
            do_y_clear_request((diagnostic_t) msg_const_diagnostic(msg));
            break;
        case a_y_clear_confirmation:
            do_y_clear_confirmation((diagnostic_t) msg_const_diagnostic(msg));
            break;
        case a_y_data:
            do_y_data(msg_const_sequence(msg), msg_const_data(msg));
            break;
        case a_y_rr:
            do_y_rr(msg_const_sequence(msg));
            break;
        case a_y_rnr:
            do_y_rnr(msg_const_sequence(msg));
            break;
        case a_y_reset:
            do_y_reset((diagnostic_t) msg_const_diagnostic(msg));
            break;
        case a_y_reset_confirmation:
            do_y_reset_confirmation((diagnostic_t) msg_const_diagnostic(msg));
            break;
    }
    if (state != state_orig)
        INFO("channel %s/%s changing to %s", find_dname(x_key), find_dname(y_key), name(state));
}

void Channel::do_clear(diagnostic_t d) {
    msg_t *x_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(x_key, &x_msg);

    msg_t *y_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);

    state = state_y_clear_request;
}

void Channel::do_reset(diagnostic_t d) {
    msg_t *x_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(x_key, &x_msg);

    msg_t *y_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);
    flow = reset(flow);
    state = state_y_reset_request;
}

void Channel::do_disconnect(diagnostic_t d) {
    msg_t *x_msg = msg_new(MSG_DISCONNECT_INDICATION);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(x_key, &x_msg);

    msg_t *y_msg = msg_new(MSG_DISCONNECT_INDICATION);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);

    connection_disconnect(x_key);
    connection_disconnect(y_key);

    state = state_ready;
}

void Channel::do_x_disconnect(diagnostic_t d) {
    // Inform the peer we're going down
    msg_t *y_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);

    // Shut down X
    connection_disconnect(x_key);

    // Clear this connection
    state = state_ready;
}

void Channel::do_x_call_request(const char *service, packet_t p, uint16_t w, throughput_t t) {
    // Validate the facilities requests
    // Note that validate() sets 'diagnostic'
    if (!validate(p)) {
        do_clear(diagnostic);
    } else if (!window_validate(w)) {
        do_clear(diagnostic);
    } else if (!validate(t)) {
        do_clear(diagnostic);
    } else {
        // CALL REQUEST NEGOTIATION -- STEP 1
        // The call request from X is throttled by the Broker's limitations
        // FIXME: we should be throttling these with configuration options
        p = throttle(p, p_last);
        w = window_throttle(w, WINDOW_MAX);
        t = throttle(t, opt_throughput);

        // Forward the call request to Y
        msg_t *y_msg = msg_new(MSG_CALL_REQUEST);
        msg_set_service(y_msg, service);
        msg_set_packet(y_msg, p);
        msg_set_window(y_msg, w);
        msg_set_throughput(y_msg, t);
        connection_msg_send(y_key, &y_msg);

        // Set the state to X CALL
        state = state_x_call_request;
    }
}

void Channel::do_x_call_collision() {
    state = state_call_collision;
}

void Channel::do_x_clear_request(diagnostic_t d) {
    // Forward the clear request on to Y
    msg_t *y_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);
    // Change state
    state = state_x_clear_request;
}

void Channel::do_x_clear_confirmation(diagnostic_t d) {
    // Y originally request a clear.  It was forwarded to X.  X has responded.
    // Forward the confirmation to Y
    msg_t *y_msg = msg_new(MSG_CLEAR_CONFIRMATION);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);

    // Clear this connection
    state = state_ready;
}

void Channel::do_x_data(uint16_t seq, const zframe_t *data) {
    // We received a data packet from X. Validate it and forward it to Y.
    size_t siz = zframe_size((zframe_t *) data);
    if (siz == 0) {
        do_reset(d_data_packet_too_small);
    } else if (siz > bytes(packet_size_index)) {
        do_reset(d_data_packet_too_large);
    } else if (seq != flow.x_send_sequence) {
        do_reset(d_data_packet_out_of_order);
    } else if (!flow_sequence_in_range(seq, flow.y_lower_window_edge, flow.window_size)) {
        do_reset(d_data_packet_not_in_window);
    } else {
        msg_t *y_msg = msg_new(MSG_DATA);
        msg_set_sequence(y_msg, seq);
        msg_set_data(y_msg, zframe_dup((zframe_t *) data));
        connection_msg_send(y_key, &y_msg);

        flow.x_send_sequence++;
    }
}

void Channel::do_x_rr(uint16_t seq) {
    // The new window has to overlap or be immediately above
    if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
        do_x_reset(d_window_edge_out_of_range);
    }        // The new window has to contain the next message to be sent from the peer.
    else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
        do_x_reset(d_window_edge_out_of_range);
    }
    msg_t *y_msg = msg_new(MSG_RR);
    msg_set_sequence(y_msg, seq);
    connection_msg_send(y_key, &y_msg);

    flow.x_lower_window_edge = seq;
}

void Channel::do_x_rnr(uint16_t seq) {
    // The new window has to overlap or be immediately above
    if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
        do_x_reset(d_window_edge_out_of_range);
    }        // The new window has to contain the next message to be sent from the peer.
    else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
        do_x_reset(d_window_edge_out_of_range);
    }
    msg_t *y_msg = msg_new(MSG_RNR);
    msg_set_sequence(y_msg, seq);
    connection_msg_send(y_key, &y_msg);

    flow.x_lower_window_edge = seq;
}

void Channel::do_x_reset(diagnostic_t d) {
    // Forward reset request to Y
    msg_t *y_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);

    // Change state
    state = state_x_reset_request;
}

void Channel::do_x_reset_confirmation(diagnostic_t d) {
    // Received a reset confirmation from X
    // Forward to Y
    msg_t *y_msg = msg_new(MSG_RESET_CONFIRMATION);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(y_key, &y_msg);

    // Change state
    state = state_data_transfer;

    // reset flow control
    flow = reset(flow);
}

void Channel::do_y_disconnect() {
    // Inform the peer we're going down
    msg_t *x_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(x_msg, d_peer_disconnected);
    connection_msg_send(x_key, &x_msg);

    // Shut down X
    connection_disconnect(x_key);

    // Clear this connection
    state = state_ready;
}

void Channel::do_y_call_accepted() {
    // Validate the facilities requests
    // Note that 'diagnostic' gets set as a side-effect here
    if (!validate(packet_size_index)) {
        do_clear(diagnostic);
    } else if (!window_validate(window_size)) {
        do_clear(diagnostic);
    } else if (!validate(throughput_index)) {
        do_clear(diagnostic);
    } else {
        // Throttle the facilities requests
        // FIXME: we should be throttling these with configuration options
        packet_size_index = throttle(packet_size_index, p_last);
        window_size = window_throttle(window_size, WINDOW_MAX);
        throughput_index = throttle(throughput_index, t_last);

        // Forward the call accepted to X
        msg_t *x_msg = msg_new(MSG_CALL_ACCEPTED);
        // msg_set_service(x_msg, strdup(msg_service(msg)));
        msg_set_packet(x_msg, packet_size_index);
        msg_set_window(x_msg, window_size);
        msg_set_throughput(x_msg, throughput_index);
        connection_msg_send(x_key, &x_msg);

        // Set the state to X CALL
        state = state_data_transfer;
    }
}

void Channel::do_y_call_collision() {
    state = state_call_collision;
}

void Channel::do_y_clear_request(diagnostic_t d) {
    // Forward the clear request on to Y
    msg_t *x_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(x_key, &x_msg);
    // Change state
    state = state_x_clear_request;
}

void Channel::do_y_clear_confirmation(diagnostic_t d) {
    // Y originally request a clear.  It was forwarded to X.  X has responded.
    // Forward the confirmation to Y
    msg_t *x_msg = msg_new(MSG_CLEAR_CONFIRMATION);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(x_key, &x_msg);

    // Clear this connection
    state = state_ready;
}

void Channel::do_y_data(uint16_t seq, const zframe_t *data) {
    // We received a data packet from X. Validate it and forward it to Y.
    size_t siz = zframe_size((zframe_t *) data);
    if (siz == 0) {
        WARN("y data packet #%d too small", seq);
        do_reset(d_data_packet_too_small);
    } else if (siz > bytes(packet_size_index)) {
        WARN("y data packet #%d too large", seq);
        do_reset(d_data_packet_too_large);
    } else if (seq != flow.y_send_sequence) {
        WARN("y data packet #%d out of order, expected #%s", seq, flow.y_send_sequence);
        do_reset(d_data_packet_out_of_order);
    } else if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size)) {
        WARN("y data packet #%d not in window [%d to %d]", seq, flow.x_lower_window_edge, flow.x_lower_window_edge + flow.window_size);
        do_reset(d_data_packet_not_in_window);
    } else {
        msg_t *x_msg = msg_new(MSG_DATA);
        msg_set_sequence(x_msg, seq);
        msg_set_data(x_msg, zframe_dup((zframe_t *) data));
        connection_msg_send(x_key, &x_msg);

        flow.y_send_sequence++;
    }
}

void Channel::do_y_rr(uint16_t seq) {
    // The new window has to overlap or be immediately above
    if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
        do_y_reset(d_window_edge_out_of_range);
    }        // The new window has to contain the next message to be sent from the peer.
    else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
        do_y_reset(d_window_edge_out_of_range);
    }
    msg_t *x_msg = msg_new(MSG_RR);
    msg_set_sequence(x_msg, seq);
    connection_msg_send(x_key, &x_msg);

    flow.x_lower_window_edge = seq;
}

void Channel::do_y_rnr(uint16_t seq) {
    // The new window has to overlap or be immediately above
    if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
        do_y_reset(d_window_edge_out_of_range);
    }        // The new window has to contain the next message to be sent from the peer.
    else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
        do_y_reset(d_window_edge_out_of_range);
    }
    msg_t *x_msg = msg_new(MSG_RNR);
    msg_set_sequence(x_msg, seq);
    connection_msg_send(x_key, &x_msg);

    flow.x_lower_window_edge = seq;
}

void Channel::do_y_reset(diagnostic_t d) {
    // Forward reset request to Y
    msg_t *x_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(x_key, &x_msg);

    // Change state
    state = state_x_reset_request;
}

void Channel::do_y_reset_confirmation(diagnostic_t d) {
    // Received a reset confirmation from X
    // Forward to Y
    msg_t *x_msg = msg_new(MSG_RESET_CONFIRMATION);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(x_key, &x_msg);

    // Change state
    state = state_data_transfer;

    // reset flow control
    flow = reset(flow);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CHANNEL STORE
//////////////////////////////////////////////////////////////////////////////////////////////////


Channel*
find_channel(const char *key) {
    TRACE("entering find_channel(key = %p '%s')", key, key);
    int found = 0;
    Channel* ch = (Channel*) NULL;
    for (auto it = channel_store.begin(); it != channel_store.end(); ++it) {
        if (strcmp((*it)->x_key, key) == 0) {
            found = 1;
            ch = *it;
            break;
        } else if (strcmp((*it)->y_key, key) == 0) {
            found = 2;
            ch = *it;
            break;
        }
    }
    if (found == 0) {
        NOTE("did not find %s in channel_store", find_dname(key));
    } else if (found == 1) {
        NOTE("found %s as X in channel_store", find_dname(key));
    } else if (found == 2) {
        NOTE("found %s as Y in channel_store", find_dname(key));
    } else
        abort();
    TRACE("exiting find_channel(key = %p '%s') = %p", key, key, ch);
    return ch;
}

void
add_channel(const char *x_key, const char *y_key) {
    INFO("connecting %s/%s as channel", find_dname(x_key), find_dname(y_key));
    Channel *ch = new Channel(x_key, y_key);
    channel_store.push_back(ch);
}

void
remove_channel(Channel* c) {
    INFO("removing channel %s/%s", find_dname(c->x_key), find_dname(c->y_key));
    for (auto it = channel_store.begin();
            it != channel_store.end();
            ++it) {
        if (strcmp((*it)->x_key, c->x_key) == 0 && strcmp((*it)->y_key, c->y_key) == 0) {
            channel_store.erase(it);
            break;
        }
    }
}