#ifndef JOZA_CHANNEL_H
#define JOZA_CHANNEL_H

#include "../libjoza/joza_msg.h"
#include "worker.h"
#include "packet.h"
#include "tput.h"
#include "seq.h"
#include "state.h"

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


void channel_dispatch_by_lcn(joza_msg_t *M, ukey_t LCN, role_t R);
bool_t channel_available(void);
ukey_t add_channel(zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname);
#endif
