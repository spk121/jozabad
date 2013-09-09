#pragma once
#include <czmq.h>
#include <string>
#include <czmq.h>

using namespace std;

// A ZFrame is meant to capture a zframe_t* and release it appropriately
class ZFrame {
public:
    ZFrame(const zframe_t *pf)
    {
        _pzframe = zframe_dup((zframe_t *)pf);
        update_hex();
    }

    ZFrame(const ZFrame& rf)
    {
        _pzframe = zframe_dup(rf._pzframe);
        update_hex();
    }

    ZFrame(const void *data, size_t size)
    {
        _pzframe = zframe_new(data, size);
        update_hex();
    }
    
    ~ZFrame()
    {
        zframe_destroy (&_pzframe);
    }
    
    size_t size()
    {
        return zframe_size(_pzframe);
    }

    zframe_t *_pzframe;
    string _hex;
    void update_hex() 
    {
        char *h = zframe_strhex(_pzframe);
        _hex = h;
        free(h);
    }
    
};

#if 0
class Worker {

private:
Zframe *zmqAddr;
string x121Address;
IoDir ioDir;

public:
int id;
zframe_t *zmq_address; 
char x121_address[X121_ADDRESS_LENGTH]; 
direction_t direction;
uint8_t busy;

Connection(zframe_t *address, const char *name, direction_t d);
Connection(zframe_t const* address, const char *name, direction_t d);
~Connection();
}
#endif
