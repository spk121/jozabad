#include <glib.h>
#include "diag.h"
#include "name.h"

/* Returns TRUE if the worker name N is valid. */
diag_t val_name(gchar *N)
{
    if (strnlen_s(N, NAME_LEN + 1) == 0)
    {
        return d_calling_address_too_short;
    }
    else if (strnlen_s(N, NAME_LEN + 1) > NAME_LEN)
    {
        return d_calling_address_too_long;
    }
    else if (!safeascii(N, NAME_LEN))
    {
        return d_calling_address_format_invalid;
    }
    return d_unspecified;
}
#endif


