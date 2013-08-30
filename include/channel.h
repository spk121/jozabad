/*
 * File:   channel.h
 * Author: mike
 *
 * Created on August 25, 2013, 10:39 AM
 */

#ifndef CHANNEL_H
#define	CHANNEL_H

#include <vector>
#include <czmq.h>
#include <stdint.h>
// #include "../include/msg.h"
#include "log.h"
#include "../include/state.h"
#include "../include/flow.h"
#include "../include/packet.h"
#include "../include/window.h"
#include "../include/throughput.h"
#include "msg.h"

using namespace std;
#define CHANNELS_MAGIC 0x301
#define CHANNELS_MIN 1
#define CHANNELS_MAX 256

class Channel {
public:
    char *x_key;
    char *y_key;

    state_t state;
    flow_t flow;
    packet_t packet_size_index;
    uint16_t window_size;
    throughput_t throughput_index;

	Channel (const char *key_x, const char *key_y) {x_key = strdup(key_x); y_key = strdup(key_y);}
	~Channel () {free(x_key); free(y_key);};
};

typedef vector<Channel*> channel_store_t;
extern channel_store_t channel_store;

Channel*
find_channel(channel_store_t* p_channel_store, const char *key, const char *dname);
void
add_channel (channel_store_t* p_channel_store, const char *x_key, const char *x_dname,
             const char *y_key, const char *y_dname);
Channel
channel_dispatch (Channel c, const char *x_dname, const char *y_dname, const msg_t *msg);
void
remove_channel (channel_store_t* cs, Channel* c, const char* xdn, const char* ydn);
#if 0
class ChannelStore {
 public:
    vector<Channel *> store;

    ChannelStore();

	void add_channel(const char *key_x, const char *key_y) {
		Channel* ch = new Channel(key_x, key_y);
        store.push_back(ch);
		Connection *cx = connection_store.find_worker(key_x);
		Connection *cy = connection_store.find_worker(key_y);
		INFO ("connecting %s/%s as channel", cx->dname(), cy->dname());
	};

};

extern ChannelStore channel_store;

#endif

#if 0
void
channel_store_init(channel_store_t *channels);
uint16_t
channel_store_find_worker (channel_store_t *channels, const char *key);

#endif

#if 0
channel_t
channel_dispatch (channel_t channel, const parch_msg_t *msg);
void
channel_store_add_channel (channel_store_t *c, const char *x_key, const char *y_key);
void
channel_store_remove_channel (channel_store_t *c, int id);
#endif

#endif	/* CHANNEL_H */

