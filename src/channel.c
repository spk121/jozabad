#include "../include/svc.h"


channel_store_t channel_store = {CHANNELS_MAGIC, CHANNELS_MIN, CHANNELS_MAX, 0};

void
channel_store_init(channel_store_t *c)
{
    NOTE("initializing channel_store");
    memset (c->channels, 0, sizeof (c->channels));
    c->count = 1;
}

uint16_t
channel_store_find_worker (channel_store_t *c, const char *key)
{
    NOTE("searching for %s in channel_store", dname(key));
    int found = 0;
    int i = c->min;
    while (i < c->count) {
        if (strcmp(c->channels[i].x_key, key) == 0) {
            found = 1;
            break;
        }
        if (strcmp(c->channels[i].y_key, key) == 0) {
            found = 2;
            break;
        }
        i ++;
    }
    if (found) {
        if (found == 1)
            NOTE("found %s as X in channel_store", dname(key));
        else if (found == 2)
            NOTE("found %s as Y in channel_store", dname(key));
        return i;
    }
    NOTE("did not find %s in channel_store", dname(key));
    return 0;
}

void
channel_store_add_channel (channel_store_t *c, const char *x_key, const char *y_key) {
    INFO ("connecting %s/%s as channel", dname(x_key), dname(y_key));
    c->channels[c->count].state = state_ready;
    c->channels[c->count].x_key = strdup(x_key);
    c->channels[c->count].y_key = strdup(y_key);
    c->channels[c->count].throughput_index = 0;
    c->channels[c->count].window_size = 0;
    c->channels[c->count].packet_size_index = 0;
    c->count ++;
}

void
channel_store_remove_channel (channel_store_t *c, int id) {
    INFO ("removing channel %s/%s", dname(c->channels[id].x_key), dname(c->channels[id].y_key));
    free(c->channels[c->count].x_key);
    free(c->channels[c->count].y_key);
    if (id + 1 < c->count)
        memcpy(&c->channels[id], &c->channels[id+1], (c->count - id - 1) * sizeof(channel_t));
    c->count --;
}

static channel_t
do_clear (channel_t c) {
    parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_diagnostic(x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_diagnostic (y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    c.state = state_y_clear_request;
    return c;
}

// Forced shutdown of both ends of a connection.  Is this necessary?
static channel_t
do_disconnect (channel_t c) {
    parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_DISCONNECT_INDICATION);
    parch_msg_set_diagnostic(x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_DISCONNECT_INDICATION);
    parch_msg_set_diagnostic (y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    connection_disconnect(&connection_store, c.x_key);
    connection_disconnect(&connection_store, c.y_key);

    c.state = state_ready;
    return c;
}

static channel_t
do_reset (channel_t c, diagnostic_t d) {
    parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_RESET_REQUEST);
    parch_msg_set_diagnostic(x_msg, d);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_RESET_REQUEST);
    parch_msg_set_diagnostic (y_msg, d);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    c.flow = flow_reset(c.flow);
    c.state = state_y_reset_request;
    return c;
}

static channel_t
do_x_call_collision (channel_t c) {
    c.state = state_call_collision;
    return c;
}

static channel_t
do_x_call_request (channel_t c, parch_msg_t *msg) {
    // Validate the facilities requests
    if (!parch_packet_index_validate (c.packet_size_index)) {
        c = do_clear(c);
    }
    else if (!parch_window_validate(c.window_size)) {
        c = do_clear(c);
    }
    else if (!parch_throughput_index_validate(c.throughput_index)) {
        c = do_clear(c);
    }
    else {
        // Throttle the facilities requests
        // FIXME: we should be throttling these with configuration options
        c.packet_size_index = parch_packet_index_throttle(c.packet_size_index, PACKET_INDEX_MAX);
        c.window_size = parch_window_throttle(c.window_size, WINDOW_MAX);
        c.throughput_index = parch_throughput_index_throttle(c.throughput_index, THROUGHPUT_INDEX_MAX);

        // Forward the call request to Y
        parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_CALL_REQUEST);
        parch_msg_set_service(y_msg, parch_msg_service(msg));
        parch_msg_set_packet (y_msg, c.packet_size_index);
        parch_msg_set_window (y_msg, c.window_size);
        parch_msg_set_throughput (y_msg, c.throughput_index);
        connection_msg_send (&connection_store, c.y_key, &y_msg);

        // Set the state to X CALL
        c.state = state_x_call_request;
    }
    return c;
}
static channel_t
do_x_clear_confirmation (channel_t c, diagnostic_t d) {
    // Y originally request a clear.  It was forwarded to X.  X has responded.
    // Forward the confirmation to Y
     parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_CLEAR_CONFIRMATION);
     parch_msg_set_diagnostic(y_msg, d);
     connection_msg_send (&connection_store, c.y_key, &y_msg);

    // Clear this connection

    c.state = state_ready;
    return c;
}

static channel_t
do_x_clear_request (channel_t c, uint8_t diagnostic) {
    // Forward the clear request on to Y
    parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_diagnostic(y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);
    // Change state
    c.state = state_x_clear_request;
    return c;
}

static channel_t
do_x_data (channel_t c, uint16_t seq, zframe_t *data)
{
    // We received a data packet from X. Validate it and forward it to Y.
    if (zframe_size(data) == 0) {
        return do_reset(c, d_data_packet_too_small);
    }
    else if (zframe_size(data) > parch_packet_bytes(c.packet_size_index)) {
        return do_reset(c, d_data_packet_too_large);
    }
    else if (seq != c.flow.x_send_sequence) {
        return do_reset(c, d_data_packet_out_of_order);
    }
    else if (!flow_sequence_in_range(seq, c.flow.y_lower_window_edge, c.flow.window_size)) {
        return do_reset(c, d_data_packet_not_in_window);
    }
    else {
        parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_DATA);
        parch_msg_set_sequence(y_msg, seq);
        parch_msg_set_data(y_msg, zframe_dup(data));
        connection_msg_send (&connection_store, c.y_key, &y_msg);

        c.flow.x_send_sequence ++;
        return c;
    }
    abort ();
}

static channel_t
do_x_disconnect (channel_t c) {
    // Inform the peer we're going down
    parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_diagnostic (y_msg, diagnostic);
    connection_msg_send (&connection_store, c.y_key, &y_msg);

    // Shut down X
    connection_disconnect(&connection_store, c.x_key);

    // Clear this connection
    c.state = state_ready;
    return c;
}
static channel_t
do_x_reset (channel_t c, diagnostic_t d) {
    // Forward reset request to Y
     parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_RESET_REQUEST);
     parch_msg_set_diagnostic(y_msg, d);
     connection_msg_send (&connection_store, c.y_key, &y_msg);

     // Change state
     c.state = state_x_reset_request;
     return c;
}

static channel_t
do_x_reset_confirmation (channel_t c, diagnostic_t d) {
    // Received a reset confirmation from X
    // Forward to Y
     parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_RESET_CONFIRMATION);
     parch_msg_set_diagnostic(y_msg, d);
     connection_msg_send (&connection_store, c.y_key, &y_msg);

     // Change state
     c.state = state_data_transfer;

     // reset flow control
     c.flow = flow_reset(c.flow);
     return c;
}

static channel_t
do_x_rnr (channel_t c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_RNR);
  parch_msg_set_sequence(y_msg, seq);
  connection_msg_send (&connection_store, c.y_key, &y_msg);

  c.flow.x_lower_window_edge = seq;
  return c;
}

static channel_t
do_x_rr (channel_t c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_x_reset (c, d_window_edge_out_of_range);
  }
  parch_msg_t *y_msg = parch_msg_new(PARCH_MSG_RR);
  parch_msg_set_sequence(y_msg, seq);
  connection_msg_send (&connection_store, c.y_key, &y_msg);

  c.flow.x_lower_window_edge = seq;
  return c;
}


/////////////////////////
static channel_t
do_y_call_collision (channel_t c) {
    c.state = state_call_collision;
    return c;
}

static channel_t
do_y_call_accepted (channel_t c, parch_msg_t *msg) {
    // Validate the facilities requests
    if (!parch_packet_index_validate (c.packet_size_index)) {
        c = do_clear(c);
    }
    else if (!parch_window_validate(c.window_size)) {
        c = do_clear(c);
    }
    else if (!parch_throughput_index_validate(c.throughput_index)) {
        c = do_clear(c);
    }
    else {
        // Throttle the facilities requests
        // FIXME: we should be throttling these with configuration options
        c.packet_size_index = parch_packet_index_throttle(c.packet_size_index, PACKET_INDEX_MAX);
        c.window_size = parch_window_throttle(c.window_size, WINDOW_MAX);
        c.throughput_index = parch_throughput_index_throttle(c.throughput_index, THROUGHPUT_INDEX_MAX);

        // Forward the call accepted to X
        parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_CALL_ACCEPTED);
        // parch_msg_set_service(x_msg, strdup(parch_msg_service(msg)));
        parch_msg_set_packet (x_msg, c.packet_size_index);
        parch_msg_set_window (x_msg, c.window_size);
        parch_msg_set_throughput (x_msg, c.throughput_index);
        connection_msg_send (&connection_store, c.x_key, &x_msg);

        // Set the state to X CALL
        c.state = state_data_transfer;
    }
    return c;
}
static channel_t
do_y_clear_confirmation (channel_t c, diagnostic_t d) {
    // Y originally request a clear.  It was forwarded to X.  X has responded.
    // Forward the confirmation to Y
     parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_CLEAR_CONFIRMATION);
     parch_msg_set_diagnostic(x_msg, d);
     connection_msg_send (&connection_store, c.x_key, &x_msg);

    // Clear this connection
    c.state = state_ready;
    return c;
}

static channel_t
do_y_clear_request (channel_t c, uint8_t diagnostic) {
    // Forward the clear request on to Y
    parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_diagnostic(x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);
    // Change state
    c.state = state_x_clear_request;
    return c;
}

static channel_t
do_y_data (channel_t c, uint16_t seq, zframe_t *data)
{
    // We received a data packet from X. Validate it and forward it to Y.
    if (zframe_size(data) == 0) {
        WARN("y data packet #%d too small", seq);
        return do_reset(c, d_data_packet_too_small);
    }
    else if (zframe_size(data) > parch_packet_bytes(c.packet_size_index)) {
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
        parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_DATA);
        parch_msg_set_sequence(x_msg, seq);
        parch_msg_set_data(x_msg, zframe_dup(data));
        connection_msg_send (&connection_store, c.x_key, &x_msg);

        c.flow.y_send_sequence ++;
        return c;
    }
    abort ();
}

static channel_t
do_y_disconnect (channel_t c) {
    // Inform the peer we're going down
    parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_diagnostic (x_msg, diagnostic);
    connection_msg_send (&connection_store, c.x_key, &x_msg);

    // Shut down X
    connection_disconnect(&connection_store, c.x_key);

    // Clear this connection
    c.state = state_ready;
    return c;
}
static channel_t
do_y_reset (channel_t c, diagnostic_t d) {
    // Forward reset request to Y
     parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_RESET_REQUEST);
     parch_msg_set_diagnostic(x_msg, d);
     connection_msg_send (&connection_store, c.x_key, &x_msg);

     // Change state
     c.state = state_x_reset_request;
     return c;
}

static channel_t
do_y_reset_confirmation (channel_t c, diagnostic_t d) {
    // Received a reset confirmation from X
    // Forward to Y
     parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_RESET_CONFIRMATION);
     parch_msg_set_diagnostic(x_msg, d);
     connection_msg_send (&connection_store, c.x_key, &x_msg);

     // Change state
     c.state = state_data_transfer;

     // reset flow control
     c.flow = flow_reset(c.flow);
     return c;
}

static channel_t
do_y_rnr (channel_t c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_RNR);
  parch_msg_set_sequence(x_msg, seq);
  connection_msg_send (&connection_store, c.x_key, &x_msg);

  c.flow.x_lower_window_edge = seq;
  return c;
}

static channel_t
do_y_rr (channel_t c, uint16_t seq) {
  // The new window has to overlap or be immediately above
  if (!flow_sequence_in_range(seq, c.flow.x_lower_window_edge, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  // The new window has to contain the next message to be sent from the peer.
  else if (!flow_sequence_in_range(c.flow.y_send_sequence, seq, c.flow.window_size + 1)) {
      return do_y_reset (c, d_window_edge_out_of_range);
  }
  parch_msg_t *x_msg = parch_msg_new(PARCH_MSG_RR);
  parch_msg_set_sequence(x_msg, seq);
  connection_msg_send (&connection_store, c.x_key, &x_msg);

  c.flow.x_lower_window_edge = seq;
  return c;

}

//////////////////////////


channel_t
channel_dispatch (channel_t c, const parch_msg_t *msg) {
    assert(c.x_key);
    assert(c.y_key);
    state_t state_orig = c.state;

    char *key = zframe_strhex(parch_msg_address(msg));
    bool is_y = strcmp(key, c.y_key) == 0;

    action_t a;
    if (is_y)
        a = y_action_table[c.state][parch_msg_id(msg)];
    else
        a = x_action_table[c.state][parch_msg_id(msg)];
    INFO("%s dispatching %s in %s", dname(key), action_names[a], state_names[c.state]);
    free (key);
    key = NULL;
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
            c = do_x_call_request (c, msg);
            break;
        case a_x_call_accepted:
            // Should never happen, since X always connects before Y
            abort ();
            break;
        case a_x_call_collision:
            c = do_x_call_collision (c);
            break;
        case a_x_clear_request:
            c = do_x_clear_request (c, parch_msg_diagnostic(msg));
            break;
        case a_x_clear_confirmation:
            c = do_x_clear_confirmation (c, parch_msg_diagnostic (msg));
            break;
        case a_x_data:
            c = do_x_data (c, parch_msg_sequence(msg), parch_msg_data(msg));
            break;
        case a_x_rr:
            c = do_x_rr (c, parch_msg_sequence(msg));
            break;
        case a_x_rnr:
            c = do_x_rnr (c, parch_msg_sequence(msg));
            break;
        case a_x_reset:
            c = do_x_reset (c, parch_msg_diagnostic(msg));
            break;
        case a_x_reset_confirmation:
            c = do_x_reset_confirmation(c, parch_msg_diagnostic(msg));
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
            c = do_y_call_accepted (c, msg);
            break;
        case a_y_call_collision:
            c = do_y_call_collision (c);
            break;
        case a_y_clear_request:
            c = do_y_clear_request (c, parch_msg_diagnostic(msg));
            break;
        case a_y_clear_confirmation:
            c = do_y_clear_confirmation (c, parch_msg_diagnostic (msg));
            break;
        case a_y_data:
            c = do_y_data (c, parch_msg_sequence(msg), parch_msg_data(msg));
            break;
        case a_y_rr:
            c = do_y_rr (c, parch_msg_sequence(msg));
            break;
        case a_y_rnr:
            c = do_y_rnr (c, parch_msg_sequence(msg));
            break;
        case a_y_reset:
            c = do_y_reset (c, parch_msg_diagnostic(msg));
            break;
        case a_y_reset_confirmation:
            c = do_y_reset_confirmation(c, parch_msg_diagnostic(msg));
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
        INFO("channel %s/%s changing to %s", dname(c.x_key), dname(c.y_key), state_names[c.state]);
    return c;
}
