#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Msg.hxx"

using namespace std;

enum class State {
    ready, s2, s3
        };

class Subchannel {
public:
    string _key;
    uint16_t _ps;
    uint16_t _pr;
    Subchannel(string key) : _key(key), _ps(0), _pr(0) {};
};

class Channel {
public:
    Channel(string xkey, string ykey, uint16_t W, uint16_t P, uint32_t T)
        :  _state(State::ready), _x(xkey), _y(ykey)
    {
        _windowSize = W;
        _maxPacketSize = P;
        _throughput = T;
    }
    ~Channel() {};
private:
    State _state;
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

    pair<int, shared_ptr<Channel>> find (string key);
    void insert (string x, string y, uint16_t W, uint16_t P, uint32_t T);
    size_t count (string key);
private:
    vector_shared_ptr_channel_t _store;
    map_shared_ptr_channel_t _xmap, _ymap;
};


