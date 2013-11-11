#pragma once
#include <glib.h>

#define RAII_VARIABLE(vartype,varname,initval,dtor) \
    vartype varname __attribute__((cleanup(dtor))) = (initval)

void raii_option_context_free(GOptionContext **pcontext);
void raii_gcharp_free(gchar **pstr);
