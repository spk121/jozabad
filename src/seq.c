/*
    seq.c - message sequence numbers and flow control

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

#include <assert.h>
#include "seq.h"
#include "bool.h"

int seq_rngchk(seq_t x)
{
    if (x < SEQ_MIN)
        return -1;
    else if (x > SEQ_MAX)
        return 1;
    return 0;
}

// To do this comparison, we use double-wide integer types to avoid worrying about
// numerical overflow.
bool_t seq_in_range(seq_t x, seq_t lo, seq_t hi)
{
    if (hi < lo && (x <= hi || x >= lo))
        return TRUE;
    else if (hi > lo && (x >= lo && x <= hi))
        return TRUE;
    else if (hi == lo && lo == x)
        return TRUE;
    return FALSE;
}

int window_rngchk(seq_t x)
{
    if (x < WINDOW_MIN)
        return -1;
    else if (x > WINDOW_MAX)
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
    } else if (request < WINDOW_NEGOTIATE) {
        if (request >= current)
            return 1;
        else
            return 0;
    }
    /* else request == WINDOW_NEGOTIATE */
    return 1;
}
