#include <assert.h>
#include "iodir.h"

const char iodir_names[4][22] = {
    "bidirectional",
    "incoming calls barred",
    "outgoing calls barred",
    "calls barred"
};


int iodir_validate(iodir_t x)
{
    if (x == io_bidirectional
            || x == io_incoming_calls_barred
            || x == io_outgoing_calls_barred)
        return 1;
    return 0;
}

const char *iodir_name(iodir_t x)
{
    assert(iodir_validate(x));
    return &iodir_names[x][0];
}
