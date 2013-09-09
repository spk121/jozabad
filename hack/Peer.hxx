#pragma once
#include <czmq.h>
#include <string>
#include <map>

#include "Msg.hxx"
#include "ZFrame.hxx"

#define MAX_PEER_COUNT (4000)

using namespace std;

typedef int IoDir;

class Peer {
    static int _count;

public:
    Peer(const zframe_t *pzf, string name, IoDir dir);
    Peer(const Peer& p)
        : _address(p._address), _name(p._name), _ioDir(p._ioDir)
    {
        _refcount = p._refcount;
    }


    ZFrame _address;
    int _id;
    string _name;
    IoDir _ioDir;
    int _refcount;
};

class PeerStore {
 public:
    void insert(const zframe_t *pzf, string name, IoDir dir);
    size_t count(string key);
    size_t count_by_address(string addr);
    Peer& get(string key);
    Peer& get_by_name(string addr);
    int dispatch(Msg& msg);
    int do_connect(Msg& msg);
    int do_disconnect(Msg& msg);
    void disconnect(string key); 

    pair<int, string> do_call_request_step_1(Msg& msg);

 private:
    map<string, Peer> _map;
};
