#include <assert.h>
#include <stdint.h>
#include "ukey.h"
#include "bool.h"

uint32_t ukey_find(ukey_t arr[], uint32_t n, ukey_t key);
index_ukey_t ukey_next(ukey_t arr[], uint32_t n, ukey_t key);

#if UKEY_WIDTH == 1
typedef uint16_t double_ukey_t;
# define UKEY_SHIFT 8
# define UKEY_MASK UINT16_C(0xFF)
#elif UKEY_WIDTH == 2
typedef uint32_t double_ukey_t;
# define UKEY_SHIFT 16
# define UKEY_MASK UINT32_C(0xFFFF)
#elif UKEY_WIDTH == 4
typedef uint64_t double_ukey_t;
#define UKEY_SHIFT 32
#define UKEY_MASK UINT64_C(0xFFFFFFFF)
#else
# error "Bad UKEY_WIDTH"
#endif


// Using a bisection search on a matrix ARR of length N, which is
// supposed to contain a strictly monotonically increasing list of
// unique key integers, return the location of the key, or, if it is
// not found, the location where the key would be inserted.  A return
// value of N indicates after the end of the matrix.
uint32_t ukey_find(ukey_t arr[], uint32_t n, ukey_t key)
{
    uint32_t lo, hi, mid;

    assert(key < UKEY_MAX);

    if (n == 0)
        return 0;

    lo = 0;
    hi = n + 1;
    while (hi - lo > 1) {
        mid = (hi + lo) >> 1;
        if (key >= arr[mid - 1])
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}

// Given a table ARR of strictly monotonically increasing integers
// that function as unique keys, and a desired KEY, this searches the
// array to ensure that KEY does not appear in the array.  If it
// doesn't appear in the array, KEY is returned.  If it does appear,
// the next avaialble unique integer key is returned.  J is an array
// index that is the starting point of the search.
static ukey_t _keynext(ukey_t arr[], uint32_t n, ukey_t key, uint32_t j)
{
    assert (n < UKEY_MAX);  // infinite loop when all integers are being used as keys
    assert (key < UKEY_MAX);
    assert (j <= n);

    while (j < n) {
        ukey_t k = arr[j];
        if (k < key)
            j ++;
        else if (k == key) {
            key ++;

            // Check if ID has numerically wrapped back to zero
            if (key == UKEY_MAX) {
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
index_ukey_t ukey_next(ukey_t arr[], uint32_t n, ukey_t key)
{
    index_ukey_t ret;
    int index;
    bool_t did_loop = FALSE;

    assert(n < UKEY_MAX);
    assert(key < UKEY_MAX);

loop:
    index = ukey_find(arr, n, key);
    if (index > 0) {
        // If this ID is a duplicate, keyfind() will have suggested
        // that it be inserted to the right of an entry with the same
        // ID.  So we check the entry to the left to see if this ID is
        // a duplicate, and then increment it if necessary.
        if (arr[index - 1] == key) {
            assert (!did_loop);
            key = _keynext(arr, n, key, index - 1);
            did_loop = TRUE;
            goto loop;
        }
    }
    ret.index = index;
    ret.key = key;
    return ret;
}
