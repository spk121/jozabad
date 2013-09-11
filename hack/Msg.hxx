#pragma once

#include "../libjoza/joza_msg.h"
#include "../libjoza/joza_lib.h"
#include <czmq.h>
#include <zmq.h>
#include <string>
#include "ZFrame.hxx"

using namespace std;

// This little class just exists to make sure that a joza_msg_t is
// properly freed.
class Msg {
public:
    Msg(int id) {
        _msg = joza_msg_new(id);
        update_hex();
    }

    Msg(void *socket) {
        _msg = joza_msg_recv (socket);
        update_hex();
    }

    Msg(Msg&& msg) {
        _msg = joza_msg_dup (msg._msg);
        update_hex();
    }

    ~Msg()
    {
        if (_msg)
            joza_msg_destroy (&_msg);
    }

    const zframe_t* address() 
    {
        return joza_msg_address(_msg);
    }

    string key()
    {
        return _hex;
    }

    uint8_t ioDir()
    {
        return joza_msg_directionality(_msg);
    }

    bool is_a(int id) 
    {
        return (id == joza_msg_id(_msg));
    }

    int id()
    {
        return joza_msg_id(_msg);
    }

    bool is_valid()
    {
        return (_msg != NULL);
    }

    string called_address() 
    {
        return joza_msg_called_address(_msg);
    }

	string calling_address() 
    {
        return joza_msg_calling_address(_msg);
    }
    
    int send_and_destroy(void *socket)
    {
        return joza_msg_send (&_msg, socket);
        update_hex();
    }

    void take(joza_msg_t *msg)
    {
        _msg = msg;
        update_hex();
    }

	packet_t packet() 
	{
		return (packet_t) _msg->packet;
	}

	uint16_t window()
	{
		return _msg->window;
	}

	throughput_t throughput()
	{
		return (throughput_t) _msg->throughput;
	}

	size_t data_size()
	{
		return zframe_size(_msg->data);
	}

    zframe_t* data()
    {
        return _msg->data;
    }

    void update_hex() 
    {
        char *h = zframe_strhex(joza_msg_address(_msg));
        _hex.assign(h);
        free(h);
    }
    joza_msg_t *_msg;
    string _hex;
};
