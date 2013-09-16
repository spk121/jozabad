#include <assert.h>
#include "seq.h"

int seq_rngchk(seq_t x)
{
	if (x < SEQ_MIN)
		return -1;
	else if (x >= SEQ_MAX)
		return 1;
	return 0;
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
    }
    else if (request < WINDOW_NEGOTIATE) {
        if (request >= current)
            return 1;
        else
            return 0;
    }
	/* else request == WINDOW_NEGOTIATE */
	return 1;
}