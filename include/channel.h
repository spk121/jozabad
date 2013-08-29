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
#include "connections.h"
#include "../include/state.h"
#include "../include/flow.h"
#include "../include/packet.h"
#include "../include/window.h"
#include "../include/throughput.h"

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

	Channel* find_channel(const char *key) {
		int found = 0;
		Channel* ch = (Channel*) NULL;
		const char* dname = connection_store.find_worker(key)->dname();
		for (vector<Channel *>::iterator it = store.begin();
			it != store.end();
			++ it) {
				if (strcmp(ch->x_key, key) == 0) {
					found = 1;
					ch = *it;
					break;
				}
				else if (strcmp(ch->y_key, key) == 0) {
					found = 2;
					ch = *it;
					break;
				}
		}
		if (found == 0) {
			NOTE("did not find %s in channel_store", dname);
		}
		else if (found == 1) {
			NOTE("found %s as X in channel_store", dname);
		}
		else if (found == 2) {
			NOTE("found %s as Y in channel_store", dname);
		}
		else
			abort();
		return ch;
	}
};

extern ChannelStore channel_store;

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

