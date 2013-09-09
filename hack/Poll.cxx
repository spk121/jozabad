#include "Poll.hxx"
#include "Msg.hxx"
#include "Peer.hxx"
#include "process.hxx"
#include "../libjoza/joza_msg.h"

void *g_sock;
int receive(zloop_t* loop, zmq_pollitem_t* item, void* arg);

Poll::Poll()
{
    _socket = NULL;
    _context = NULL;
    _loop = NULL;
    _poll_input.socket = _socket;
    _poll_input.fd = 0;
    _poll_input.events = 0;
    _poll_input.revents = 0;
}

void Poll::initialize(bool verbose, string endpoint)
{
    // FIXME: Simplify this paranoid 'arrow' anti-pattern?
    _context = zctx_new();
    if (_context != NULL) {
        _socket = zsocket_new (_context, ZMQ_ROUTER);
        zsocket_bind(_socket, endpoint.c_str());
        if (_socket != NULL) {
            _loop = zloop_new ();
            if (_loop != NULL) {
                zloop_set_verbose (_loop, verbose);
                _poll_input.socket = _socket;
                _poll_input.fd = 0;
                _poll_input.events = ZMQ_POLLIN;
                _poll_input.revents = 0;
                int ret = zloop_poller (_loop, &_poll_input, &receive, NULL);
                if (ret != 0) {
                    zloop_destroy(&_loop);
                    zsocket_destroy(_context, _socket);
                    _socket = NULL;
                    zctx_destroy(&_context);
                    throw 10;
                }
                else {
                    printf ("binding ROUTER socket to %s\n", endpoint.c_str());

                    // FIXME: leaking this socket to a global.
                    // There's got to be a better way.
                    g_sock = _socket;
                }
            }
            else { 
                zsocket_destroy(_context, _socket);
                _socket = NULL;
                zctx_destroy(&_context);
                throw 11;
            }
        }
        else {
            _socket = NULL;
            zctx_destroy(&_context);
            throw 12;
        }
    }
    else {
        throw 13;
    }
}

Poll::~Poll()
{
    zloop_destroy(&_loop);
    zsocket_destroy(_context, _socket);
    _socket = NULL;
    zctx_destroy(&_context);
}

void Poll::start() {
    // Takes over control of the thread
    printf("starting main loop\n");
    zloop_start(_loop);
}

// Tries to handle the given message. 
// Returns 0 if not processed
//         1 if partially processed
//         2 if processed
int Poll::dispatch (Msg& msg)
{
    int ret = 0;
    return ret;
}

// This 'receive' callback is where all the processing really begins.
int receive(zloop_t* loop, zmq_pollitem_t* item, void* arg)
{
    int ret = 0;

    printf("in receive(%p, %p, %p)\n", loop, item, arg);

    if (item->revents & ZMQ_POLLIN) {
        Msg msg(g_sock);
        ret = dispatch(msg);
    }
    return ret;
}

