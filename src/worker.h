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
#ifndef JOZA_WORKER_H
#define JOZA_WORKER_H

#include <czmq.h>
#include <assert.h>
#include <stdint.h>
#include "iodir.h"
#include "ukey.h"
#include "joza_msg.h"
#include "mylimits.h"
#include "bool.h"

typedef struct {
    bool_t flag;
    worker_idx_t index;
} bool_index_t;

typedef enum {READY, X_CALLER, Y_CALLEE} role_t;

extern wkey_t   w_wkey[WORKER_COUNT];
extern char     w_name[WORKER_COUNT][NAME_LEN + 1];
extern zframe_t *w_zaddr[WORKER_COUNT];
extern iodir_t  w_iodir[WORKER_COUNT];
extern lcn_t    w_lcn[WORKER_COUNT];
extern role_t   w_role[WORKER_COUNT];

wkey_t worker_add(const zframe_t *A, const char *N, iodir_t I);
bool_index_t worker_get_idx_by_key(uint32_t key);
bool_t worker_dispatch_by_idx (joza_msg_t *M, worker_idx_t I);
void remove_worker_by_key(wkey_t key);
void remove_worker(wkey_t key);
#endif
