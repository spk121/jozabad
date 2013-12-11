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

typedef enum {READY, X_CALLER, Y_CALLEE} role_t;

typedef struct {
    wkey_t wkey;
    gchar *name;
    zframe_t *zaddr;
    iodir_t iodir;
    lcn_t lcn;
    role_t role;
    gint64 ctime;               /* time of creation */
    gint64 mtime;               /* time of last modification */
    gint64 atime;               /* time of last access */
} worker_t;


worker_t *worker_create(zframe_t *A, char *N, iodir_t io);
