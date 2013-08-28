#include "../include/parch.h"

bool
parch_window_validate(uint32_t i) {
    if (i == 0)
        return true;
    else if (i >= WINDOW_MIN || i <= WINDOW_MAX)
        return true;
    return false;
}

uint32_t
parch_window_apply_default(uint32_t i) {
    assert(parch_window_validate(i));

    if (i == 0)
        return WINDOW_DEFAULT;
    return i;
}

uint32_t
parch_window_throttle(uint32_t request, uint32_t limit) {
    assert(parch_window_validate(request));
    assert(parch_window_validate(limit));
    uint8_t request2 = parch_window_apply_default(request);
    uint8_t limit2 = parch_window_apply_default(limit);
    if (request2 < limit2)
        return request2;
    return limit2;
}

// Negotiation can't increase the window size.  An exception is when it is 1: in that case
// it can be increased only up to 2.
parch_window_negotiation_t
parch_window_negotiate(uint32_t request, uint32_t current) {
    assert(parch_window_validate(request));
    assert(parch_window_validate(current));

    parch_window_negotiation_t ret;
    uint8_t request2 = parch_window_apply_default(request);
    uint8_t current2 = parch_window_apply_default(current);
    if (current2 >= WINDOW_NOMINAL && (request2 > current2 || request2 < WINDOW_NOMINAL)) {
        ret.ok = false;
        ret.size = current2;
    } else if (current2 <= WINDOW_NOMINAL && (request2 < current2 || request2 > WINDOW_NOMINAL)) {
        ret.ok = false;
        ret.size = current2;
    } else {
        ret.ok = true;
        ret.size = request2;
    }
    return ret;
}
