/*
    channel.h - peer-to-peer connections

    Copyright 2013, 2014 Michael L. Gran <spk121@yahoo.com>

    This file is part of Jozabad.

    Jozabad is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jozabad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jozabad.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file channel.h
 * @author Mike Gran
 * @brief A channel is a connection between two workers
 *
 * A channel is a stateful connection between two workers.
 */

#ifndef JOZA_CHANNEL_H
#define JOZA_CHANNEL_H

#include "joza_msg.h"
// #include "worker.h"
#include "mylimits.h"
#include "packet.h"
#include "tput.h"
#include "seq.h"
#include "state.h"

#ifndef CALL_REQUEST_DATA_LEN
# define CALL_REQUEST_DATA_LEN (16)
#endif

/**
 * @brief The channel state structure.
 *
 * The channel state structure holds the addresses of two workers and the state
 * of the communication between them.
 */
typedef struct {
    lcn_t lcn;        /**< Logical Channel Number, a unique key */
    zframe_t *xzaddr; /**< a ZMQ frame containing a ZMQ router identity for X caller */
    zframe_t *yzaddr; /**< a ZMQ frame containing a ZMQ router identity for Y callee*/
    gchar *xname;
    gchar *yname;
    state_t state;    /**< call status */
    seq_t xps;        /**< ID of next packet to be sent by X */
    seq_t xpr;        /**< Smallest packet ID that X will accept from Y */
    seq_t yps;        /**< ID of next packet to be sent by Y */
    seq_t ypr;        /**< Smallest packet ID Y will accept from X */
    seq_t window;     /**< delta between the smallest and largest acceptable ID */
    packet_t pkt;
    tput_t tput;      /**< Throughput allowed for this channel */
    guint64 ctime;    /**< Creation time of the channel in microseconds since 1970 */
    guint64 mtime;    /**< Time of last message from either peer in microseconds since 1970 */
} channel_t;


channel_t *
channel_create(lcn_t lcn, zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname,
               packet_t pkt, seq_t window, tput_t tput);
lcn_t channel_add(zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname,
                  packet_t pkt, seq_t window, tput_t tput);
void channel_disconnect_all(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr);
state_t channel_dispatch(channel_t *channel, void *sock, joza_msg_t *M, bool is_y);
void channel_set_state(channel_t *channel, state_t state);
gboolean channel_is_closed(channel_t *channel);

#endif
