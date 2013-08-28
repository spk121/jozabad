#include "../include/direction.h"
#include "../include/diagnostic.h"

bool
parch_direction_validate(direction_t i) {
    if ((i == direction_bidirectional)
        || (i == direction_input_barred)
        || (i == direction_output_barred)
        || (i == direction_io_barred))
        return true;
    diagnostic = d_direction_invalid;
    return false;
}

direction_t
parch_direction_throttle(direction_t request, direction_t limit) {
    assert (parch_direction_validate(request));
    assert (parch_direction_validate(limit));

    return (unsigned int) request & (unsigned int) limit;
}

// A request can never be more lenient than the current state.
bool
parch_direction_negotiate(direction_t request, direction_t current) {
    assert (parch_direction_validate(request));
    assert (parch_direction_validate(current));

    if (((unsigned int) request | (unsigned int) current) != (unsigned int) request) {
        diagnostic = d_direction_invalid_negotiation;
        return false;
    }
    return true;
}
