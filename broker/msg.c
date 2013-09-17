#include <stdint.h>
#include <czmq.h>
#include "msg.h"

// Deep diving into the ZeroMQ source says that for ZeroMQ 3.x,
// bytes 1 to 5 of the address would work as a unique ID for a
// given connection.
uint32_t msg_addr2hash (const zframe_t *z)
{
    uint32_t x[1];

    memcpy(x, (char *) zframe_data((zframe_t *) z) + 1, sizeof(uint32_t));

    return x[0];
}