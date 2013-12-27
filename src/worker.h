/*
    worker.h - connected peers

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
#pragma once

#include <czmq.h>
#include <assert.h>
#include <stdint.h>
#include "iodir.h"
#include "joza_msg.h"
#include "mylimits.h"

typedef enum {_READY, X_CALLER, Y_CALLEE} role_t;

typedef struct {
    wkey_t wkey;
    gchar *address;
    gchar *hostname;
    zframe_t *zaddr;
    iodir_t iodir;
    lcn_t lcn;
    role_t role;
    gint64 ctime;               /* time of creation */
    gint64 mtime;               /* time of last modification */
    gint64 atime;               /* time of last message received from or sent to worker */
} worker_t;


worker_t *worker_create(zframe_t *Z, char *A, char *N, iodir_t io);
const char *worker_get_address(const worker_t *W);
gint64 worker_get_atime(const worker_t *W) G_GNUC_CONST;
const char *worker_get_hostname(const worker_t *W);
const char *worker_get_iodir_str(const worker_t *W);
lcn_t worker_get_lcn(const worker_t *W);
role_t worker_get_role(const worker_t *W) G_GNUC_CONST;
zframe_t *worker_get_zaddr(const worker_t *W);
gboolean worker_is_allowed_incoming_call(const worker_t *W);
gboolean worker_is_available_for_call(const worker_t *W);
gboolean worker_is_x_caller(const worker_t *W);
void worker_set_lcn(worker_t *W, lcn_t L);
void worker_set_role(worker_t *W, role_t R);
void worker_set_role_to_ready(worker_t *W);
void worker_set_role_to_x_caller(worker_t *W, lcn_t lcn);
void worker_set_role_to_y_callee(worker_t *W, lcn_t lcn);
void worker_update_atime(worker_t *W);
void worker_update_mtime(worker_t *W);

