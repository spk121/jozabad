/* Guidance
   - GCC - C11 only + no local includes
   - CL 11.0 - C11 subset of C++11 only (/TP) + no local includes
*/
#ifndef _PARCH_LIB_H_INCLUDE
#define	_PARCH_LIB_H_INCLUDE

#ifndef WIN32
#include <stdbool.h>
#endif
#include <stdint.h>

//----------------------------------------------------------------------------
// ASCII-STYLE NAMES
//----------------------------------------------------------------------------
bool safeascii(const char *mem, size_t n);

//----------------------------------------------------------------------------
// PHONE-STYLE NAMES
//----------------------------------------------------------------------------
bool safe121 (const char *str, size_t n);
uint32_t pack121(const char *str, size_t n);
const char *unpack121(uint32_t x);

//----------------------------------------------------------------------------
// UNIQUE-KEY VECTORS
//----------------------------------------------------------------------------
typedef struct {
    int index;
    unsigned short key;
} index_key_t;
index_key_t keynext(unsigned short arr[], int n, unsigned short key);
void indexx(unsigned short arr[], int n, unsigned short indx[]);
#endif	/* LIB_H */

