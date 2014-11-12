/*
    workers_table.c - a collection of workers

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

#include <glib.h>
#include <inttypes.h>
#include "workers_table.h"

#define WORKER_REMOVAL_TIMEOUT (30*1000)  // milliseconds

static void (*foreach_method)(gint key, worker_t *worker, gpointer user_data);

// Used in a hash-table iterator to find a worker who's address
// matches the string in USER_DATA.
static gboolean
s_compare_worker_to_name_(gpointer packed_key G_GNUC_UNUSED,
                          gpointer value,
                          gpointer user_data)
{
    // gint key = GPOINTER_TO_INT(packed_key);
    worker_t *w = value;
    gchar *str = user_data;
    if (strcmp(str, worker_get_address(w)) == 0)
        return true;
    else
        return false;
}

static gboolean
s_cull_dead_worker_(gpointer packed_key G_GNUC_UNUSED, gpointer value,
                    gpointer user_data G_GNUC_UNUSED)
{
    worker_t *worker = value;
    gint64 delta_t = (g_get_monotonic_time() - worker->atime) / 1000;
    if (delta_t > WORKER_REMOVAL_TIMEOUT) {
        g_message("removing worker %s: %" PRId64 " ms since last access",
                  worker_get_address(worker),
                  delta_t);
        return TRUE;
    }
    return FALSE;
}

static void
s_foreach_func_(gpointer packed_key, gpointer value, gpointer user_data)
{
    gint key = GPOINTER_TO_INT(packed_key);
    worker_t *worker = value;
    foreach_method(key, worker, user_data);
}

workers_table_t *
workers_table_create()
{
    return g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, g_free);
}

void
workers_table_destroy(workers_table_t **p_workers_table)
{
    g_hash_table_destroy(*p_workers_table);
    p_workers_table = NULL;
}

void
workers_table_foreach(workers_table_t *workers_table,
                      void func(gint key, worker_t *worker, gpointer user_data),
                      gpointer user_data)
{
    foreach_method = func;
    g_hash_table_foreach(workers_table, s_foreach_func_, user_data);
    foreach_method = NULL;
}

gboolean
workers_table_is_empty(workers_table_t *workers_table)
{
    return g_hash_table_size(workers_table) == 0;
}

gboolean
workers_table_is_full(workers_table_t *workers_table)
{
    return g_hash_table_size(workers_table) >= WORKER_COUNT;
}

worker_t *
workers_table_lookup_by_address(workers_table_t *workers_table, const char *address)
{
    return (worker_t *) g_hash_table_find(workers_table, s_compare_worker_to_name_, address);
}

worker_t *
workers_table_add_new_worker(workers_table_t *workers_table, gint key,
                             zframe_t *zaddr, const char *address,
                             const char *hostname, iodir_t iodir)
{
    worker_t *new_worker = worker_create(zaddr, address, hostname, iodir);
    if (new_worker) {
        new_worker->atime = g_get_monotonic_time();
        g_hash_table_insert (workers_table, GINT_TO_POINTER(key), new_worker);
    }
    return new_worker;
}

worker_t *
workers_table_lookup_by_key(workers_table_t *workers_table, gint key)
{
    return (worker_t *) g_hash_table_lookup (workers_table,
            GINT_TO_POINTER(key));
}

static gboolean s_is_other_(G_GNUC_UNUSED gpointer packed_key, gpointer value,
                            gpointer user_data)
{
    // gint key = GPOINTER_TO_INT(packed_key);
    worker_t *W = (worker_t *) value;
    worker_t *worker = (worker_t *) user_data;
    if (W != worker && W->lcn == worker->lcn)
        return TRUE;
    return FALSE;
}

worker_t *
workers_table_lookup_other(workers_table_t *workers_table, worker_t *worker)
{
    return g_hash_table_find(workers_table, s_is_other_, worker);
}

void
workers_table_remove_by_key(workers_table_t *workers_table, gint key)
{
    g_hash_table_remove(workers_table, &key);
}

void
workers_table_remove_unused(workers_table_t *workers_table)
{
    g_hash_table_foreach_remove(workers_table, s_cull_dead_worker_, NULL);
}

static void
s_add_directory_entry_to_zhash_ (gpointer packed_key G_GNUC_UNUSED,
                                 gpointer vworker,
                                 gpointer user_data)
{
    worker_t *worker = (worker_t *)vworker;
    zhash_t *hash = user_data;
    gchar *str = g_strdup_printf("%s,%s",iodir_name(worker->iodir), worker->role == _READY ? "AVAILABLE" : "BUSY");
    zhash_insert(hash, worker_get_address(worker), str);
    g_free(str);
}

zhash_t *
workers_table_create_directory_zhash(workers_table_t *workers_table)
{
    zhash_t  *dir       = zhash_new();

    // Fill a hash table with the current directory information
    g_hash_table_foreach (workers_table, s_add_directory_entry_to_zhash_, dir);
    return dir;
}

static void
s_dump_func_(gpointer packed_key G_GNUC_UNUSED,
             gpointer value,
             gpointer user_data G_GNUC_UNUSED)
{
    // gint K = GPOINTER_TO_INT(packed_key);
    worker_t *W = value;
    g_print("address %s, hostname %s, lcn %d, role %d\n",
            W->address, W->hostname, W->lcn, W->role);
}

void
workers_table_dump(workers_table_t *workers_table)
{
    g_print("___WORKERS___\n");
    g_hash_table_foreach(workers_table, s_dump_func_, NULL);
    g_print("==============\n");
}
