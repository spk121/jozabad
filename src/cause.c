#include "cause.h"

static const char cause_names[c_last + 1][30] = {
    "C_UNSPECIFIED",
    "C_WORKER_ORIGINATED",
    "C_NUMBER_BUSY",
    "C_CALL_COLLISION",
    "C_ZMQ_SENDMSG_ERR",
    "C_MALFORMED_MSG",
    "C_INVALID_FACILITY_REQUEST",
    "C_INVALID_FORWARDING_REQUEST",
    "C_ACCESS_BARRED",
    "C_ADDRESS_IN_USE",
    "C_UNKNOWN_ADDRESS",
    "C_NETWORK_CONGESTION",
    "C_LOCAL_PROCEDURE_ERROR",
    "C_REMOTE_PROCEDURE_ERROR",
    "C_QUOTA_EXCEEDED"
};

const char *cause_name(cause_t c)
{
    return &(cause_names[c][0]);
}

