#include "Channel.hxx"
#include "Peer.hxx"
#include "../libjoza/joza_msg.h"

#include <memory>
#include <map>
#include <utility>
#include <algorithm>
#include "action.hxx"

static diagnostic_t state_to_diagnostic(State s);
extern void *g_sock;
extern PeerStore peers;

// Tries to handle a message on this channel
// return 0 if not handled
//        1 if partially handled
//        2 if handled
int Channel::dispatch(Msg& msg, bool is_y)
{
    action_t a = find_action(_state, msg.id(), is_y);

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

			if (_state == State::data_transfer)
				d = d_packet_type_invalid_for_s4;
			else if (_state == State::x_reset_request)
				d = d_packet_type_invalid_for_s8;
			else if (_state == State::y_reset_request)
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
			if (_state == State::ready)
				d = d_packet_type_invalid_for_s1;
			else if (_state == State::x_call_request)
				d = d_packet_type_invalid_for_s2;
			else if (_state == State::y_call_request)
				d = d_packet_type_invalid_for_s3;
			else if (_state == State::data_transfer)
				d = d_packet_type_invalid_for_s4;
			else if (_state == State::call_collision)
				d = d_packet_type_invalid_for_s5;
			else if (_state == State::x_clear_request)
				d = d_packet_type_invalid_for_s6;
			else if (_state == State::y_clear_request)
				d = d_packet_type_invalid_for_s7;
			else if (_state == State::x_reset_request)
				d = d_packet_type_invalid_for_s8;
			else if (_state == State::y_reset_request)
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
	if (_state != State::orig)
		INFO("channel %s/%s changing to %s", find_dname(x_key), find_dname(y_key), name(_state));
    
}

// Tries to handle the given message by passing it off its associated channel.
// return 0 if not handled
//        1 if partially handled
//        2 if handled
int ChannelStore::dispatch(Msg& msg)
{
    int ret = 0;

    // This shouldn't happen.  joza_msg.c has the responsibility to
    // prevent it.
    assert (msg.id()) >= JOZA_MSG_COUNT);
        
    // Note that only the CONNECT_REQUEST messages have an explicit
    // address to the peer.  For all other messages from peers
    // connected into channels, we use channel state to help dispatch
    // them.

    if (count(msg._hex) != 0) {
        auto ch = find(msg._hex);

        // OK.  This is a message from a known peer on a known channel.
        // We can have the channel dispatch it.
        bool is_y = ch.first == 1;
        Channel channel = ch.second;

        ret = channel.dispatch(msg, is_y);
    }
    /* Else, this isn't from a connected channel, so give up. */

    return ret;
}

int ChannelStore::do_call_request_step_2(Msg& msg, string& ykey)
{
    int ret = 0;
    // do_call_request_part_one should have guaranteed that this
    // - this message is valid
    // - msg comes from a known peer
    // - msg is calling a known peer

    if (count(msg._hex) != 0) {
        auto ch = find(msg._hex);
        // The message sender is already on a call.  We need to get the call's state 
        // to send a proper diagnostic.
        joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                      c_local_procedure_error,
                                      state_to_diagnostic(ch.second->_state));
        ret = 2;
    }
    else {
        // Create the call
        insert(msg._hex, ykey, msg.window(), msg.packet(), msg.throughput());

        // FIXME: here is where the broker would throttle window/packet/throughput
        
        joza_msg_send_addr_call_request(g_sock,
                                        peers.get(ykey)._address._pzframe,
                                        (char *)msg.calling_address().c_str(),
                                        (char *)msg.called_address().c_str(),
                                        msg.packet(),
                                        msg.window(),
                                        msg.throughput(),
                                        joza_msg_data(msg._msg));
        ret = 2;
    }
    return ret;
}

pair<int, shared_ptr<Channel>> ChannelStore::find (string key)
{
    if (_xmap.count(key))
        return make_pair<int, map_shared_ptr_channel_t::mapped_type&>(0, _xmap[key]);
    else if (_ymap.count(key))
        return make_pair<int, map_shared_ptr_channel_t::mapped_type&>(1, _ymap[key]);
    else
        abort();
}

void ChannelStore::insert(string x, string y, uint16_t W, uint16_t P, uint32_t T)
{
    _store.emplace_back(make_shared<Channel>(x,y,W,P,T));
    _xmap[x] = _store.back();
    _ymap[y] = _store.back();
}

size_t ChannelStore::count(string& key)
{
    return (_xmap.count(key) + _ymap.count(key));
}

static diagnostic_t state_to_diagnostic(State s) {
    if (s == State::ready)
        return d_packet_type_invalid_for_s1;
    else if (s == State::x_call_request)
        return d_packet_type_invalid_for_s2;
    else if (s == State::y_call_request)
        return d_packet_type_invalid_for_s3;
    else if (s == State::data_transfer)
        return d_packet_type_invalid_for_s4;
    else if (s == State::call_collision)
        return d_packet_type_invalid_for_s5;
    else if (s == State::x_clear_request)
        return d_packet_type_invalid_for_s6;
    else if (s == State::y_clear_request)
        return d_packet_type_invalid_for_s7;
    else if (s == State::x_reset_request)
        return d_packet_type_invalid_for_s8;
    else if (s == State::y_reset_request)
        return d_packet_type_invalid_for_s9;
    return d_packet_type_invalid;
}
