/* lib.c
These are functions or procedures
1. they act only on their parameters
2. require no external knowledge beyond C11 + stdlib

*/

/* Guidance
- GCC - C11 only + no local includes except own public header
- CL 11.0 - C11 subset of C++11 only (/TP) + no local includes except own public header
*/
#ifdef __GNUC__
# define _GNU_SOURCE
#endif

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#ifndef WIN32
# include <stdbool.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lib.h"

//----------------------------------------------------------------------------
// ANNEX K MISSING FUNCTIONS
//----------------------------------------------------------------------------

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
// ASCII-STYLE NAMES
//----------------------------------------------------------------------------

static const char p_ini[] = "%&+,.:=_";
static const char p_mid[] = "%&+,.:=_ ";

/* Returns TRUE if the character C is in the set of valid initial
characters for a name.  */
static bool _val_init (char C)
{
	return ((C != '\0') && (isalnum(C) || strchr(p_ini, C) != NULL));
}

/* Returns TRUE if the character C is in the set of valid non-initial
characters for a name.  */
static bool _val_mid (char C)
{
	return ((C != '\0') && (isalnum(C) || strchr(p_mid, C) != NULL));
}

/* Returns TRUE if character array STR of length N contains only
safe characters.  STR need not be NULL-terminated, but, if it is
NULLs may only appear at the end. */
bool safeascii(const char *mem, size_t n)
{
	assert(mem != NULL);
	size_t i;

	if (n == 0)
		return true;
	if (!_val_init(mem[0]))
		return false;
	i = 1;
	while (_val_mid(mem[i]) && i < n)
		i++;
	while (i < n)
		if (mem[i] != '\0')
			return false;
		else
			i++;
	return true;
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
bool safe121 (const char *str, size_t n)
{
	bool ret = true;

	int len = strnlen_s(str, n);

	if (len != 11 && len != 8)
		ret = false;
	else
	{
		for (int i = 0; i < len; i ++)
		{
			if ((i == 3 || i == 8)  && str[i] != '-')
			{
				ret = false;
				break;
			}
			else if (str[i] < '0' || str[i] > '9')
			{
				ret = false;
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
	assert(safe121 (str, n) == true);

	uint32_t val = 0, scale = 100000000;
	assert(safe121(str, n));
	int len = strnlen_s(str, n);
	for (int i = 0; i < len; i ++)
	{
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
	hi = n + 1;
	while (hi - lo > 1)
	{
		mid = (hi + lo) >> 1;
		if (X >= arr[mid - 1])
			lo = mid;
		else
			hi = mid;
	}
	return lo;
}

//----------------------------------------------------------------------------
// INDEX SORTING OF STRINGS
//----------------------------------------------------------------------------


#define NSTACK 50
#define M 7

// Give an array of strings ARR, and an array of indices that point to locations in ARR,
// and 3 locations into the array of indices, swap the three specified index elements
// so that the strings to which they point would be in lexicographic order.
static void sort3 (char *arr[], unsigned long indx[], unsigned long a, unsigned long b, unsigned long c)
{
	unsigned long tmp;
	if (strcmp(arr[indx[a]], arr[indx[c]]) > 0) {
		tmp = indx[a];
		indx[a] = indx[c];
		indx[c] = tmp;
	}
	if (strcmp(arr[indx[b]], arr[indx[c]]) > 0) {
		tmp = indx[b];
		indx[b] = indx[c];
		indx[c] = tmp;
	}
	if (strcmp(arr[indx[a]], arr[indx[b]]) > 0) {
		tmp = indx[a];
		indx[a] = indx[b];
		indx[b] = tmp;
	}
}

/* Using a slow sort, create an index array INDX whose indices would sort
the values ARR.  Only modify the entries between [left,right). */
static void insertion_sort(char *arr[], unsigned long indx[], unsigned long left, unsigned long right)
{
	unsigned long i, j;
	char *val;
	unsigned long index;

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

/* Using a Quicksort, create an index array INDX whose indices would sort
the values ARR.  This is adapted from Numerical Recipes in C. */
void sindex(unsigned long n, char *arr[], unsigned long indx[])
{
	unsigned long i, indxt, right = n - 1, itemp, j, center, left = 0;
	int jstack = 0, *istack;
	char *val;
#define SWAP(a,b) itemp=(a);(a)=(b);(b)=itemp;

	istack = (int *)calloc(NSTACK, sizeof(int));
	for(j = 0; j < n; j ++)
		indx[j] = j;

	for(;;) {
		// If our subarray is down to a handful of elements,
		// we switch to an insertion sort
		if (right - left < M) {
			insertion_sort(arr, indx, left, right);
			if (jstack == 0)
				break;
			/* Pop stack and begin a new round of partitioning */
			right = istack[jstack-1];
			jstack --;
			left = istack[jstack-1];
			jstack --;
		} else {
			/* Choose median of left, center, and right elements
			as partitioning element A.  Also
			rearrange so that a[left] <= a[left+1] <= a[right]
			*/

			center = (left + right) >> 1;
			SWAP(indx[center], indx[left + 1]);
			sort3(arr, indx, left, left + 1, right);

			/* Initialize pointers for partitioning */
			i = left + 1;
			j = right;

			/* This is the partitioning element */
			indxt = indx[left+1];
			val = arr[indxt];
			for(;;) {
				/* Scan up to find element > a */
				do {
					i++;
				} while(strcmp(arr[indx[i]], val) < 0);
				/* Scan down to find element < a*/
				do { 
					j--; 
				} while(strcmp(arr[indx[j]], val) > 0);
				/* If the pointers are crossed, partitioning is complete */
				if(j<i) 
					break;
				/* Otherwise exchange them */
				SWAP(indx[i],indx[j]);
			}
			indx[left+1] = indx[j];
			/* Then insert the partitioning element here */
			indx[j] = indxt;
			jstack += 2;
			/* Push pointers to a larger subarray on the stack, and process
			smaller subarray immediately */
			if(jstack>NSTACK) abort();
			if (right - i + 1 >= j - left) {
				istack[jstack-1] = right;
				istack[jstack-2] = i;
				right = j - 1;
			}else{
				istack[jstack-1] = j - 1;
				istack[jstack-2] = left;
				left = i;
			}
		}
	}
	free (istack);
}


//----------------------------------------------------------------------------
// UNIQUE-KEY VECTORS
//----------------------------------------------------------------------------

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


// Using a bisection search on a matrix ARR of length N,
// which is supposed to contain a strictly monotonically increasing
// list of unique key integers,
// return the location of the key, or, if it is not found,
// the location where the key would be inserted.
// A return value of N indicates after the end of the matrix.
size_t keyfind(ukey_t arr[], size_t n, ukey_t key)
{
	size_t lo, hi, mid;

	lo = 0;
	hi = n + 1;
	while (hi - lo > 1)
	{
		mid = (hi + lo) >> 1;
		if (key >= arr[mid - 1])
			lo = mid;
		else
			hi = mid;
	}
	return lo;
}

// Given a table ARR of strictly monotonically increasing integers that function
// as unique keys, and a desired KEY,
// this searches the array to ensure that KEY does not appear in the array.  If it
// doesn't appear in the array, KEY is returned.  If it does appear, the next avaialble
// unique integer key is returned.  J is an array index that is the starting point
// of the search.
static ukey_t _keynext(ukey_t arr[], size_t n, ukey_t key, size_t j)
{
	assert (n < UKEY_MAX);  // infinite loop when all integers are being used as keys
	assert (j <= n);

	while (j < n)
	{
		ukey_t k = arr[j];
		if (k < key)
			j ++;
		else if (k == key) {
			key ++;

			// Check if ID has numerically wrapped back to zero
			if (key == 0)
				j = 0;
		}
		else
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
	bool did_loop = false;

loop:
	index = keyfind(arr, n, key);
	if (index > 0)
	{
		// If this ID is a duplicate, keyfind() will have suggested
		// that it be inserted to the right of an entry with the same
		// ID.  So we check the entry to the left to see if this ID
		// is a duplicate, and then increment it if necessary.
		if (arr[index - 1] == key) {
			assert (!did_loop);
			key = _keynext(arr, n, key, index - 1);
			did_loop = true;
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

// Given an unsorted list of unique integers ARR and
// a workspace array INDX, it fills INDX with a list of
// indices into the original array that would put it
// in sorted order.
// FIXME: memory inefficient.  Could use the NumRec version.
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

#if 1
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
	unsigned long indx[14];
	//insertion_sort(strings,indx,0,10);
	sindex(14, strings, indx);
	for (int i = 0; i < 14; i ++)
		printf("%d %d %s\n", i, indx[i], strings[indx[i]]);
	return 0;
}
#endif
