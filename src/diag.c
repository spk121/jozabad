#include <errno.h>
#include <zmq.h>
#include "diag.h"

diag_t errno2diag()
{
    if (errno == EAGAIN)
        return d_zmq_eagain;
    else if (errno == ENOTSUP)
        return d_zmq_enotsup;
    else if (errno == EFSM)
        return d_zmq_efsm;
    else if (errno == ETERM)
        return d_zmq_eterm;
    else if (errno == ENOTSOCK)
        return d_zmq_enotsock;
    else if (errno == EINTR)
        return d_zmq_eintr;
    else if (errno == EFAULT)
        return d_zmq_efault;
    else
        return d_zmq_error;
}
