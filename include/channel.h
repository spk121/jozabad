/*
 * File:   channel.h
 * Author: mike
 *
 * Created on August 25, 2013, 10:39 AM
 */

#ifndef CHANNEL_H
#define	CHANNEL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <czmq.h>
#include <stdint.h>
#include "../include/parch_msg.h"
#include "../include/state.h"
#include "../include/flow.h"
#include "../include/packet.h"
#include "../include/window.h"
#include "../include/throughput.h"

#define CHANNELS_MAGIC 0x301
#define CHANNELS_MIN 1
#define CHANNELS_MAX 256

struct _channel_t {
    char *x_key;
    char *y_key;
    state_t state;
    flow_t flow;
    uint8_t packet_size_index;
    uint16_t window_size;
    uint8_t throughput_index;
};

typedef struct _channel_t channel_t;

struct _channel_store_t {
    const int _magic;
    const int min;
    const int max;
    int count;
    channel_t channels[CHANNELS_MAX + 1];
};

typedef struct _channel_store_t channel_store_t;

extern channel_store_t channel_store;

void
channel_store_init(channel_store_t *channels);
uint16_t
channel_store_find_worker (channel_store_t *channels, const char *key);
channel_t
channel_dispatch (channel_t channel, const parch_msg_t *msg);
void
channel_store_add_channel (channel_store_t *c, const char *x_key, const char *y_key);
void
channel_store_remove_channel (channel_store_t *c, int id);
#ifdef	__cplusplus
}
#endif

#endif	/* CHANNEL_H */

