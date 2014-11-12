/*
    lib.c - general functions

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

#ifdef __GNUC__
# define _GNU_SOURCE
#endif

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "lib.h"

//----------------------------------------------------------------------------
// COMPAT
//----------------------------------------------------------------------------

size_t intlen(int x)
{
    double dx = (double)x;
    if (x == 0)
        return 1;

    int d = (int)(log10(fabs(dx))+1.0);
    if (x < 0)
        d++;
    return d;
}

char *cstrdup (const char *s)
{
    size_t len = strlen (s) + 1;
    void *s2 = malloc (len);

    if (s2 == NULL)
        return NULL;

    return (char *) memcpy (s2, s, len);
}

#if __GLIBC__ == 2
// This can be removed once Annex K hits GNU libc
size_t strnlen_s (const char *s, size_t maxsize)
{
    if (s == NULL)
        return 0;

    return strnlen (s, maxsize);
}
#endif

//----------------------------------------------------------------------------
// ECMA-1 STRINGS
//----------------------------------------------------------------------------

// An obscure, 6-bit encoding

const char ecma1[65] =            \
                                  " \t\n\v\f\r\016\017()*+,-./" \
                                  "0123456789:;<=>?"            \
                                  "\000ABCDEFGHIJKLMNO"         \
                                  "PQRSTUVWXYZ[\\]\033\177";

//----------------------------------------------------------------------------
// ASCII-STYLE NAMES
//----------------------------------------------------------------------------

static const char p_ini[] = "%&+,.:=_";
static const char p_mid[] = "%&+,.:=_- ";

/* Returns TRUE if the character C is in the set of valid initial
characters for a name.  */
static gboolean _val_init (char C)
{
    return ((C != '\0') && (isalnum(C) || strchr(p_ini, C) != NULL));
}

/* Returns TRUE if the character C is in the set of valid non-initial
characters for a name.  */
static gboolean _val_mid (char C)
{
    return ((C != '\0') && (isalnum(C) || strchr(p_mid, C) != NULL));
}

/* Returns TRUE if character array STR of length N contains only
safe characters.  STR need not be NULL-terminated, but, if it is
NULLs may only appear at the end. */
gboolean safeascii(const char *mem, size_t n)
{
    assert(mem != NULL);
    int state = 1;              /* init = 1, mid = 2, final = 3 */
    int bad = -1;

    for (size_t i = 0; i < n; i ++) {
        if (state == 1) {
            if (!_val_init(mem[i]))
                bad = i;
            state = 2;
        } else if (state == 2) {
            if (!_val_mid(mem[i]))
                state = 3;
        }

        /* Note that it is valid to go straight from INIT to FINAL */
        if (state == 3) {
            if (mem[i] != '\0')
                bad = i;
            /* We only check for one NULL */
            break;
        }
    }
    if (bad == -1)
        return TRUE;
    return FALSE;
}

//----------------------------------------------------------------------------
// PHONE-STYLE NAMES
//----------------------------------------------------------------------------

// This address "format" is really just a 32-bit number
// chopped up for easy reading.  It isn't really X.121.
// It is one of the following, but, the hyphens are required.
// AAA-BBBB-CC
// or AAA-BBBB
// if the CC's are missing, they are presumed to be zero
// Conceptually AAA-BBBB is "phone number" and CC is an
// "extension".

// Returns TRUE is character array STR holds a valid 'x121'-style address.
// STR need not be null terminated.
gboolean safe121 (const char *str, size_t n)
{
    gboolean ret = TRUE;

    int len = strnlen_s(str, n);

    if (len != 11 && len != 8)
        ret = FALSE;
    else {
        for (int i = 0; i < len; i ++) {
            if ((i == 3 || i == 8)  && str[i] != '-') {
                ret = FALSE;
                break;
            } else if (str[i] < '0' || str[i] > '9') {
                ret = FALSE;
                break;
            }
        }
    }
    return ret;
}

// Returns the integer-representation of an 'x121' name stored
// in character array STR of length N.
uint32_t pack121(const char *str, size_t n)
{
    assert(safe121 (str, n) == TRUE);

    uint32_t val = 0, scale = 100000000;
    assert(safe121(str, n));
    int len = strnlen_s(str, n);
    for (int i = 0; i < len; i ++) {
        val += (str[i] - '0') * scale;
        scale /= 10;
    }
    return val;
}

// Not thread safe, obviously.  Behold the laziness!
const char *unpack121(uint32_t x)
{
    static char str[12];
    str[11] = '\0';
    str[10] = '0' + (x % 10);
    x /= 10;
    str[9] = '0' + (x % 10);
    x /= 10;
    str[8] = '-';
    if (str[10] == '0' && str[9] == '0')
        str[8] = '\0';
    else
        str[8] = '-';
    str[7] = '0' + (x % 10);
    x /= 10;
    str[6] = '0' + (x % 10);
    x /= 10;
    str[5] = '0' + (x % 10);
    x /= 10;
    str[4] = '0' + (x % 10);
    x /= 10;
    str[3] = '-';
    str[2] = '0' + (x % 10);
    x /= 10;
    str[1] = '0' + (x % 10);
    x /= 10;
    str[0] = '0' + (x % 10);
    return str;
}

//----------------------------------------------------------------------------
// SEARCHING ORDERED UINT32_T ARRAYS
//----------------------------------------------------------------------------

// Search a matrix ARR of length N of strictly monotonically increasing
// integers. Return the location of X, or, if X is not found, return the
// location where X would be inserted.
size_t ifind(uint32_t arr[], size_t n, uint32_t X)
{
    size_t lo, hi, mid;

    lo = 0;
    hi = n;
    while (hi - lo > 1) {
        mid = (hi + lo) >> 1;
        if (X >= arr[mid])
            lo = mid;
        else
            hi = mid;
    }
    if (X <= arr[lo])
        return lo;
    return hi;
}

//----------------------------------------------------------------------------
// INDEX SORTING OF STRINGS
//----------------------------------------------------------------------------

// Give an array of strings ARR, and an array of indices that point to
// locations in ARR, and 3 locations into the array of indices, swap
// the three specified index elements so that the strings to which
// they point would be in lexicographic order.
static void sort3 (char *arr[], size_t indx[], size_t a, size_t b, size_t c)
{
    size_t tmp;
#define SWAP(a,b) tmp=(a);(a)=(b);(b)=tmp;

    if (strcmp(arr[indx[a]], arr[indx[c]]) > 0) {
        SWAP(indx[a], indx[c]);
    }
    if (strcmp(arr[indx[b]], arr[indx[c]]) > 0) {
        SWAP(indx[b], indx[c]);
    }
    if (strcmp(arr[indx[a]], arr[indx[b]]) > 0) {
        SWAP(indx[a],indx[b]);
    }
#undef SWAP
}

// Using a slow sort, create an index array INDX whose indices would
// sort the values ARR.  Only modify the entries between [left,right).
static void iisort(char *arr[], size_t indx[], size_t left, size_t right)
{
    size_t i, j;
    char *val;
    size_t index;

    for (j = left + 1; j < right; j ++) {
        index = indx[j];
        val = arr[index];
        i = j;
        while (i > left && strcmp(arr[indx[i-1]], val) > 0) {
            indx[i] = indx[i-1];
            i --;
        }
        indx[i] = index;
    }
}


// NSTACK is the depth of the stack used to hold Quicksort brackets.
#define NSTACK 50

// M is the size of a bracket where sort switches from Quick Sort to
// Insertion Sort.
#define M 7

// Using a Quicksort, create an index array INDX whose indices would
// sort the values ARR.  This is adapted from Numerical Recipes in C.
void qisort(char *arr[], size_t n, size_t indx[])
{
    size_t left = 0;
    size_t center;
    size_t right = n - 1;
    size_t i, j, index_cur, itemp;
    int stack_size = 0, *stack;
    char *value_cur;
#define SWAP(a,b) itemp=(a);(a)=(b);(b)=itemp;

    stack = (int *)calloc(NSTACK, sizeof(int));
    for(j = 0; j < n; j ++)
        indx[j] = j;

    for(;;) {
        // If our subarray is down to a handful of elements, we switch
        // to an insertion sort.
        if (right - left < M) {
            iisort(arr, indx, left, right);
            if (stack_size == 0)
                break;

            // Pop the stack and begin a new round of partitioning.
            right = stack[stack_size-1];
            stack_size --;
            left = stack[stack_size-1];
            stack_size --;
        } else {
            // Choose the center string of left, center, and right
            // elements as partitioning element.  Rearrange the three
            // elements in sorted order.
            center = (left + right) >> 1;
            SWAP(indx[center], indx[left + 1]);
            sort3(arr, indx, left, left + 1, right);

            i = left + 1;
            j = right;

            index_cur = indx[left+1];
            value_cur = arr[index_cur];
            for(;;) {
                // Scan up to find a string greater than target
                // string.
                do {
                    i++;
                } while(strcmp(arr[indx[i]], value_cur) < 0);
                // Scan down to find a string less that our target
                // string.
                do {
                    j--;
                } while(strcmp(arr[indx[j]], value_cur) > 0);
                // If the indices cross, partitioning is complete.
                if(j<i)
                    break;
                // Otherwise, exchange them.
                SWAP(indx[i],indx[j]);
            }
            indx[left+1] = indx[j];
            // Insert the index of the target string here.
            indx[j] = index_cur;
            stack_size += 2;
            // Push pointers to a larger subarray on the stack, and
            // process smaller subarray immediately.
            if(stack_size>NSTACK) abort();
            if (right - i + 1 >= j - left) {
                stack[stack_size-1] = right;
                stack[stack_size-2] = i;
                right = j - 1;
            } else {
                stack[stack_size-1] = j - 1;
                stack[stack_size-2] = left;
                left = i;
            }
        }
    }
    free (stack);
#undef SWAP
}

// Using an unordered matrix of stringx ARR of length N, an an index
// matrix whose contents order the string matrix return the location
// of the STR, or, if it is not found, the last location checked.  A
// return value of N indicates after the end of the matrix.
size_t namefind(const char *arr[], size_t n, size_t nidx[], const char *str)
{
    size_t lo, hi, mid;

    lo = 0;
    hi = n + 1;
    while (hi - lo > 1) {
        mid = (hi + lo) >> 1;
        if (strcmp(str, arr[nidx[mid-1]]) >= 0)
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}


//----------------------------------------------------------------------------
// UNIQUE-KEY VECTORS
//----------------------------------------------------------------------------
#if 0
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
size_t keyfind(ukey_t arr[], size_t n, ukey_t key)
{
    size_t lo, hi, mid;

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
static ukey_t _keynext(ukey_t arr[], size_t n, ukey_t key, size_t j)
{
    assert (n < UKEY_MAX);  // infinite loop when all integers are being used as keys
    assert (j <= n);

    while (j < n) {
        ukey_t k = arr[j];
        if (k < key)
            j ++;
        else if (k == key) {
            key ++;

            // Check if ID has numerically wrapped back to zero
            if (key == 0)
                j = 0;
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
index_ukey_t keynext(ukey_t arr[], size_t n, ukey_t key)
{
    index_ukey_t ret;
    int index;
    gboolean did_loop = FALSE;

loop:
    index = keyfind(arr, n, key);
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

// Used in the sort in indexx() below.
static int compu64(const void *x, const void *y)
{
    double_ukey_t a, b;
    a = *(double_ukey_t *)x;
    b = *(double_ukey_t *)y;
    if (a < b)
        return -1;
    else if (a == b)
        return 0;
    else
        return 1;
}

// Given an unsorted list of unique integers ARR and a workspace array
// INDX, it fills INDX with a list of indices into the original array
// that would put it in sorted order.  FIXME: memory inefficient.
// Could use the NumRec version.
void indexx(ukey_t arr[], size_t n, ukey_t indx[])
{
    double_ukey_t *arrindx = (double_ukey_t *) calloc(n, sizeof(double_ukey_t));

    // Pack the array value and index into one array.
    // The value is first so we end up sorting on it.
    for (double_ukey_t i = 0; i < n; i ++)
        arrindx[i] = ((double_ukey_t) arr[i]) << UKEY_SHIFT | i;
    qsort(arrindx, n, sizeof(double_ukey_t), compu64);
    for (double_ukey_t i = 0; i < n; i ++)
        indx[i] = (ukey_t) (arrindx[i] & UKEY_MASK);
    free (arrindx);
}
#endif

//----------------------------------------------------------------------------
// STRICT C11 TIME
//----------------------------------------------------------------------------

// Strict C11 is pretty weak when it comes to times, but, here's how
// you get an delta time.  It returns -1.0 if there is some mysterious
// failure.  The resolution of this is unspecified in C11.
double now()
{
    static int first = 1;
    static time_t time0;
    time_t time1, tret;

    if (first) {
        tret = time(&time0);
        if (tret == (time_t)-1)
            return -1.0;
        first = 0;
    }

    tret = time(&time1);
    if (tret == (time_t)-1)
        return -1.0;

    return difftime(time1, time0);
}

//----------------------------------------------------------------------------
// GLIB TIME
//----------------------------------------------------------------------------

char *
monotonic_time_to_string(gint64 T)
{
    GDateTime *date, *date2;
    double subseconds;
    // Compute date to the nearest second.
    date = g_date_time_new_from_unix_utc(T / G_USEC_PER_SEC);
    // Then add the subsecond fraction.
    subseconds = (double) (T % G_USEC_PER_SEC) / (double) G_USEC_PER_SEC;
    date2 = g_date_time_add_seconds(date, subseconds);
    gchar *str = g_date_time_format(date2, "%T");
    g_date_time_unref(date2);
    g_date_time_unref(date);
    return str;
}

//----------------------------------------------------------------------------
// ZeroMQ HELPERS
//----------------------------------------------------------------------------

zctx_t *
zctx_new_or_die (void)
{
    zctx_t *c = NULL;

    c = zctx_new();
    if (c == NULL) {
        g_error("failed to create a ZeroMQ context");
        exit(1);
    }
    return c;
}

void *zsocket_new_or_die(zctx_t *ctx, int type)
{
    void *sock;

    g_assert(ctx != NULL);

    sock = zsocket_new(ctx, type);
    if (sock == NULL) {
        g_error("failed to create a new ZeroMQ socket");
        exit(1);
    }
    return sock;
}

zloop_t *zloop_new_or_die(void)
{
    zloop_t *L = zloop_new();

    if (L == NULL) {
        g_error("failed to create a new ZeroMQ main loop");
        exit (1);
    }
    return L;
}

#if 0
#include <stdio.h>
int main()
{
    ukey_t x[] = {8, 6, 7, 5, 3, 0, 9};
    ukey_t idx[7];
    indexx(x, 7, idx);
    for (int i = 0; i < 7; i ++)
        printf("%d %d %d\n", i, x[i], idx[i]);
    for (int i = 0; i < 7; i ++)
        printf("%d %d\n", i, x[idx[i]]);

    char* strings[14] = {"now", "is", "the", "time", "for", "all", "good", "men", "to", "the", "aid", "of", "their", "country"};
    size_t indx[14];
    //insertion_sort(strings,indx,0,10);
    qisort(strings, 14, indx);
    for (int i = 0; i < 14; i ++)
        printf("%d %d %s\n", i, indx[i], strings[indx[i]]);
    return 0;
}
#endif
