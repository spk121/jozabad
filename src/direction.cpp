#include <cassert>

#include "../include/direction.h"
#include "../include/diagnostic.h"

char direction_name[direction_last + 1][DIRECTION_NAME_MAX_LEN] = {
    "BIDIRECTIONAL",
    "INPUT BARRED",
    "OUTPUT BARRED",
    "I/O BARRED"
};

bool
validate(direction_t i) {
    if ((i == direction_bidirectional)
        || (i == direction_input_barred)
        || (i == direction_output_barred)
        || (i == direction_io_barred))
        return true;
    diagnostic = d_direction_invalid;
    return false;
}

direction_t
throttle(direction_t request, direction_t limit) {
    assert (validate(request));
    assert (validate(limit));

    return (direction_t) ((unsigned int) request & (unsigned int) limit);
}

// A request can never be more lenient than the current state.
bool
negotiate(direction_t request, direction_t current) {
    assert (validate(request));
    assert (validate(current));

    if (((unsigned int) request | (unsigned int) current) != (unsigned int) request) {
        diagnostic = d_direction_invalid_negotiation;
        return false;
    }
    return true;
}

char const * const
name(direction_t i)
{
    assert(validate(i));
    return direction_name[i];
}

