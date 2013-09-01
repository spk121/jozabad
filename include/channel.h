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
#include "connections.h"
#include "diagnostic.h"

#ifndef DEF_CONNECTION_STORE_T
#define DEF_CONNECTION_STORE_T
struct Connection;
typedef map<string, Connection*> connection_store_t;
extern connection_store_t connection_store;
#endif

using namespace std;

class Channel {
public:
    char *x_key;
    char *y_key;

    state_t state;
    flow_t flow;
    packet_t packet_size_index;
    uint16_t window_size;
    throughput_t throughput_index;

    Channel(const char *key_x, const char *key_y) {
        x_key = strdup(key_x);
        y_key = strdup(key_y);
        state = state_ready;
        flow = init();
        packet_size_index = p_last;
        window_size = WINDOW_MAX;
        throughput_index = t_last;
    }

    ~Channel() {
        free(x_key);
        free(y_key);
    };

    void do_clear(connection_store_t *con_store, diagnostic_t d) {
        msg_t *x_msg = msg_new(MSG_CLEAR_REQUEST);
        msg_set_diagnostic(x_msg, d);
        connection_msg_send(con_store, x_key, &x_msg);

        msg_t *y_msg = msg_new(MSG_CLEAR_REQUEST);
        msg_set_diagnostic(y_msg, d);
        connection_msg_send(con_store, y_key, &y_msg);

        state = state_y_clear_request;
    }

    void do_reset(connection_store_t* con_store, diagnostic_t d) {
        msg_t *x_msg = msg_new(MSG_RESET_REQUEST);
        msg_set_diagnostic(x_msg, d);
        connection_msg_send(con_store, x_key, &x_msg);

        msg_t *y_msg = msg_new(MSG_RESET_REQUEST);
        msg_set_diagnostic(y_msg, d);
        connection_msg_send(con_store, y_key, &y_msg);
        flow = reset(flow);
        state = state_y_reset_request;
    }

    void do_disconnect(connection_store_t* con_store, diagnostic_t d) {
        msg_t *x_msg = msg_new(MSG_DISCONNECT_INDICATION);
        msg_set_diagnostic(x_msg, d);
        connection_msg_send(con_store, x_key, &x_msg);

        msg_t *y_msg = msg_new(MSG_DISCONNECT_INDICATION);
        msg_set_diagnostic(y_msg, d);
        connection_msg_send(con_store, y_key, &y_msg);

        connection_disconnect(con_store, x_key);
        connection_disconnect(con_store, y_key);

        state = state_ready;
    }

    void do_x_call_request(connection_store_t *con_store, const char *service, packet_t p, uint16_t w, throughput_t t) {
        // Validate the facilities requests
        if (!validate(p)) {
            do_clear(con_store, diagnostic);
        } else if (!window_validate(w)) {
            do_clear(con_store, diagnostic);
        } else if (!validate(t)) {
            do_clear(con_store, diagnostic);
        } else {
            // CALL REQUEST NEGOTIATION -- STEP 1
            // The call request from X is throttled by the Broker's limitations
            // FIXME: we should be throttling these with configuration options
            p = throttle(p, p_last);
            w = window_throttle(w, WINDOW_MAX);
            t = throttle(t, opt_throughput);

            // Forward the call request to Y
            msg_t *y_msg = msg_new(MSG_CALL_REQUEST);
            msg_set_service(y_msg, service);
            msg_set_packet(y_msg, p);
            msg_set_window(y_msg, w);
            msg_set_throughput(y_msg, t);
            connection_msg_send(con_store, y_key, &y_msg);

            // Set the state to X CALL
            state = state_x_call_request;
        }
    }
};

typedef vector<Channel*> _channel_store_t;
#ifndef TYPEDEF_CHANNEL_STORE_T
#define TYPEDEF_CHANNEL_STORE_T
typedef _channel_store_t channel_store_t;
#endif
//extern channel_store_t channel_store;

Channel*
find_channel(channel_store_t* p_channel_store, const char *key, const char *dname);
void
add_channel(channel_store_t* p_channel_store, const char *x_key, const char *x_dname,
        const char *y_key, const char *y_dname);
Channel
channel_dispatch(Channel c, const char *x_dname, const char *y_dname, const msg_t *msg);
void
remove_channel(channel_store_t* cs, Channel* c, const char* xdn, const char* ydn);
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
        INFO("connecting %s/%s as channel", cx->dname(), cy->dname());
    };

};

extern ChannelStore channel_store;

#endif

#if 0
void
channel_store_init(channel_store_t *channels);
uint16_t
channel_store_find_worker(channel_store_t *channels, const char *key);

#endif

#if 0
channel_t
channel_dispatch(channel_t channel, const parch_msg_t *msg);
void
channel_store_add_channel(channel_store_t *c, const char *x_key, const char *y_key);
void
channel_store_remove_channel(channel_store_t *c, int id);
#endif

#endif	/* CHANNEL_H */

