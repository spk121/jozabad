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
#include "lcn.h"

#ifndef CALL_REQUEST_DATA_LEN
# define CALL_REQUEST_DATA_LEN (16)
#endif

extern lcn_t c_lcn[CHAN_COUNT];
extern zframe_t *c_xzaddr[CHAN_COUNT]; /* ZMQ address of caller X */
extern zframe_t *c_yzaddr[CHAN_COUNT]; /* ZMQ address of callee Y */
extern chan_idx_t c_yidx[CHAN_COUNT]; /* index array that sorts ykey array */
extern const char *c_xname[CHAN_COUNT];
extern const char *c_yname[CHAN_COUNT];
extern packet_t c_pkt[CHAN_COUNT];
extern tput_t c_tput[CHAN_COUNT]; /* bits/sec permitted on this channel */
extern seq_t c_window[CHAN_COUNT];
extern state_t c_state[CHAN_COUNT];

void channel_dispatch_by_lcn(void *sock, joza_msg_t *M, lcn_t LCN, bool is_y);
bool channel_available(void);
lcn_t channel_add(zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname,
                  packet_t pkt, seq_t window, tput_t tput);
void channel_disconnect_all(void *sock);

#endif
