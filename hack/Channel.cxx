#include "Channel.hxx"
#include "Peer.hxx"
#include "../libjoza/joza_msg.h"

#include <memory>
#include <map>
#include <utility>
#include <algorithm>

static diagnostic_t state_to_diagnostic(State s);
extern void *g_sock;
extern PeerStore peers;

// Tries to handle the given message. 
// return 0 if not handled
//        1 if partially handled
//        2 if handled
int ChannelStore::dispatch(Msg& msg)
{
    int ret = 0;
    // Else, if this message
    // - is one of the regular messages
    // - is from a known peer
    // - that peer is in a connection
    // - the connected peer is valid
    // then figure out what needs to be validated.
    // Check if
    // - the message has valid addresses
    // - the message has valid faciltities requests
    // - the message has valid ps and pr values
    // then, use the state machine and action table to process the message

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
