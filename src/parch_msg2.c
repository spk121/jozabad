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
parch_msg_validate_connect_request(parch_msg_t *self, diagnostic_t *diag) {
    bool ok = true;
    *diag = err_none;
    if (parch_msg_incoming(self) == 1 && parch_msg_outgoing(self) == 1) {
        // A channel can't bar traffic in both directions
        ok = false;
        *diag = err_data_barred;
    } else if (!parch_throughput_index_validate(parch_msg_throughput(self))) {
        ok = false;
        *diag = err_throughput_out_of_range;
    } else if (parch_msg_packet(self) < PACKET_CLASS_MIN || parch_msg_packet(self) > PACKET_CLASS_MAX) {
        ok = false;
        *diag = err_packet_size_out_of_range;
    } else if (parch_msg_window(self) < WINDOW_MIN || parch_msg_window(self) > WINDOW_MAX) {
        ok = false;
        *diag = err_window_size_out_of_range;
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

static inline char *
s_msg_address_strhex(parch_msg_t *msg) {
    return zframe_strhex(parch_msg_address(msg));
}

static inline char *
s_msg_service_name_dup(parch_msg_t *msg) {
    return strdup(parch_msg_service(msg));
}

static inline zframe_t *
s_msg_address_dup(parch_msg_t *msg) {
    return zframe_dup(parch_msg_address(msg));
}

int
parch_msg_id_is_valid(parch_msg_t *self) {
    int id = parch_msg_id(self);
    return (id == PARCH_MSG_DATA
            || id == PARCH_MSG_RR
            || id == PARCH_MSG_RNR
            || id == PARCH_MSG_CALL_REQUEST
            || id == PARCH_MSG_CALL_ACCEPTED
            || id == PARCH_MSG_CLEAR_REQUEST
            || id == PARCH_MSG_CLEAR_CONFIRMATION
            || id == PARCH_MSG_RESET_REQUEST
            || id == PARCH_MSG_RESET_CONFIRMATION
            || id == PARCH_MSG_CONNECT
            || id == PARCH_MSG_CONNECT_INDICATION
            || id == PARCH_MSG_DISCONNECT
            || id == PARCH_MSG_DISCONNECT_INDICATION);
}