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

using namespace std;

#include "direction.h"
#include "throughput.h"
#include "name.h"
#include "msg.h"
#include "state.h"
#include "flow.h"
#include "packet.h"
#include "diagnostic.h"

//#include "channel.h"


#define DNAME_LENGTH_MAX 16
#define DNAME_COUNT (4*26)
#define CONNECTIONS_MAX (256)

class Connection {
public:
    int id;
    zframe_t *address; // The ZeroMQ address frame of the worker
    char name[NAME_LENGTH_MAX]; // The name of the worker. Null-terminated string.
    throughput_t throughput;
    direction_t direction;
    uint8_t busy; // When true, this connection is on a call

    Connection(zframe_t *address, const char *name, throughput_t t, direction_t d);
    ~Connection();
};

typedef map<string, Connection*> connection_store_t;
Connection* find_worker(const char* key);
const char* find_dname(const char *key);
void connection_msg_send(const char *key, msg_t **msg);
char const *dname(uint16_t i);


void add_connection(const char* key, msg_t* msg);
void connection_msg_send(const char* key, msg_t** msg);
bool connection_dispatch(const char *key, msg_t *msg);
Connection* find_worker_by_name(const char* name);
void connection_disconnect(const char *key);


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

    void dispatch(const msg_t *msg);
    void do_clear(diagnostic_t d);
    void do_reset(diagnostic_t d);
    void do_disconnect(diagnostic_t d);
    void do_x_disconnect(diagnostic_t d);
    void do_x_call_request(const char *service, packet_t p, uint16_t w, throughput_t t);
    void do_x_call_collision(void);
    void do_x_clear_request(diagnostic_t d);
    void do_x_clear_confirmation(diagnostic_t d);
    void do_x_data(uint16_t seq, const zframe_t *data);
    void do_x_rr(uint16_t seq);
    void do_x_rnr(uint16_t seq);
    void do_x_reset(diagnostic_t d);
    void do_x_reset_confirmation(diagnostic_t d);
    void do_y_disconnect(void);
    void do_y_call_accepted(void);
    void do_y_call_collision(void);
    void do_y_clear_request(diagnostic_t d);
    void do_y_clear_confirmation(diagnostic_t d);
    void do_y_data(uint16_t seq, const zframe_t *data);
    void do_y_rr(uint16_t seq);
    void do_y_rnr(uint16_t seq);
    void do_y_reset(diagnostic_t d);
    void do_y_reset_confirmation(diagnostic_t d);
};


typedef vector<Channel*> channel_store_t;
extern channel_store_t channel_store;

void add_channel(const char *x_key, const char *y_key);
void remove_channel(Channel* c);
Channel* find_channel(const char *key);

#endif	/* CONNECTIONS_H */

