/* Guidance
   - GCC - C11 only + no local includes
   - CL 11.0 - C11 subset of C++11 only (/TP) + no local includes
*/
#ifndef _PARCH_LIB_H_INCLUDE
#define	_PARCH_LIB_H_INCLUDE

#include <stdint.h>
#include <stdlib.h>

#include "bool.h"

//----------------------------------------------------------------------------
// BASICS
//----------------------------------------------------------------------------

typedef struct {
    bool_t flag;
    size_t index;
} bool_index_t;

size_t intlen(int x);

#if __GLIBC__ == 2
// This can be removed once Annex K hits GNU libc
size_t strnlen_s (const char *s, size_t maxsize);
#endif

//----------------------------------------------------------------------------
// ASCII-STYLE NAMES
//----------------------------------------------------------------------------
bool_t safeascii(const char *mem, size_t n);

//----------------------------------------------------------------------------
// PHONE-STYLE NAMES
//----------------------------------------------------------------------------
bool_t safe121 (const char *str, size_t n);
uint32_t pack121(const char *str, size_t n);
const char *unpack121(uint32_t x);

//----------------------------------------------------------------------------
// INDEX SORTING OF STRINGS
//----------------------------------------------------------------------------

void qisort(char *arr[], size_t n, size_t indx[]);
size_t namefind(const char *arr[], size_t n, size_t nidx[], const char *str);

//----------------------------------------------------------------------------
// SEARCHING ORDERED UINT32_T ARRAYS
//----------------------------------------------------------------------------

size_t ifind(uint32_t arr[], size_t n, uint32_t X);

//----------------------------------------------------------------------------
// UNIQUE-KEY VECTORS
//----------------------------------------------------------------------------
#if 0
#ifndef UKEY_WIDTH
# define UKEY_WIDTH 2
#endif

#if UKEY_WIDTH == 1
typedef uint8_t ukey_t;
#define UKEY_MIN UINT8_C(0)
#define UKEY_MAX UINT8_C(SCHAR_MAX)
#define UKEY_C(x) UINT8_C(x)
#elif UKEY_WIDTH == 2
typedef uint16_t ukey_t;
#define UKEY_MIN UINT16_C(0)
#define UKEY_MAX UINT16_C(INT16_MAX)
#define UKEY_C(x) UINT16_C(x)
#elif UKEY_WIDTH == 4
typedef uint32_t ukey_t;
#define UKEY_MIN UINT32_C(0)
#define UKEY_MAX UINT32_C(INT32_MAX)
#define UKEY_C(x) UINT32_C(x)
#else
# error "Bad UKEY_WIDTH"
#endif

typedef struct {
    size_t index;
    ukey_t key;
} index_ukey_t;

size_t keyfind(ukey_t arr[], size_t n, ukey_t key);
index_ukey_t keynext(ukey_t arr[], size_t n, ukey_t key);
void indexx(ukey_t arr[], size_t n, ukey_t indx[]);
#endif
//----------------------------------------------------------------------------
// STRICT C11 TIME
//----------------------------------------------------------------------------

double now(void);

#endif	/* LIB_H */

