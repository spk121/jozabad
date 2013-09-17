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
#include "packet.h"
#include "tput.h"
#include "seq.h"
#include "state.h"
#include "ukey.h"

#ifndef WINDOW_DEFAULT
# define WINDOW_DEFAULT SEQ_C(2)
#endif

#ifndef CHANNEL_COUNT
# define CHANNEL_COUNT UKEY_C(256)
#endif
static_assert(CHANNEL_COUNT < UKEY_MAX, "CHANNEL_COUNT too large for ukey_t");

#ifndef CALL_REQUEST_DATA_LEN
# define CALL_REQUEST_DATA_LEN (16)
#endif

extern ukey_t c_lcn[CHANNEL_COUNT];
extern zframe_t *c_xzaddr[CHANNEL_COUNT]; /* ZMQ address of caller X */
extern zframe_t *c_yzaddr[CHANNEL_COUNT]; /* ZMQ address of callee Y */
extern size_t c_yidx[CHANNEL_COUNT]; /* index array that sorts ykey array */
extern char *c_xname[CHANNEL_COUNT];
extern char *c_yname[CHANNEL_COUNT];
extern packet_t c_pkt[CHANNEL_COUNT];
extern tput_t c_tput[CHANNEL_COUNT]; /* bits/sec permitted on this channel */
extern seq_t c_window[CHANNEL_COUNT];
extern state_t c_state[CHANNEL_COUNT];

void channel_dispatch_by_lcn(joza_msg_t *M, ukey_t LCN, bool_t is_y);
bool_t channel_available(void);
ukey_t channel_add(zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname);
#endif
