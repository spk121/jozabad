#include "../include/window.h"
#include "../include/diagnostic.h"

bool
window_validate(uint16_t i) {
    if (i == 0)
        return true;
    else if (i < WINDOW_MIN) {
        diagnostic = d_window_too_small;
        return false;
    }
    else if (i > WINDOW_MAX) {
        diagnostic = d_window_too_large;
        return false;
    }
    return true;
}

uint16_t
window_apply_default(uint16_t i) {
    assert(window_validate(i));
    if (i == 0)
        return WINDOW_DEFAULT;
    return i;
}

uint16_t
window_throttle(uint16_t request, uint16_t limit) {
    assert(window_validate(request));
    assert(window_validate(limit));
    uint8_t request2 = window_apply_default(request);
    uint8_t limit2 = window_apply_default(limit);
    if (request2 < limit2)
        return request2;
    return limit2;
}

// Negotiation can't increase the window size.  An exception is when it is 1: in that case
// it can be increased only up to 2.
bool
window_negotiate(uint16_t request, uint16_t current) {
    assert(window_validate(request));
    assert(window_validate(current));

    uint8_t request2 = window_apply_default(request);
    uint8_t current2 = window_apply_default(current);
    if (current2 >= WINDOW_NOMINAL && (request2 > current2 || request2 < WINDOW_NOMINAL)) {
        diagnostic = d_window_invalid_negotiation;
        return false;
    }
    else if (current2 <= WINDOW_NOMINAL && (request2 < current2 || request2 > WINDOW_NOMINAL)) {
        diagnostic = d_window_invalid_negotiation;
        return false;
    }
    return true;
}
