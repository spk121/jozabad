#include "parch.h"

parch_msg_t *
parch_msg_new_clear_request_msg(zframe_t *address, byte cause, byte diagnostic) {
    parch_msg_t *msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_address(msg, address);
    parch_msg_set_cause(msg, cause);
    parch_msg_set_diagnostic(msg, diagnostic);
    return msg;
}

parch_msg_t *
parch_msg_new_reset_request_msg(zframe_t *address, byte cause, byte diagnostic) {
    parch_msg_t *msg = parch_msg_new(PARCH_MSG_RESET_REQUEST);
    parch_msg_set_address(msg, address);
    parch_msg_set_cause(msg, cause);
    parch_msg_set_diagnostic(msg, diagnostic);
    return msg;
}

parch_msg_t *
parch_msg_new_disconnect_indication_msg(zframe_t *address, byte cause, byte diagnostic) {
    parch_msg_t *msg = parch_msg_new(PARCH_MSG_DISCONNECT_INDICATION);
    parch_msg_set_address(msg, address);
    parch_msg_set_cause(msg, cause);
    parch_msg_set_diagnostic(msg, diagnostic);
    return msg;
}

bool
parch_msg_validate_connect_request(parch_msg_t *self) {
    bool ok = true;
    if (parch_msg_incoming(self) == 1 && parch_msg_outgoing(self) == 1) {
        // A channel can't bar traffic in both directions
        ok = false;
    } else if (parch_msg_throughput(self) < THROUGHPUT_CLASS_MIN || parch_msg_throughput(self) > THROUGHPUT_CLASS_MAX) {
        ok = false;
    } else if (parch_msg_packet(self) < PACKET_CLASS_MIN || parch_msg_packet(self) > PACKET_CLASS_MAX) {
        ok = false;
    } else if (parch_msg_window(self) < WINDOW_MIN || parch_msg_window(self) > WINDOW_MAX) {
        ok = false;
    }
    return ok;
}

void
parch_msg_apply_defaults_to_connect_request(parch_msg_t *self) {
    if (parch_msg_window(self) == 0)
        parch_msg_set_window(self, WINDOW_DEFAULT);
    if (parch_msg_packet(self) == 0)
        parch_msg_set_packet(self, PACKET_CLASS_DEFAULT);
}

void
parch_msg_swap_incoming_and_outgoing(parch_msg_t *self) {
    byte tmp = parch_msg_incoming(self);
    parch_msg_set_incoming(self, parch_msg_outgoing(self));
    parch_msg_set_outgoing(self, tmp);
}