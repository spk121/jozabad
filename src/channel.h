/*
    channel.h - peer-to-peer connections

    Copyright 2013 Michael L. Gran <spk121@yahoo.com>

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

typedef struct {
    lcn_t lcn;
    zframe_t *xzaddr, *yzaddr;
    gchar *xname, *yname;
    state_t state;
    seq_t xps, xpr, yps, ypr, window;
    packet_t pkt;
    tput_t tput;
    guint64 ctime, mtime;
} channel_t;


channel_t *
channel_create(lcn_t lcn, zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname,
               packet_t pkt, seq_t window, tput_t tput);
lcn_t channel_add(zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname,
                  packet_t pkt, seq_t window, tput_t tput);
void channel_disconnect_all(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr);
state_t channel_dispatch(channel_t *channel, void *sock, joza_msg_t *M, bool is_y);


#endif
