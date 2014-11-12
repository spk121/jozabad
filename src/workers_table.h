/*
    worker_table.h - a collection of workers

    Copyright 2013, 2014 Michael L. Gran <spk121@yahoo.com>

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

/**
 * @file workers_table.h
 * @author Mike Gran
 * @brief A hash table of all connected workers.
 *
 * These are functions that manipulate a hash table that contains information
 * about all the workers connected to the server.
 */

#pragma once

#include <glib.h>
#include <czmq.h>
#include "worker.h"
#include "iodir.h"

typedef GHashTable workers_table_t;

workers_table_t *
workers_table_create(void);
void
workers_table_destroy(workers_table_t **p_workers_table);
void
workers_table_foreach(workers_table_t *workers_table, void func(gint key, worker_t *worker, gpointer user_data), gpointer user_data);
gboolean
workers_table_is_empty(workers_table_t *workers_table);
gboolean
workers_table_is_full(workers_table_t *workers_table);
worker_t *
workers_table_add_new_worker(workers_table_t *workers_table, gint key, zframe_t *zaddr, const char *address, const char *hostname, iodir_t iodir);
worker_t *
workers_table_lookup_by_address(workers_table_t *workers_table, const char *address);
worker_t *
workers_table_lookup_by_key(workers_table_t *workers_table, gint key);
worker_t *
workers_table_lookup_other(workers_table_t *workers_table, worker_t *worker);
void
workers_table_remove_by_key(workers_table_t *workers_table, gint key);
void
workers_table_remove_unused(workers_table_t *workers_table);
zhash_t *
workers_table_create_directory_zhash(workers_table_t *workers_table);

void
workers_table_dump(workers_table_t *workers_table);


