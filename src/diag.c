/*
    diag.c - diagnostics for diagnostic messages

    Copyright 2013 Michael L. Gran <spk121@yahoo.com>

    This file is part of Jozabad.

    Jozabad is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jozabad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jozabad.  If not, see <http://www.gnu.org/licenses/>.

*/

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
