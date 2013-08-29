/*
 * File:   connections.h
 * Author: mike
 *
 * Created on August 24, 2013, 1:47 PM
 */

#ifndef CONNECTIONS_H
#define	CONNECTIONS_H

#include <zmq.h>
#include <map>
#include <string>
#include <memory>

using namespace std;

#include "direction.h"
#include "throughput.h"
//#include "name.h"
//#include "msg.h"


#define DNAME_LENGTH_MAX 16
#define DNAME_COUNT (4*26)

class Connection {
 public:
    uint16_t id;
    zframe_t *address; // The ZeroMQ address frame of the worker
    char name[NAME_LENGTH_MAX]; // The name of the worker. Null-terminated string.
    throughput_t throughput;
    direction_t direction;
    uint8_t busy; // When true, this connection is on a call

    Connection (zframe_t *address, const char *name, throughput_t t, direction_t d);
    char const *dname();
    ~Connection ();
};

#define CONNECTIONS_MAX (256)

class ConnectionStore {
 public:
    std::map<string, Connection *> store;
    ConnectionStore();
    Connection* find_worker(const char *key);
    void connect(const char *key, msg_t *msg);
    void connection_msg_send(const char *key, msg_t **msg);
};

ConnectionStore connection_store;

#if 0
struct _connection_store_t {
    const int min;
    const int max;
    int count;
    connection_t connections[CONNECTIONS_MAX + 1];
};

typedef struct _connection_store_t connection_store_t;

extern connection_store_t connection_store;

void
connection_store_init(connection_store_t *c) ;
uint16_t
connection_store_find_worker (connection_store_t *c, const char *key);
uint16_t
connection_store_find_worker_by_name (connection_store_t *c, const char *name, const char *my_key);
void
connection_dispatch (connection_store_t *c, const char *key, parch_msg_t *msg);

void add_to_worker_directory(char *key, zframe_t *address, const char *service, byte throughput,
        byte incoming, byte outgoing);

void
connection_msg_send (connection_store_t *c, const char *key, parch_msg_t **msg);
void
connection_connect(connection_store_t *c, const char *key, parch_msg_t *msg);
void
connection_disconnect(connection_store_t *c, const char *key);
const char *
dname(const char *key);
#endif
#endif	/* CONNECTIONS_H */

