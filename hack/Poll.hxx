#pragma once

#include <string>
#include <czmq.h>
#include <zmq.h>
#include "Peer.hxx"
#include "Msg.hxx"
#include "../libjoza/joza_msg.h"

using namespace std;

class Poll {
public:
    // Initializes the ZMQ context, socket, and loop
    Poll();
    void initialize(bool verbose, string endpoint);
    void start ();
    int dispatch (Msg& msg);
    void *socket() { return _socket; };
    ~Poll();

private:
    void* _socket;
    zctx_t* _context;
    zloop_t* _loop;
    zmq_pollitem_t _poll_input;
};

struct PollStore {
    Poll _poll;
};
