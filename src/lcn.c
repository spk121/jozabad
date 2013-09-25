/*
    lcn.c - logical channel numbers

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
#include <stdint.h>
#include "lcn.h"
#include "bool.h"

chan_idx_t lcn_find(lcn_t arr[], chan_idx_t n, lcn_t key);
chan_idx_lcn_t lcn_next(lcn_t arr[], chan_idx_t n, lcn_t key);

static_assert(sizeof(lcn_t) == 2, "double_lcn_t doesn't match lcn_t");
typedef uint32_t double_lcn_t;
# define LCN_SHIFT 16
# define LCN_MASK UINT32_C(0xFFFF)

// Using a bisection search on a matrix ARR of length N, which is
// supposed to contain a strictly monotonically increasing list of
// unique key integers, return the location of the key, or, if it is
// not found, the location where the key would be inserted.  A return
// value of N indicates after the end of the matrix.
chan_idx_t lcn_find(lcn_t arr[], chan_idx_t n, lcn_t key)
{
    chan_idx_t lo, hi, mid;

    assert(key < LCN_MAX);

    if (n == CHAN_IDX_C(0))
        return CHAN_IDX_C(0);

    lo = CHAN_IDX_C(0);
    hi = n;
    while (hi - lo > CHAN_IDX_C(1)) {
        mid = (hi + lo) >> 1;
        if (key >= arr[mid])
            lo = mid;
        else
            hi = mid;
    }
    if (key <= arr[lo])
        return lo;
    return hi;
}

// Given a table ARR of strictly monotonically increasing integers
// that function as unique keys, and a desired KEY, this searches the
// array to ensure that KEY does not appear in the array.  If it
// doesn't appear in the array, KEY is returned.  If it does appear,
// the next avaialble unique integer key is returned.  J is an array
// index that is the starting point of the search.
static lcn_t _keynext(lcn_t arr[], chan_idx_t n, lcn_t key, chan_idx_t j)
{
    assert (n < LCN_MAX);  // infinite loop when all integers are being used as keys
    assert (key < LCN_MAX);
    assert (j <= n);

    while (j < n) {
        lcn_t k = arr[j];
        if (k < key)
            j ++;
        else if (k == key) {
            key ++;

            // Check if ID has numerically wrapped back to zero
            if (key == LCN_MAX) {
                key = 0;
                j = 0;
            }
        } else
            break;
    }
    return key;
}

// Given a table ARR of strictly monotonically increasing integers that
// are presumed to be a list of unique keys
// - find the next free unique KEY
// - find the location where that KEY could be inserted into the array
// The KEY parameter is the starting point for the search for a new key.
chan_idx_lcn_t lcn_next(lcn_t arr[], chan_idx_t n, lcn_t key)
{
    chan_idx_lcn_t ret;
    chan_idx_t index;
    bool_t did_loop = FALSE;

    assert(n < LCN_MAX);
    assert(key < LCN_MAX);

loop:
    index = lcn_find(arr, n, key);
    // If this ID is a duplicate, lcn_find() will have suggested
    // that it be inserted at the location of an entry with the same
    // ID.  So we check the entry to the left to see if this ID is
    // a duplicate, and then increment it if necessary.
    if (arr[index] == key) {
        assert (!did_loop);
        key = _keynext(arr, n, key, index);
        did_loop = TRUE;
        goto loop;
    }
    ret.index = index;
    ret.key = key;
    return ret;
}
