#include "Msg.hxx"
#include "Peer.hxx"
#include <czmq.h>
#include <map>
#include <algorithm>
#include "../libjoza/joza_lib.h"

using namespace std;
extern void *g_sock;
#define MAX_CALL_REQUEST_DATA_SIZE 16

int Peer::_count = 0;

Peer::Peer(const zframe_t *pzf, string name, IoDir dir)
    : _address(pzf), _name(name), _ioDir(dir), _refcount(0)
{
    _id = _count++;
}

size_t PeerStore::count(string key)
{
    return _map.count(key);
}

size_t PeerStore::count_by_name(string addr)
{
    //return std::count(_map.begin(), _map.end(), [addr](pair<string, Peer> x){return x.second._name == addr;});
    for (auto it = _map.begin();
         it != _map.end();
         ++it) {
        if (it->second._name == addr)
            return 1;
    }
    return 0;
}

void PeerStore::disconnect(string key) 
{
    _map.erase(_map.find(key));
}


// Tries to handle the given message. 
// return 0 if not handled
//        1 if partially handled
//        2 if handled
int PeerStore::dispatch(Msg& msg)
{
    int ret = 0;
    // Here we are going to handle messages that modify or query the PeerStore.
    // Really this is just CONNECT and DISCONNECT.
    // In the future, it will also include PHONEBOOK.

    if (msg.is_a(JOZA_MSG_CONNECT))
        ret = do_connect(msg);
    else if (msg.is_a(JOZA_MSG_DISCONNECT))
        ret = do_disconnect(msg);
    return ret;
}

// Handling a CALL_REQUEST message is special because it requires both
// PeerStore and ChannelStore.  This is the first half of the
// CALL_REQUEST processing.

typedef pair<int, string> _pair;

_pair PeerStore::do_call_request_step_1(Msg& msg)
{
    pair<int, string> ret(-1,"");
    size_t X_count = count(msg.key());

    assert(msg.is_a(JOZA_MSG_CALL_REQUEST));
    assert (X_count <= 1);

    if (X_count == 0) {
        // If we don't know who this is from, give up.
        joza_msg_send_addr_diagnostic (g_sock, msg.address(), c_local_procedure_error, d_unspecified);
        ret = _pair(2, "");
    }
    else {
        Peer& X = get(msg.key());

        // This message from from outside, to be paranoid and check it.

        if (X._name != msg.calling_address()) {
            // Bad calling address
            joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                          c_local_procedure_error,
                                          d_invalid_calling_address);
            ret = _pair(2, "");
        }
        else if (!address_validate(msg.called_address().c_str())) {
            // Bad calling address
            joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                          c_local_procedure_error,
                                          d_invalid_called_address);
            ret = _pair(2, "");
        }
        else if (!packet_validate(msg.packet())) {
            joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                          c_invalid_facility_request,
                                          d_facility_parameter_not_allowed);
            ret = _pair(2, "");
        }
        else if (!window_validate(msg.window())) {
            joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                          c_invalid_facility_request,
                                          d_facility_parameter_not_allowed);
            ret = _pair(2, "");
        }
        else if (!throughput_validate(msg.throughput())) {
            joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                          c_invalid_facility_request,
                                          d_facility_parameter_not_allowed);
            ret = _pair(2, "");
        }
        else if (msg.data_size() > MAX_CALL_REQUEST_DATA_SIZE) {
            joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                          c_local_procedure_error,
                                          d_packet_too_long);
            ret = _pair(2, "");
        }
        else {
            size_t Y_count = count_by_name(msg.called_address());

            assert (Y_count <= 1);
            if (Y_count == 0) {
                // Connect request to unknown peer
                joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                              c_local_procedure_error,
                                              d_invalid_called_address);
                ret = _pair(2, "");
            }
            else {
                Peer& Y = get_by_name(msg.called_address());

                if (X.busy()) {
                    // X is already on a call.  At this point I know it will get rejected, but,
                    // I let ChannelStore do it so it can send a "d_packet_type_invalid_for_sX"
                    // message.
                    ret = _pair(1, Y.key());
                }
                else if (Y.busy()) {
                    // Y is on a call and X is not.  Thus, Y is busy.
                    joza_msg_send_addr_diagnostic (g_sock, msg.address(), c_number_busy, d_call_collision);
                    ret = _pair(2, "");
                }
                else {
                    // OK finally. This is a valid message on valid free peers.
                    ret = _pair(1, Y.key());
                }
            }
        }
    }

    if (ret.first == -1)
        abort ();
    return ret;
}


int PeerStore::do_connect(Msg& msg)
{
    int ret = 0;

    if (count(msg.key()) != 0) {
        // Connected peer is trying to connect again.
        joza_msg_send_addr_diagnostic (g_sock, msg.address(), c_local_procedure_error, d_unspecified);
        ret = 2;
    }
    else if (_map.size() > MAX_PEER_COUNT) {
        // Too many peers already.  Reject this guy.
        joza_msg_send_addr_diagnostic(g_sock, msg.address(), c_network_congestion, d_unspecified);
        ret = 2;
    }
    else if (!address_validate(msg.calling_address().c_str())) {
        joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                      c_local_procedure_error,
                                      d_invalid_calling_address);
        ret = 2;            
    }
    else if (!direction_validate((direction_t) msg.ioDir())) {
        joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                      c_invalid_facility_request,
                                      d_facility_parameter_not_allowed);
        ret = 2;
    }
    else if (count_by_name(msg.calling_address()) != 0) {
        // Duplicate address
        joza_msg_send_addr_diagnostic(g_sock, msg.address(),
                                      c_local_procedure_error,
                                      d_invalid_calling_address);
        ret = 2;            
    }
    else {
        // Message from a new peer
        insert (msg.address(), msg.calling_address(), msg.ioDir());
        joza_msg_send_addr_connect_indication(g_sock, msg.address());
        ret = 2;
    }
    return ret;
}

int PeerStore::do_disconnect(Msg& msg)
{
    int ret = 0;
    if (count(msg.key()) == 0) {
        // Unknown peer is trying to disconnect
        joza_msg_send_addr_diagnostic (g_sock, msg.address(), c_local_procedure_error, d_unspecified);
        ret = 2;
    }
    else {
        disconnect(msg.key());
        joza_msg_send_addr_connect_indication(g_sock, msg.address());
        ret = 2;
    }
    return ret;
}

Peer& PeerStore::get(string key)
{
    return _map.at(key);
}

Peer& PeerStore::get_by_name(string addr)
{
    for (auto it = _map.begin(); it != _map.end(); ++it) {
        if (it->second._name == addr)
            return it->second;
    }
    abort();
}


// Create a new entry in the peer store
void PeerStore::insert(const zframe_t *pzf, string name, IoDir dir)
{
    char *key;
    key = zframe_strhex((zframe_t *) pzf);
    pair<string, Peer> v(key, Peer(pzf, name, dir));
    free (key);

    auto ret = _map.insert(v);

    // The check to see if we're re-adding a Peer belongs in dispatch().
    assert(ret.second == true);
};

#if 0
int main()
{
    char data[6] = "ABCDE";
    zframe_t *pzf = zframe_new(data, 5);
    char *hex = zframe_strhex(pzf);
    Peer p (pzf, "name", 1);

    pair<string, Peer> v(hex,p);
    peers.insert(v);
    free(hex);
                 
    return 0;
}

#endif