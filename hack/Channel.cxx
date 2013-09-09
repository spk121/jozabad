#include "Channel.hxx"

#include <memory>
#include <map>
#include <utility>
#include <algorithm>

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

int ChannelStore::do_call_request_step_2(Msg& msg, string ykey)
{
    // If this message
    // - is a call request
    // - is from a known peer
    // - that peer is not in a connection
    // - the message has a called_address that is of a known peer
    // - and that known peer is not in a connection
    // - and has valid facilities values
    // then, connect the callee and caller
    return 0;
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

size_t ChannelStore::count(string key)
{
    return (_xmap.count(key) + _ymap.count(key));
}

