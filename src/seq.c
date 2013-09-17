#include <assert.h>
#include "seq.h"
#include "bool.h"

int seq_rngchk(seq_t x)
{
    if (x < SEQ_MIN)
        return -1;
    else if (x >= SEQ_MAX)
        return 1;
    return 0;
}

// To do this comparison, we use double-wide integer types to avoid worrying about
// numerical overflow.
bool_t seq_in_range(seq_t x, seq_t lo, seq_t hi)
{
    if (hi < lo) {
        if (x <= hi || x >= lo)
            return TRUE;
    } else if (hi > lo) {
        if (x >= lo && x <= hi)
            return TRUE;
    }
    return FALSE;
}

// Checks to see if, in the context of an CALL_REQUEST negotiation, it
// is valid to set the throughput from CURRENT to REQUEST.  Returns
// non-zero if true.
int window_negotiate(seq_t request, seq_t current)
{
    assert (seq_rngchk(request) == 0);
    assert (seq_rngchk(current) == 0);

    if (request > WINDOW_NEGOTIATE) {
        if (request <= current)
            return 1;
        else
            return 0;
    } else if (request < WINDOW_NEGOTIATE) {
        if (request >= current)
            return 1;
        else
            return 0;
    }
    /* else request == WINDOW_NEGOTIATE */
    return 1;
}