#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "name.h"

/* Guidance
   - C11 only
   - MSC09-C. Character encoding: Use subset of ASCII for safety
*/

const static char p_ini[] = "%&+,.:=_";
const static char p_mid[] = " %&+,.:=_";

/* Returns TRUE if the character C is in the set of valid initial
   characters for a name.  */
static bool val_init (char C)
{
    return ((C != '\0') && (isalnum(C) || strchr(p_ini, C) != NULL));
}

/* Returns TRUE if the character C is in the set of valid non-initial
   characters for a name.  */
static bool val_mid (char C)
{
    return ((C != '\0') && (isalnum(C) || strchr(p_mid, C) != NULL));
}

/* Returns TRUE if the worker name N is valid. */
bool val_name(name_t N)
{
    int i;
  
    if (!val_init (N.str[0]))
        return false;
    i = 1;
    while (val_mid(N.str[i]) && i < NAME_LEN)
        i++;
    while (i < NAME_LEN)
        if (N.str[i] != '\0')
            return false;
        else
            i++;
    return true;
 }
        
        
