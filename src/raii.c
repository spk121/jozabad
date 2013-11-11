#include <glib.h>
#include "raii.h"

void raii_option_context_free(GOptionContext **pcontext)
{
    if (*pcontext)
        g_option_context_free(*pcontext);
    *pcontext = NULL;
}

void raii_gcharp_free(gchar **pstr)
{
    if (*pstr)
        g_free(*pstr);
    *pstr = NULL;
}


