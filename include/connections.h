/*
 * File:   connections.h
 * Author: mike
 *
 * Created on August 24, 2013, 1:47 PM
 */

#ifndef CONNECTIONS_H
#define	CONNECTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <czmq.h>
#include "parch_msg.h"

#define CONNECTIONS_MIN 1
#define CONNECTIONS_MAX 256

#include "direction.h"

struct _connection_t {
    char *key; // the string hash of the ZeroMQ address frame
    zframe_t *address; // The ZeroMQ address frame of the worker
    char dname[16]; // The name of the worker in debug messages
    char name[41]; // The name of the worker. Null-terminated string.
    uint8_t throughput_index; // An index indicating the maximum bits/sec allowed
    uint8_t busy; // When true, this connection is on a call
    direction_t direction;
};
typedef struct _connection_t connection_t;

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
#ifdef	__cplusplus
}
#endif

#endif	/* CONNECTIONS_H */

