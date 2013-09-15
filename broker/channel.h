#ifndef JOZA_CHANNEL_H
#define JOZA_CHANNEL_H

#include "../libjoza/joza_msg.h"
#include "worker.h"

#ifndef SEQ_WIDTH
# define SEQ_WIDTH 2
#endif

#if SEQ_WIDTH == 1
typedef uint8_t seq_t;
typedef uint16_t dseq_t;
# define SEQ_MIN UINT8_C(0)
# define SEQ_MAX UINT8_C(SCHAR_MAX)
# define SEQ_C(x) UINT8_C(x)
#elif SEQ_WIDTH == 2
typedef uint16_t seq_t;
typedef uint32_t dseq_t;
# define SEQ_MIN UINT16_C(0)
# define SEQ_MAX UINT16_C(INT16_MAX)
# define SEQ_C(x) UINT16_C(x)
#elif SEQ_WIDTH == 4
typedef uint32_t seq_t;
typedef uint64_t dseq_t;
# define SEQ_MIN UINT32_C(0)
# define SEQ_MAX UINT32_C(INT32_MAX)
# define SEQ_C(x) UINT32_C(x)
#else
#error "Bad SEQ_WIDTH"
#endif

#ifndef WINDOW_DEFAULT
# define WINDOW_DEFAULT SEQ_C(2)
#endif

#ifndef CHANNEL_COUNT
# define CHANNEL_COUNT UKEY_C(256)
#endif
static_assert(CHANNEL_COUNT < UKEY_MAX, "CHANNEL_COUNT too large for ukey_t");

extern void *c_xzaddr[CHANNEL_COUNT]; /* ZMQ address of caller X */
extern void *c_yzaddr[CHANNEL_COUNT]; /* ZMQ address of callee Y */
extern size_t c_yidx[CHANNEL_COUNT]; /* index array that sorts ykey array */
extern char *c_xname[CHANNEL_COUNT];
extern char *c_yname[CHANNEL_COUNT];

void channel_dispatch_by_lcn(joza_msg_t *M, ukey_t LCN, role_t R);

#endif
