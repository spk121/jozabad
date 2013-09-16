/* Guidance
   - GCC - C11 only
   - MSC09-C. Character encoding: Use subset of ASCII for safety
*/

#include <assert.h>
#include <ctype.h>
#ifndef WIN32
# include <stdbool.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "name.h"

/* Returns TRUE if the worker name N is valid. */
#if 0
bool val_name(name_t N)
{
	return safeascii(N.str, NAME_LEN);
 }
#endif
        
        
