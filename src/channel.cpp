#include "../include/action.h"
#include "../include/channel.h"
#include "../include/log.h"
#include "../include/diagnostic.h"
#include "../include/connections.h"
#include "../include/flow.h"

extern throughput_t opt_throughput;
channel_store_t channel_store;

Channel *
find_channel(channel_store_t* p_channel_store, const char *key, const char *dname) {
    int found = 0;
    Channel* ch = (Channel*) NULL;
    for (auto it = p_channel_store->begin();
         it != p_channel_store->end();
         ++ it) {
        if (strcmp((*it)->x_key, key) == 0) {
            found = 1;
            ch = *it;
            break;
        }
        else if (strcmp((*it)->y_key, key) == 0) {
            found = 2;
            ch = *it;
            break;
        }
    }
    if (found == 0) {
        NOTE("did not find %s in channel_store", dname);
    }
    else if (found == 1) {
        NOTE("found %s as X in channel_store", dname);
    }
    else if (found == 2) {
        NOTE("found %s as Y in channel_store", dname);
    }
    else
        abort();
    return ch;
}

void
add_channel (channel_store_t* store, const char *x_key, const char *x_dname,
             const char *y_key, const char *y_dname) {
    INFO ("connecting %s/%s as channel", x_dname, y_dname);
    Channel *ch = new Channel(x_key, y_key);
    store->push_back(ch);
}

void
remove_channel (channel_store_t* cs, Channel* c, const char* xdn, const char* ydn) {
    INFO ("removing channel %s/%s", xdn, ydn);
    for (auto it = cs->begin();
         it != cs->end();
         ++ it) {
        if (strcmp((*it)->x_key, c->x_key) == 0 && strcmp((*it)->y_key, c->y_key) == 0) {
            cs->erase(it);
            break;
        }
    }
}

#if 0
void
channel_store_remove_channel (channel_store_t *c, int id) {
    INFO ("removing channel %s/%s", dname(c->channels[id].x_key), dname(c->channels[id].y_key));
    free(c->channels[c->count].x_key);
    free(c->channels[c->count].y_key);
    if (id + 1 < c->count)
        memcpy(&c->channels[id], &c->channels[id+1], (c->count - id - 1) * sizeof(channel_t));
    c->count --;
}
#endif

static Channel
do_clear (Channel c) {
    msg_t *x_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    msg_t *y_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic (y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    c.state = state_y_clear_request;
    return c;
}

// Forced shutdown of both ends of a connection.  Is this necessary?
static Channel
do_disconnect (Channel c) {
    msg_t *x_msg = msg_new(MSG_DISCONNECT_INDICATION);
    msg_set_diagnostic(x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    msg_t *y_msg = msg_new(MSG_DISCONNECT_INDICATION);
    msg_set_diagnostic (y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    connection_disconnect(&connection_store, c.x_key);
    connection_disconnect(&connection_store, c.y_key);

    c.state = state_ready;
    return c;
}

static Channel
do_reset (Channel c, diagnostic_t d) {
    msg_t *x_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    msg_t *y_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic (y_msg, d);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    c.flow = reset(c.flow);
    c.state = state_y_reset_request;
    return c;
}

static Channel
do_x_call_collision (Channel c) {
    c.state = state_call_collision;
    return c;
}

static Channel
do_x_call_request (Channel c, const msg_t *msg) {
    // Validate the facilities requests
    if (!validate (c.packet_size_index)) {
        c = do_clear(c);
    }
    else if (!window_validate(c.window_size)) {
        c = do_clear(c);
    }
    else if (!validate(c.throughput_index)) {
        c = do_clear(c);
    }
    else {
        // CALL REQUEST NEGOTIATION -- STEP 1
        // The call request from X is throttled by the Broker's limitations
        // FIXME: we should be throttling these with configuration options
        c.packet_size_index = throttle(c.packet_size_index, p_last);
        c.window_size = window_throttle(c.window_size, WINDOW_MAX);
        c.throughput_index = throttle(c.throughput_index, opt_throughput);

        // Forward the call request to Y
        msg_t *y_msg = msg_new(MSG_CALL_REQUEST);
        msg_set_service(y_msg, msg_const_service(msg));
        msg_set_packet (y_msg, c.packet_size_index);
        msg_set_window (y_msg, c.window_size);
        msg_set_throughput (y_msg, c.throughput_index);
        connection_msg_send (&connection_store, c.y_key, &y_msg);

        // Set the state to X CALL
        c.state = state_x_call_request;
    }
    return c;
}
static Channel
do_x_clear_confirmation (Channel c, uint8_t d) {
    // Y originally request a clear.  It was forwarded to X.  X has responded.
    // Forward the confirmation to Y
     msg_t *y_msg = msg_new(MSG_CLEAR_CONFIRMATION);
     msg_set_diagnostic(y_msg, d);
     connection_msg_send (&connection_store, c.y_key, &y_msg);

    // Clear this connection

    c.state = state_ready;
    return c;
}

static Channel
do_x_clear_request (Channel c, uint8_t diagnostic) {
    // Forward the clear request on to Y
    msg_t *y_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);
    // Change state
    c.state = state_x_clear_request;
    return c;
}

static Channel
do_x_data (Channel c, uint16_t seq, const zframe_t *data)
{
    // We received a data packet from X. Validate it and forward it to Y.
    size_t siz = zframe_size((zframe_t *)data);
    if (siz == 0) {
        return do_reset(c, d_data_packet_too_small);
    }
    else if (siz > bytes(c.packet_size_index)) {
        return do_reset(c, d_data_packet_too_large);
    }
    else if (seq != c.flow.x_send_sequence) {
        return do_reset(c, d_data_packet_out_of_order);
    }
    else if (!flow_sequence_in_range(seq, c.flow.y_lower_window_edge, c.flow.window_size)) {
        return do_reset(c, d_data_packet_not_in_window);
    }
    else {
        msg_t *y_msg = msg_new(MSG_DATA);
        msg_set_sequence(y_msg, seq);
        msg_set_data(y_msg, zframe_dup((zframe_t *) data));
        connection_msg_send (&connection_store, c.y_key, &y_msg);

        c.flow.x_send_sequence ++;
        return c;
    }
    abort ();
}

static Channel
do_x_disconnect (Channel c) {
    // Inform the peer we're going down
    msg_t *y_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic (y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    // Shut down X
    connection_disconnect(&connection_store, c.x_key);

    // Clear this connection
    c.state = state_ready;
    return c;
}
static Channel
do_x_reset (Channel c, uint8_t d) {
    // Forward reset request to Y
     msg_t *y_msg = msg_new(MSG_RESET_REQUEST);
     msg_set_diagnostic(y_msg, d);
     connection_msg_send (&connection_store, c.y_key, &y_msg);

     // Change state
     c.state = state_x_reset_request;
     return c;
}

static Channel
do_x_reset_confirmation (Channel c, uint8_t d) {
    // Received a reset confirmation from X
    // Forward to Y
     msg_t *y_msg = msg_new(MSG_RESET_CONFIRMATION);
     msg_set_diagnostic(y_msg, d);
     connection_msg_send (&connection_store, c.y_key, &y_msg);

     // Change state
     c.state = state_data_transfer;

     // reset flow control
     c.flow = reset(c.flow);
     return c;
}

static Channel
do_x_rnr (Channel c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  msg_t *y_msg = msg_new(MSG_RNR);
  msg_set_sequence(y_msg, seq);
  connection_msg_send (&connection_store, c.y_key, &y_msg);

  c.flow.x_lower_window_edge = seq;
  return c;
}

static Channel
do_x_rr (Channel c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  msg_t *y_msg = msg_new(MSG_RR);
  msg_set_sequence(y_msg, seq);
  connection_msg_send (&connection_store, c.y_key, &y_msg);

  c.flow.x_lower_window_edge = seq;
  return c;
}


/////////////////////////
static Channel
do_y_call_collision (Channel c) {
    c.state = state_call_collision;
    return c;
}

static Channel
do_y_call_accepted (Channel c) {
    // Validate the facilities requests
    if (!validate (c.packet_size_index)) {
        c = do_clear(c);
    }
    else if (!window_validate(c.window_size)) {
        c = do_clear(c);
    }
    else if (!validate(c.throughput_index)) {
        c = do_clear(c);
    }
    else {
        // Throttle the facilities requests
        // FIXME: we should be throttling these with configuration options
        c.packet_size_index = throttle(c.packet_size_index, p_last);
        c.window_size = window_throttle(c.window_size, WINDOW_MAX);
        c.throughput_index = throttle(c.throughput_index, t_last);

        // Forward the call accepted to X
        msg_t *x_msg = msg_new(MSG_CALL_ACCEPTED);
        // msg_set_service(x_msg, strdup(msg_service(msg)));
        msg_set_packet (x_msg, c.packet_size_index);
        msg_set_window (x_msg, c.window_size);
        msg_set_throughput (x_msg, c.throughput_index);
        connection_msg_send (&connection_store, c.x_key, &x_msg);

        // Set the state to X CALL
        c.state = state_data_transfer;
    }
    return c;
}
static Channel
do_y_clear_confirmation (Channel c, uint8_t d) {
    // Y originally request a clear.  It was forwarded to X.  X has responded.
    // Forward the confirmation to Y
     msg_t *x_msg = msg_new(MSG_CLEAR_CONFIRMATION);
     msg_set_diagnostic(x_msg, d);
     connection_msg_send (&connection_store, c.x_key, &x_msg);

    // Clear this connection
    c.state = state_ready;
    return c;
}

static Channel
do_y_clear_request (Channel c, uint8_t diagnostic) {
    // Forward the clear request on to Y
    msg_t *x_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic(x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);
    // Change state
    c.state = state_x_clear_request;
    return c;
}

static Channel
do_y_data (Channel c, uint16_t seq, const zframe_t *data)
{
    // We received a data packet from X. Validate it and forward it to Y.
    size_t siz = zframe_size((zframe_t *)data);
    if (siz == 0) {
        WARN("y data packet #%d too small", seq);
        return do_reset(c, d_data_packet_too_small);
    }
    else if (siz > bytes(c.packet_size_index)) {
        WARN("y data packet #%d too large", seq);
        return do_reset(c, d_data_packet_too_large);
    }
    else if (seq != c.flow.y_send_sequence) {
        WARN("y data packet #%d out of order, expected #%s", seq, c.flow.y_send_sequence);
        return do_reset(c, d_data_packet_out_of_order);
    }
    else if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size)) {
        WARN("y data packet #%d not in window [%d to %d]", seq, c.flow.x_lower_window_edge, c.flow.x_lower_window_edge + c.flow.window_size);
        return do_reset(c, d_data_packet_not_in_window);
    }
    else {
        msg_t *x_msg = msg_new(MSG_DATA);
        msg_set_sequence(x_msg, seq);
        msg_set_data(x_msg, zframe_dup((zframe_t *) data));
        connection_msg_send (&connection_store, c.x_key, &x_msg);

        c.flow.y_send_sequence ++;
        return c;
    }
    abort ();
}

static Channel
do_y_disconnect (Channel c) {
    // Inform the peer we're going down
    msg_t *x_msg = msg_new(MSG_CLEAR_REQUEST);
    msg_set_diagnostic (x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    // Shut down X
    connection_disconnect(&connection_store, c.x_key);

    // Clear this connection
    c.state = state_ready;
    return c;
}
static Channel
do_y_reset (Channel c, uint8_t d) {
    // Forward reset request to Y
     msg_t *x_msg = msg_new(MSG_RESET_REQUEST);
     msg_set_diagnostic(x_msg, d);
     connection_msg_send (&connection_store, c.x_key, &x_msg);

     // Change state
     c.state = state_x_reset_request;
     return c;
}

static Channel
do_y_reset_confirmation (Channel c, uint8_t d) {
    // Received a reset confirmation from X
    // Forward to Y
     msg_t *x_msg = msg_new(MSG_RESET_CONFIRMATION);
     msg_set_diagnostic(x_msg, d);
     connection_msg_send (&connection_store, c.x_key, &x_msg);

     // Change state
     c.state = state_data_transfer;

     // reset flow control
     c.flow = reset(c.flow);
     return c;
}

static Channel
do_y_rnr (Channel c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  msg_t *x_msg = msg_new(MSG_RNR);
  msg_set_sequence(x_msg, seq);
  connection_msg_send (&connection_store, c.x_key, &x_msg);

  c.flow.x_lower_window_edge = seq;
  return c;
}

static Channel
do_y_rr (Channel c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  msg_t *x_msg = msg_new(MSG_RR);
  msg_set_sequence(x_msg, seq);
  connection_msg_send (&connection_store, c.x_key, &x_msg);

  c.flow.x_lower_window_edge = seq;
  return c;

}

//////////////////////////


Channel
channel_dispatch (Channel c, const char *x_dname, const char *y_dname, const msg_t *msg) {
    char* msg_key = NULL;
    assert(c.x_key);
    assert(c.y_key);
    state_t state_orig = c.state;

    msg_key = (char*) zframe_strhex((zframe_t*) msg_const_address(msg));
    bool is_y = strcmp(msg_key, c.y_key) == 0;

    action_t a;
    if (is_y)
        a = y_action_table[c.state][msg_const_id(msg)];
    else
        a = x_action_table[c.state][msg_const_id(msg)];
    INFO("%s/%s dispatching %s in %s", x_dname, y_dname, name(a), name(c.state));
    free (msg_key);
    msg_key = NULL;
    switch (a) {
        case a_unspecified:
            abort ();
            break;
        case a_discard:
            break;
        case a_reset:
            c = do_reset (c, diagnostic);
            break;
        case a_clear:
            c = do_clear (c);
            break;
        case a_disconnect:
            c = do_disconnect (c);
            break;

        case a_x_connect:
            // Connection happens before the channel exists
            abort ();
            break;
        case a_x_disconnect:
            c = do_x_disconnect (c);
            break;
        case a_x_call_request:
            do_x_call_request (c, msg);
            break;
        case a_x_call_accepted:
            // Should never happen, since X always connects before Y
            abort ();
            break;
        case a_x_call_collision:
            c = do_x_call_collision (c);
            break;
        case a_x_clear_request:
            c = do_x_clear_request (c, msg_const_diagnostic(msg));
            break;
        case a_x_clear_confirmation:
            c = do_x_clear_confirmation (c, msg_const_diagnostic (msg));
            break;
        case a_x_data:
            c = do_x_data (c, msg_const_sequence(msg), msg_const_data(msg));
            break;
        case a_x_rr:
            c = do_x_rr (c, msg_const_sequence(msg));
            break;
        case a_x_rnr:
            c = do_x_rnr (c, msg_const_sequence(msg));
            break;
        case a_x_reset:
            c = do_x_reset (c, msg_const_diagnostic(msg));
            break;
        case a_x_reset_confirmation:
            c = do_x_reset_confirmation(c, msg_const_diagnostic(msg));
            break;

        // case a_y_connect:
        case a_y_disconnect:
            c = do_y_disconnect (c);
            break;
        case a_y_call_request:
            // Should never happen, since X connects before Y be definition.
            abort ();
            break;
        case a_y_call_accepted:
            c = do_y_call_accepted (c);
            break;
        case a_y_call_collision:
            c = do_y_call_collision (c);
            break;
        case a_y_clear_request:
            c = do_y_clear_request (c, msg_const_diagnostic(msg));
            break;
        case a_y_clear_confirmation:
            c = do_y_clear_confirmation (c, msg_const_diagnostic (msg));
            break;
        case a_y_data:
            c = do_y_data (c, msg_const_sequence(msg), msg_const_data(msg));
            break;
        case a_y_rr:
            c = do_y_rr (c, msg_const_sequence(msg));
            break;
        case a_y_rnr:
            c = do_y_rnr (c, msg_const_sequence(msg));
            break;
        case a_y_reset:
            c = do_y_reset (c, msg_const_diagnostic(msg));
            break;
        case a_y_reset_confirmation:
            c = do_y_reset_confirmation(c, msg_const_diagnostic(msg));
            break;

#if 0
    a_unspecified = 0,
    a_discard,
    a_reset,
    a_clear,
    a_disconnect,

    // These are actions requested by the node
    a_x_connect,
    a_x_disconnect,
    a_x_call_request,
    a_x_call_accepted,
    a_x_call_collision,
    a_x_clear_request,
    a_x_clear_confirmation,
    a_x_data,
    a_x_rr,
    a_x_rnr,
    a_x_reset,
    a_x_reset_confirmation,

    // These are actions request by the peer
    a_y_disconnect,
    a_y_call_request,
    a_y_call_accepted,
    a_y_call_collision,
    a_y_clear_request,
    a_y_clear_confirmation,
    a_y_data,
    a_y_rr,
    a_y_rnr,
    a_y_reset,
    a_y_reset_confirmation,
    a_last = a_y_reset_confirmation,
#endif
    }
    if(c.state != state_orig)
        INFO("channel %s/%s changing to %s", x_dname, y_dname, state_names[c.state]);
    return c;
}

