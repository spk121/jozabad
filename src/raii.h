#pragma once
#include <glib.h>
#include "poll.h"

#define RAII_VARIABLE(vartype,varname,initval,dtor) \
    vartype varname __attribute__((cleanup(dtor))) = (initval)

void raii_option_context_free(GOptionContext **pcontext);
void raii_gcharp_free(gchar **pstr);
void raii_pollp_destroy(joza_poll_t **poll);
