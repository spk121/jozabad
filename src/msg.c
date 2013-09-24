#include <czmq.h>
#include "msg.h"
#include "poll.h"
#include "joza_msg.h"
#include "cause.h"
#include "diag.h"
#include "worker.h"
#include "log.h"

// Deep diving into the ZeroMQ source says that for ZeroMQ 3.x,
// bytes 1 to 5 of the address would work as a unique ID for a
// given connection.
wkey_t msg_addr2key (const zframe_t *z)
{
    wkey_t x[1];

    memcpy(x, (char *) zframe_data((zframe_t *) z) + 1, sizeof(wkey_t));

    return x[0];
}

void diagnostic(const zframe_t *A, cause_t C, diag_t D)
{
    wkey_t key;
    bool_index_t bi; 

    key = msg_addr2key(A);
    bi = worker_get_idx_by_key(key);
    INFO("sending DIAGNOSTIC to %s, %s", w_name[bi.index], cause_name(C));
    joza_msg_send_addr_diagnostic(g_poll_sock, A, C, D);
}

void call_request(zframe_t *call_addr, zframe_t *return_addr, char *xname, char *yname,
                  packet_t pkt, seq_t window, tput_t tput, zframe_t *data)
{
    int ret;
    ret = joza_msg_send_addr_call_request(g_poll_sock, call_addr, xname, yname, pkt, window, tput, data);
    if (ret == -1)
        DIAGNOSTIC(return_addr, c_zmq_sendmsg_err, errno2diag());
}
