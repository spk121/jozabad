#include "parch.h"

parch_msg_t *
parch_msg_new_clear_request_msg (zframe_t *address, byte cause, byte diagnostic)
{
    parch_msg_t *msg = parch_msg_new(PARCH_MSG_CLEAR_REQUEST);
    parch_msg_set_address(msg, address);
    parch_msg_set_cause(msg, cause);
    parch_msg_set_diagnostic(msg, diagnostic);
    return msg;
}

parch_msg_t *
parch_msg_new_reset_request_msg (zframe_t *address, byte cause, byte diagnostic)
{
    parch_msg_t *msg = parch_msg_new(PARCH_MSG_RESET_REQUEST);
    parch_msg_set_address(msg, address);
    parch_msg_set_cause(msg, cause);
    parch_msg_set_diagnostic(msg, diagnostic);
    return msg;
}
