/*
 * File:   connections.h
 * Author: mike
 *
 * Created on August 24, 2013, 1:47 PM
 */

#ifndef CONNECTIONS_H
#define	CONNECTIONS_H

#include <zmq.h>
#include <czmq.h>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include "../libjoza/joza_msg.h"
#include "../libjoza/joza_lib.h"
#include "state.h"
#include "flow.h"

using namespace std;

#define X121_ADDRESS_LENGTH (15)
#define CONNECTIONS_MAX (256)

class Connection {
public:
    int id;
    zframe_t *zmq_address; 
    char x121_address[X121_ADDRESS_LENGTH]; 
    direction_t direction;
    uint8_t busy;

    Connection(zframe_t *address, const char *name, direction_t d);
    Connection(zframe_t const* address, const char *name, direction_t d);
    ~Connection();
};

typedef map<string, Connection*> connection_store_t;

const char*
    find_dname(const char *key);
Connection*
    find_worker(const char* key);
void
    connection_msg_send(const char *key, joza_msg_t **msg);
void
    add_connection(const char *key, zframe_t const* zmq_addr,
                   char const* x121_addr, direction_t d);
bool
    connection_dispatch(const char *key, joza_msg_t *msg);
Connection*
    find_worker_by_x121_address(const char* addr);
void
    connection_disconnect(const char *key);


class Channel {
public:
    char *x_key;
    char *y_key;

    state_t state;
    flow_t flow;
    packet_t packet_size_index;
    uint16_t window_size;
    throughput_t throughput_index;

    Channel(const char *key_x, const char *key_y);
    ~Channel();

    void dispatch(const joza_msg_t *msg);
    void do_clear(cause_t c, diagnostic_t d);
    void do_reset(cause_t c, diagnostic_t d);
    void do_disconnect(diagnostic_t d);
    void do_x_disconnect(diagnostic_t d);
    void do_x_call_request(const char *called_address, packet_t p, uint16_t w, throughput_t t);
    void do_x_call_collision(void);
    void do_x_clear_request(diagnostic_t d);
    void do_x_clear_confirmation(diagnostic_t d);
    void do_x_data(uint16_t ps, uint16_t pr, const zframe_t *data);
    void do_x_rr(uint16_t pr);
    void do_x_rnr(uint16_t pr);
    void do_x_reset(diagnostic_t d);
    void do_x_reset_confirmation(diagnostic_t d);
    void do_y_disconnect(void);
    void do_y_call_accepted(void);
    void do_y_call_collision(void);
    void do_y_clear_request(diagnostic_t d);
    void do_y_clear_confirmation(diagnostic_t d);
    void do_y_data(uint16_t ps, uint16_t pr, const zframe_t *data);
    void do_y_rr(uint16_t pr);
    void do_y_rnr(uint16_t pr);
    void do_y_reset(diagnostic_t d);
    void do_y_reset_confirmation(diagnostic_t d);
};


typedef vector<Channel*> channel_store_t;
extern channel_store_t channel_store;

void add_channel(const char *x_key, const char *y_key);
void remove_channel(Channel* c);
Channel* find_channel(const char *key);

#endif	/* CONNECTIONS_H */

