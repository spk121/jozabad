/*
  worker.c - connected peers

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

#include <glib.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <czmq.h>
#include "worker.h"
#include "packet.h"
#include "tput.h"
#include "iodir.h"
#include "seq.h"
#include "cause.h"
#include "diag.h"
#include "channel.h"
#include "packet.h"
#include "poll.h"
#include "msg.h"
#include "joza_msg.h"
#include "lib.h"

/*
  These arrays form a key table whose data is keyed either by
  a string key of the ZMQ address or by name.

  wkey  wkey_t   [primary key]    a key created from the ZMQ Router identity
  name  char[17]   [secondary key]  a string name for the connection
  zaddr zframe_t *                  a ZMQ frame containing a ZMQ Router identity
  iodir iodir_t                     whether this worker makes or receives calls
  lcn   lcn_t                       Logical Channel Number
  role  role_t                      X (caller), Y (callee), or READY
  ctime double                    time this worker was created
  mtime double                    time of last message dispatched by this worker

  nidx  worker_idx_t     [secondary key]  an array that keeps the sort indices
  for the secondary key
*/

worker_t *worker_create(zframe_t *Z, char *A, char *N, iodir_t io)
{
    worker_t *worker = NULL;
    wkey_t key = 0;
    guint64 elapsed_time = g_get_monotonic_time();

    g_message("In %s(Z = %p, A = %s, N = %s, io = %s)", __FUNCTION__, (void *) Z, A, N, iodir_name(io));

    key = msg_addr2key(Z);
    // First, validate the message
    worker = g_new0(worker_t, 1);
    worker->wkey = key;
    worker->address = g_strdup(A);
    worker->hostname = g_strdup(N);
    worker->zaddr = zframe_dup(Z);
    worker->iodir = io;
    worker->lcn = 0;
    worker->role = _READY;
    worker->ctime = elapsed_time;
    worker->mtime = elapsed_time;
    worker->atime = elapsed_time;
    return worker;
}

const char *worker_get_address(const worker_t *W)
{
    return W->address;
}

gint64 worker_get_atime(const worker_t *W)
{
    return W->atime;
}

const char *worker_get_hostname(const worker_t *W)
{
    return W->hostname;
}

const char *worker_get_iodir_str(const worker_t *W)
{
    return iodir_name(W->iodir);
}

lcn_t worker_get_lcn(const worker_t *W)
{
    return W->lcn;
}

role_t worker_get_role(const worker_t *W)
{
    return W->role;
}

zframe_t *worker_get_zaddr(const worker_t *W)
{
    return W->zaddr;
}

gboolean worker_is_allowed_incoming_call(const worker_t *W)
{
    return iodir_incoming_calls_allowed (W->iodir);
}

gboolean worker_is_available_for_call(const worker_t *W)
{
    return worker_is_allowed_incoming_call(W) && W->role == _READY;
}

gboolean worker_is_x_caller(const worker_t *W)
{
    return W->role == X_CALLER;
}

void worker_set_lcn(worker_t *W, lcn_t L)
{
    worker_update_mtime(W);
    W->lcn = L;
}

void worker_set_role(worker_t *W, role_t R)
{
    worker_update_mtime(W);
    W->role = R;
}

void worker_set_role_to_ready(worker_t *W)
{
    W->lcn = LCN_C(0);
    W->role = _READY;
}

void worker_set_role_to_x_caller(worker_t *W, lcn_t lcn)
{
    W->lcn = lcn;
    W->role = X_CALLER;
}

void worker_set_role_to_y_callee(worker_t *W, lcn_t lcn)
{
    W->lcn = lcn;
    W->role = Y_CALLEE;
}

void worker_update_atime(worker_t *W)
{
    W->atime = g_get_monotonic_time();
}

void worker_update_mtime(worker_t *W)
{
    W->mtime = g_get_monotonic_time();
}



