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
 * @brief A hash table of all connected workers.
 *
 * The workers table is a hash table of workers whose key is a unique
 * ID scraped from the ZeroMQ address.  It holds all the workers
 * connected to a server, whether or not they are connected to a
 * channel.
 */

#ifndef JOZA_WORKERS_TABLE_H
#define JOZA_WORKERS_TABLE_H

#include <glib.h>
#include <czmq.h>
#include "worker.h"
#include "iodir.h"

/**
 * @brief The type for the hash table used for the workers
 */
typedef GHashTable workers_table_t;

/**
 * @brief Creates a new empty workers table.
 *
 * Allocates a new empty hash table whose key is an integer and whose
 * value is a pointer to a #worker_t.
 * @return A new, empty hash table.
 */
G_GNUC_INTERNAL
workers_table_t *workers_table_create(void) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Destroy a #workers_table_t
 *
 * Deallocates the hash table and sets its pointer to NULL.
 *
 * @param p_workers_table  A pointer to a workers table.
 */
void
workers_table_destroy(workers_table_t **p_workers_table);

/**
 * @brief Return TRUE is the workers_table is empty
 *
 * @param workers_table
 * @return TRUE if the workers_table has zero entries.  FALSE otherwise.
 */
gboolean
workers_table_is_empty(workers_table_t *workers_table);

/**
 * @brief Return TRUE is the workers_table is full.
 *
 * A table is considered full if it has WORKER_COUNT entries.
 *
 * @param workers_table
 * @return TRUE if the workers_table is full.  FALSE otherwise.
 */
gboolean
workers_table_is_full(workers_table_t *workers_table);

/**
 * @brief Creates a new worker and adds it to the worker hash table
 *
 * The key for the hash table is the wkey_t extracted from the ZeroMQ address
 *
 * @param workers_table  The workers table to which the worker is to be added
 * @param key  The unique key that identifies the worker
 * @param zaddr  a CZMQ frame containing a ZMQ router identity for this worker
 * @param address  a plain-text identifier for the worker
 * @param hostname  additional identity information for the worker
 * @param iodir  indicates whether the worker can place calls or receive them
 * @return An new worker, or NULL on failure. Do not free.
 */
worker_t *
workers_table_add_new_worker(workers_table_t *workers_table, wkey_t key, zframe_t *zaddr,
                             const char *address, const char *hostname, iodir_t iodir);

/**
 * @brief Searches the workers table for a worker with a given address
 *
 * @param workers_table  The workers table to be searched
 * @param address  a plain-text identifier for the desired worker
 * @return The matching worker, or NULL on failure. Do not free.
 */
worker_t *
workers_table_lookup_by_address(workers_table_t *workers_table, const char *address);

/**
 * @brief Searches the workers table for a worker with a given key
 *
 * @param workers_table  The workers table to be searched
 * @param key  a unique key that identifies the worker
 * @return The matching worker, or NULL on failure. Do not free.
 */
worker_t *
workers_table_lookup_by_key(workers_table_t *workers_table, wkey_t key);

/**
 * @brief Searches the workers table for a worker with the same logical channel number
 *
 * @param workers_table  The workers table to be searched
 * @param worker  an existing worker that must already be in the table
 * @return A worker with a matching LCN, or NULL on failure. Do not free.
 */
worker_t *
workers_table_lookup_other(workers_table_t *workers_table, worker_t *worker);

/**
 * @brief Removes a worker with a given key for the workers table
 *
 * @param workers_table  The workers table to be searched
 * @param key  a unique key that identifies the worker
 */
void
workers_table_remove_by_key(workers_table_t *workers_table, wkey_t key);

/**
 * @brief Creates a text hash table of all the current workers and their status
 *
 * This hash table is used in the DIRECTORY message.
 *
 * @param workers_table  The workers table to be described
 * @return A CZMQ text hash table with all the workers and their status
 */
zhash_t *
workers_table_create_directory_zhash(workers_table_t *workers_table);

/**
 * @brief Prints the current workers table to stdout
 *
 * @param workers_table  The workers table to be described
 */
void
workers_table_dump(workers_table_t *workers_table);

#endif
