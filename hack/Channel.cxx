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

#if 0
int main()
{
    ChannelStore cs;

    cs.insert("Adam", "Brody", 2, 128, 64000);
    auto x = cs.find("Adam");
    auto y = cs.find("Brody");
    auto z = 100;
    // auto z = cs.find("Charlie");
#if 0
    ChannelVector channels;
    ChannelMap xchannels;
    ChannelMap ychannels;

    add_channel(channels, xchannels, ychannels, "X2", "Y2", 2, 128, 64000);
    channels.emplace_back(make_shared<Channel>("X1","Y1",2,128,64000));
    xchannels.insert(pair<string,shared_ptr<Channel>>("key1", channels.front()));
    xchannels["key2"] = channels.front();
    shared_ptr<Channel> d = xchannels["key1"];
    xchannels.erase("key2");
    d = xchannels["key3"];
    if (xchannels.count("key4"))
        d = xchannels.at("key4");
#endif
    return 0;
}
#endif
