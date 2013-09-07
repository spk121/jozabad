#include <string>
#include <memory>
#include "connections.h"
#include "../libjoza/joza_msg.h"
#include "action.h"
#include "dname.h"
#include "lib.h"
#include "log.h"
#include "poll.h"

connection_store_t  connection_store;
channel_store_t     channel_store;
extern throughput_t opt_throughput;
extern int          opt_max_connections;
extern void*        sock;

// The DNAMES are used in log messages, because hex addresses are hard
// to keep straight.
static int iter = 0;

Connection::Connection(zframe_t *a, const char *n, direction_t d) {
	assert(x121_validate(n));
	zmq_address = zframe_dup(a);
	id = iter++;
	strcpy(x121_address, n);
	direction = d;
}

Connection::Connection(zframe_t const* a, char const* n, direction_t d) {
	assert(x121_validate(n));
	zmq_address = zframe_dup((zframe_t *) a);
	id = iter++;
	strcpy(x121_address, n);
	direction = d;
}

Connection::~Connection() {
	if (zmq_address) {
		zframe_destroy(&zmq_address);
		zmq_address = NULL;
	}
}

////////////////////////////////////////////////////////////////
// CONNECTION STORE
////////////////////////////////////////////////////////////////

const char*
	find_dname(const char *key) {
		if (connection_store.count(key) == 0)
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

void
	add_connection(const char *key, zframe_t const* zmq_addr,
	char const* x121_addr, direction_t d)
{
	assert (connection_store.count(key) == 0);

	// Validate the connection message
	if (!x121_validate (x121_addr)) {
		INFO("invalid connection request - %s (%s)",
			x121_addr,
			direction_name(d));
		joza_msg_send_addr_diagnostic(sock,  zmq_addr,
			c_local_procedure_error,
			d_invalid_called_address);
	}
	else if (!direction_validate(d)) {
		INFO("invalid connection request - %s (%s)",
			x121_addr,
			direction_name(d));
		joza_msg_send_addr_diagnostic(sock,  zmq_addr,
			c_invalid_facility_request,
			d_facility_parameter_not_allowed);
	}
	else if (connection_store.size() > opt_max_connections) {
		INFO("invalid connection request - no free connections");
		joza_msg_send_addr_diagnostic(sock,  zmq_addr,
			c_network_congestion,
			d_unspecified);
	}
	else {
		Connection* C = new Connection(zmq_addr, x121_addr, d);
		connection_store.insert(pair<string, Connection *>(key, C));
		INFO("new connection %s - %s (%s)",
			find_dname(key),
			x121_addr,
			direction_name(d));
		NOTE("connection count is %d", connection_store.size());

		joza_msg_send_addr_connect_indication(sock, zmq_addr);
	}
}

Connection*
	find_worker_by_x121_address(const char* addr) {
		TRACE("find_worker_by_x121_address(name = %p '%s')", addr, addr);
		Connection* connection = NULL;
		for (auto it = connection_store.begin();
			it != connection_store.end();
			++it) {
				if (strcmp(it->second->x121_address, addr) == 0)
					connection = it->second;
		}
		return connection;
}

void
	connection_msg_send(const char *key, joza_msg_t **msg) {
		Connection *w = find_worker(key);
		joza_msg_set_address(*msg, w->zmq_address);
		INFO("sending '%s' to %s", joza_msg_const_command(*msg), dname(w->id));
		joza_msg_send(msg, sock);
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
	connection_dispatch(const char *key, joza_msg_t *msg) {
		// This dispatcher is for messages from already-registered workers
		// that aren't connected to peers.  Thus the only such messages
		// are DISCONNECT and CALL_REQUEST
		bool more = false;
		int id = joza_msg_id(msg);
		if (id ==JOZA_MSG_DISCONNECT) {
			connection_disconnect(key);
		} else if (id == JOZA_MSG_CALL_REQUEST) {
			// Look for a peer key
			Connection* x;
			Connection* y;
			const char *y_addr;

			x = find_worker(key);
			y_addr = joza_msg_called_address(msg);
			y = find_worker_by_x121_address(y_addr);
			if (y == 0) {
				// No available worker found.  Send a clear request.
			} else {
				INFO("adding channel %s/%s", dname(x->id), dname(y->id));
				char *y_key = zframe_strhex(y->zmq_address);
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

void Channel::dispatch(const joza_msg_t *msg) {
	state_t state_orig;
	char* msg_key;
	bool is_y;
	action_t a;

	state_orig = state;
	msg_key = zframe_strhex((zframe_t*) joza_msg_const_address(msg));
	is_y = strcmp(msg_key, y_key) == 0;
	a = find_action(state, joza_msg_const_id(msg), is_y);
	INFO("%s/%s dispatching %s in %s",
		find_dname(x_key),
		find_dname(y_key),
		name(a),
		name(state));
	free(msg_key);
	msg_key = NULL;


	switch (a) {
	case a_unspecified:
		abort();
		break;
	case a_discard:
		break;
	case a_reset:
		// If the action table has responded with a "reset" action, it is because
		// the message was received in the wrong state.
		{
			diagnostic_t d;
			cause_t c;
			if (is_y)
				c = c_remote_procedure_error;
			else
				c = c_local_procedure_error;

			if (state == state_data_transfer)
				d = d_packet_type_invalid_for_s4;
			else if (state == state_x_reset_request)
				d = d_packet_type_invalid_for_s8;
			else if (state == state_y_reset_request)
				d = d_packet_type_invalid_for_s9;
			else
				abort();
			do_reset(c,d);
		}
		break;
	case a_clear:
		// If the action table has responded with a "clear" action, it is because
		// the message was received in the wrong state.
		{
			diagnostic_t d;
			cause_t c;
			if (is_y)
				c = c_remote_procedure_error;
			else
				c = c_local_procedure_error;
			if (state == state_ready)
				d = d_packet_type_invalid_for_s1;
			else if (state == state_x_call_request)
				d = d_packet_type_invalid_for_s2;
			else if (state == state_y_call_request)
				d = d_packet_type_invalid_for_s3;
			else if (state == state_data_transfer)
				d = d_packet_type_invalid_for_s4;
			else if (state == state_call_collision)
				d = d_packet_type_invalid_for_s5;
			else if (state == state_x_clear_request)
				d = d_packet_type_invalid_for_s6;
			else if (state == state_y_clear_request)
				d = d_packet_type_invalid_for_s7;
			else if (state == state_x_reset_request)
				d = d_packet_type_invalid_for_s8;
			else if (state == state_y_reset_request)
				d = d_packet_type_invalid_for_s9;
			else
				abort();
			do_clear(c, d);
		}
		break;
	case a_disconnect:
		// The action table never declares a bi-directional abort
		abort ();
		break;
	case a_x_connect:
		// Connection happens before the channel exists
		abort();
		break;
	case a_x_disconnect:
		do_x_disconnect((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	case a_x_call_request:
		do_x_call_request(joza_msg_const_called_address(msg),
			(packet_t) joza_msg_const_packet(msg),
			joza_msg_const_window(msg),
			(throughput_t) joza_msg_const_throughput(msg));
		break;
	case a_x_call_accepted:
		// Should never happen, since X always connects before Y
		abort();
		break;
	case a_x_call_collision:
		do_x_call_collision();
		break;
	case a_x_clear_request:
		do_x_clear_request((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	case a_x_clear_confirmation:
		do_x_clear_confirmation((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	case a_x_data:
		do_x_data(joza_msg_const_ps(msg),
			joza_msg_const_pr(msg),
			joza_msg_const_data(msg));
		break;
	case a_x_rr:
		do_x_rr(joza_msg_const_pr(msg));
		break;
	case a_x_rnr:
		do_x_rnr(joza_msg_const_pr(msg));
		break;
	case a_x_reset:
		do_x_reset((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	case a_x_reset_confirmation:
		do_x_reset_confirmation((diagnostic_t) joza_msg_const_diagnostic(msg));
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
		do_y_clear_request((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	case a_y_clear_confirmation:
		do_y_clear_confirmation((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	case a_y_data:
		do_y_data(joza_msg_const_ps(msg),
			joza_msg_const_pr(msg), 
			joza_msg_const_data(msg));
		break;
	case a_y_rr:
		do_y_rr(joza_msg_const_pr(msg));
		break;
	case a_y_rnr:
		do_y_rnr(joza_msg_const_pr(msg));
		break;
	case a_y_reset:
		do_y_reset((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	case a_y_reset_confirmation:
		do_y_reset_confirmation((diagnostic_t) joza_msg_const_diagnostic(msg));
		break;
	}
	if (state != state_orig)
		INFO("channel %s/%s changing to %s", find_dname(x_key), find_dname(y_key), name(state));
}

void Channel::do_clear(cause_t c, diagnostic_t d) {
	joza_msg_t *x_msg = NULL;
	joza_msg_t *y_msg = NULL;

	x_msg = joza_msg_new(JOZA_MSG_CLEAR_REQUEST);
	joza_msg_set_cause(x_msg, c);
	joza_msg_set_diagnostic(x_msg, d);
	connection_msg_send(x_key, &x_msg);

	y_msg = joza_msg_new(JOZA_MSG_CLEAR_REQUEST);
	if (c == c_remote_procedure_error)
		joza_msg_set_cause(x_msg, c_local_procedure_error);
	else if (c == c_local_procedure_error)
		joza_msg_set_cause(x_msg, c_remote_procedure_error);
	else
		joza_msg_set_cause(x_msg, c);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);

	state = state_y_clear_request;
}

void Channel::do_reset(cause_t c, diagnostic_t d) {
	joza_msg_t* x_msg;
	joza_msg_t* y_msg;

	x_msg = joza_msg_new(JOZA_MSG_RESET_REQUEST);
	joza_msg_set_cause(x_msg, c);
	joza_msg_set_diagnostic(x_msg, d);
	connection_msg_send(x_key, &x_msg);

	y_msg = joza_msg_new(JOZA_MSG_RESET_REQUEST);
	if (c == c_remote_procedure_error)
		joza_msg_set_cause(x_msg, c_local_procedure_error);
	else if (c == c_local_procedure_error)
		joza_msg_set_cause(x_msg, c_remote_procedure_error);
	else
		joza_msg_set_cause(x_msg, c);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);
	flow = reset(flow);
	state = state_y_reset_request;
}

void Channel::do_disconnect(diagnostic_t d) {
	joza_msg_t* x_msg;
	joza_msg_t* y_msg;

	x_msg = joza_msg_new(JOZA_MSG_DISCONNECT_INDICATION);
	joza_msg_set_diagnostic(x_msg, d);
	connection_msg_send(x_key, &x_msg);

	y_msg = joza_msg_new(JOZA_MSG_DISCONNECT_INDICATION);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);

	connection_disconnect(x_key);
	connection_disconnect(y_key);

	state = state_ready;
}

void Channel::do_x_disconnect(diagnostic_t d) {
	// Inform the peer we're going down
	joza_msg_t* y_msg;
	y_msg = joza_msg_new(JOZA_MSG_CLEAR_REQUEST);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);

	// Shut down X
	connection_disconnect(x_key);

	// Clear this connection
	state = state_ready;
}

void Channel::do_x_call_request(const char *called_address,
								packet_t p, uint16_t w, throughput_t t)
{
	if (!packet_validate(p)) {
		do_clear(c_invalid_facility_request, d_facility_parameter_not_allowed);
	} else if (!window_validate(w)) {
		do_clear(c_invalid_facility_request, d_facility_parameter_not_allowed);
	} else if (!throughput_validate(t)) {
		do_clear(c_invalid_facility_request, d_facility_parameter_not_allowed);
	} else {
		// CALL REQUEST NEGOTIATION -- STEP 1
		// The call request from X is throttled by the Broker's limitations
		// FIXME: we should be throttling these with configuration options
		t = throughput_throttle(t, opt_throughput);

		// Forward the call request to Y
		joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_CALL_REQUEST);
		joza_msg_set_called_address(y_msg, called_address);
		joza_msg_set_packet(y_msg, p);
		joza_msg_set_window(y_msg, w);
		joza_msg_set_throughput(y_msg, t);
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
	joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_CLEAR_REQUEST);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);
	// Change state
	state = state_x_clear_request;
}

void Channel::do_x_clear_confirmation(diagnostic_t d) {
	// Y originally request a clear.  It was forwarded to X.  X has responded.
	// Forward the confirmation to Y
	joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_CLEAR_CONFIRMATION);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);

	// Clear this connection
	state = state_ready;
}

void Channel::do_x_data(uint16_t ps, uint16_t pr, const zframe_t *data) {
	// We received a data packet from X. Validate it and forward it to Y.
	size_t siz = zframe_size((zframe_t *) data);
	if (siz == 0) {
		do_reset(c_local_procedure_error, d_packet_too_short);
	} else if (siz > packet_bytes(packet_size_index)) {
		do_reset(c_local_procedure_error, d_packet_too_long);
	} else if (ps != flow.x_send_sequence) {
		do_reset(c_local_procedure_error, d_invalid_ps);
	} else if (!flow_sequence_in_range(ps, flow.y_lower_window_edge, flow.window_size)) {
		do_reset(c_local_procedure_error, d_invalid_ps);
	} else {
		joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_DATA);
		joza_msg_set_ps(y_msg, ps);
		joza_msg_set_data(y_msg, zframe_dup((zframe_t *) data));
		connection_msg_send(y_key, &y_msg);

		flow.x_send_sequence++;
	}
}

void Channel::do_x_rr(uint16_t seq) {
	// The new window has to overlap or be immediately above
	if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
		do_x_reset(d_invalid_pr);
	}// The new window has to contain the next message to be sent from the peer.
	else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
		do_x_reset(d_invalid_pr);
	}
	joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_RR);
	joza_msg_set_pr(y_msg, seq);
	connection_msg_send(y_key, &y_msg);

	flow.x_lower_window_edge = seq;
}

void Channel::do_x_rnr(uint16_t seq) {
	// The new window has to overlap or be immediately above
	if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
		do_x_reset(d_invalid_pr);
	}// The new window has to contain the next message to be sent from the peer.
	else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
		do_x_reset(d_invalid_pr);
	}
	joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_RNR);
	joza_msg_set_pr(y_msg, seq);
	connection_msg_send(y_key, &y_msg);

	flow.x_lower_window_edge = seq;
}

void Channel::do_x_reset(diagnostic_t d) {
	// Forward reset request to Y
	joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_RESET_REQUEST);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);

	// Change state
	state = state_x_reset_request;
}

void Channel::do_x_reset_confirmation(diagnostic_t d) {
	// Received a reset confirmation from X
	// Forward to Y
	joza_msg_t *y_msg = joza_msg_new(JOZA_MSG_RESET_CONFIRMATION);
	joza_msg_set_diagnostic(y_msg, d);
	connection_msg_send(y_key, &y_msg);

	// Change state
	state = state_data_transfer;

	// reset flow control
	flow = reset(flow);
}

void Channel::do_y_disconnect() {
	// Inform the peer we're going down
	joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_CLEAR_REQUEST);
	joza_msg_set_diagnostic(x_msg, d_invalid_called_address);
	connection_msg_send(x_key, &x_msg);

	// Shut down X
	connection_disconnect(x_key);

	// Clear this connection
	state = state_ready;
}

void Channel::do_y_call_accepted() {
	// Validate the facilities requests
	// Note that 'diagnostic' gets set as a side-effect here
	if (!packet_validate(packet_size_index)) {
		do_clear(c_remote_procedure_error, d_facility_parameter_not_allowed);
	} else if (!window_validate(window_size)) {
		do_clear(c_remote_procedure_error, d_facility_parameter_not_allowed);
	} else if (!throughput_validate(throughput_index)) {
		do_clear(c_remote_procedure_error, d_facility_parameter_not_allowed);
	} else {

		// Forward the call accepted to X
		joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_CALL_ACCEPTED);
		// msg_set_service(x_msg, strdup(msg_service(msg)));
		joza_msg_set_packet(x_msg, packet_size_index);
		joza_msg_set_window(x_msg, window_size);
		joza_msg_set_throughput(x_msg, throughput_index);
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
	joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_CLEAR_REQUEST);
	joza_msg_set_diagnostic(x_msg, d);
	connection_msg_send(x_key, &x_msg);
	// Change state
	state = state_x_clear_request;
}

void Channel::do_y_clear_confirmation(diagnostic_t d) {
	// Y originally request a clear.  It was forwarded to X.  X has responded.
	// Forward the confirmation to Y
	joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_CLEAR_CONFIRMATION);
	joza_msg_set_diagnostic(x_msg, d);
	connection_msg_send(x_key, &x_msg);

	// Clear this connection
	state = state_ready;
}

void Channel::do_y_data(uint16_t seq, uint16_t pr, const zframe_t *data) {
	// We received a data packet from Y. Validate it and forward it to X.
	size_t siz = zframe_size((zframe_t *) data);
	if (siz == 0) {
		WARN("y data packet #%d too small", seq);
		do_reset(c_remote_procedure_error, d_packet_too_short);
	} else if (siz > packet_bytes(packet_size_index)) {
		WARN("y data packet #%d too large", seq);
		do_reset(c_remote_procedure_error, d_packet_too_long);
	} else if (seq != flow.y_send_sequence) {
		WARN("y data packet #%d out of order, expected #%s", seq, flow.y_send_sequence);
		do_reset(c_remote_procedure_error, d_invalid_ps);
	} else if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size)) {
		WARN("y data packet #%d not in window [%d to %d]", seq, flow.x_lower_window_edge, flow.x_lower_window_edge + flow.window_size);
		do_reset(c_remote_procedure_error, d_invalid_ps);
	} else {
		joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_DATA);
		joza_msg_set_pr(x_msg, seq);
		joza_msg_set_data(x_msg, zframe_dup((zframe_t *) data));
		connection_msg_send(x_key, &x_msg);

		flow.y_send_sequence++;
	}
}

void Channel::do_y_rr(uint16_t seq) {
	// The new window has to overlap or be immediately above
	if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
		do_y_reset(d_invalid_pr);
	}// The new window has to contain the next message to be sent from the peer.
	else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
		do_y_reset(d_invalid_pr);
	}
	joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_RR);
	joza_msg_set_pr(x_msg, seq);
	connection_msg_send(x_key, &x_msg);

	flow.x_lower_window_edge = seq;
}

void Channel::do_y_rnr(uint16_t seq) {
	// The new window has to overlap or be immediately above
	if (!flow_sequence_in_range(seq, flow.x_lower_window_edge, flow.window_size + 1)) {
		do_y_reset(d_invalid_pr);
	}// The new window has to contain the next message to be sent from the peer.
	else if (!flow_sequence_in_range(flow.y_send_sequence, seq, flow.window_size + 1)) {
		do_y_reset(d_invalid_pr);
	}
	joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_RNR);
	joza_msg_set_pr(x_msg, seq);
	connection_msg_send(x_key, &x_msg);

	flow.x_lower_window_edge = seq;
}

void Channel::do_y_reset(diagnostic_t d) {
	// Forward reset request to Y
	joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_RESET_REQUEST);
	joza_msg_set_diagnostic(x_msg, d);
	connection_msg_send(x_key, &x_msg);

	// Change state
	state = state_x_reset_request;
}

void Channel::do_y_reset_confirmation(diagnostic_t d) {
	// Received a reset confirmation from X
	// Forward to Y
	joza_msg_t *x_msg = joza_msg_new(JOZA_MSG_RESET_CONFIRMATION);
	joza_msg_set_diagnostic(x_msg, d);
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
