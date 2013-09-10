#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Msg.hxx"

using namespace std;

enum class State {
    ready,
    x_call_request,
    y_call_request,
    data_transfer,
    call_collision,
    x_clear_request,
    y_clear_request,
    x_reset_request,
    y_reset_request,

    state_last = y_reset_request
} state_t;

struct Subchannel {
    string _key;
    uint16_t _ps = 0;
    uint16_t _pr = 0;
    Subchannel() = default;
};

class Channel {
public:
    Channel(string xkey, string ykey, uint16_t W, uint16_t P, uint32_t T)
    {
        _state = {State::ready};
        _x._key.assign(xkey);
        _y._key.assign(ykey);
        _windowSize = {W};
        _maxPacketSize = {P};
        _throughput = {T};
    }
    ~Channel() {};

    State _state;
private:
    Subchannel _x;
    Subchannel _y;
    uint16_t _windowSize;
    uint16_t _maxPacketSize;
    uint32_t _throughput;
};

typedef vector<shared_ptr<Channel>> vector_shared_ptr_channel_t;
typedef map<string, shared_ptr<Channel>> map_shared_ptr_channel_t;

class ChannelStore {
public:
    ChannelStore() : _store(), _xmap(), _ymap() {};
    int dispatch(Msg& msg);
    int do_call_request_step_2(Msg& msg, string& y_key);

    pair<int, shared_ptr<Channel>> find (string key);
    void insert (string x, string y, uint16_t W, uint16_t P, uint32_t T);
    size_t count (string& key);
private:
    vector_shared_ptr_channel_t _store;
    map_shared_ptr_channel_t _xmap, _ymap;
};


