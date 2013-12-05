#include <glib.h>
#include "raii.h"
#include "poll.h"

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

void raii_pollp_destroy(joza_poll_t **pollp)
{
    if (*pollp)
        poll_destroy(*pollp);
    *pollp = NULL;
}


