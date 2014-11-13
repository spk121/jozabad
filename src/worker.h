/*
    worker.h - connected peers

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
 * @file worker.h
 * @brief A  worker's connection to the server
 *
 */

#ifndef JOZA_WORKER_H
#define JOZA_WORKER_H

#include <glib.h>
#include <czmq.h>
#include <assert.h>
#include <stdint.h>
#include "iodir.h"
#include "joza_msg.h"
#include "mylimits.h"

/**
 * @brief List of worker's possible roles
 *
 * Indicates whether a worker is not connected or is connected to another worker
 */
typedef enum {
    _READY = 0,   /**< worker is not on a call */
    X_CALLER = 1, /**< worker is on a call it initiated */
    Y_CALLEE = 2  /**< worker is on a call it received */
} role_t;

/**
 * @brief A worker's connection to the server
 */
typedef struct {
    wkey_t wkey;     /**< A unique ID for this worker, scraped from the ZeroMQ address */
    gchar *address;  /**< A plain-text address or name for this worker */
    gchar *hostname; /**< UNUSED: Optional identification information for this worker. */
    zframe_t *zaddr; /**< A CZMQ frame containing a ZeroMQ address */
    iodir_t iodir;   /**< indicates whether this worker can make or received calls */
    lcn_t lcn;       /**< when on a call, this is the Logical Channel Number of the call.  Otherwise 0 */
    role_t role;     /**< indicates whether it is unconnected, a caller or a callee */
    gint64 ctime;    /**< time of creation */
    gint64 mtime;    /**< time of last modification */
    gint64 atime;    /**< time of last message received from or sent to worker */
} worker_t;

/**
 * @brief Allocate a new worker structure
 *
 * @param zaddr  A CZMQ frame containing a ZeroMQ Router identity
 * @param address  A plain-text address for the new worker
 * @param name  additional identification information for the worker
 * @param io  the directionality of the new worker
 * @return A newly allocated worker.  The caller must free.
 */
worker_t *worker_create(zframe_t *zaddr, char *address, char *name, iodir_t io);

/**
 * @brief Return the plain-text address of a worker
 *
 * @param W  A worker
 * @return the plain-text address of the worker.  Do not free.
 */
const char *worker_get_address(const worker_t *W);

/**
 * @brief Return the last access time of the worker
 *
 * The access time is in units of microseconds since 1970.
 * @param W A worker
 * @return Access time in microseconds
 */
gint64 worker_get_atime(const worker_t *W) G_GNUC_CONST;

/**
 * @brief Return the Logical Channel Number of the worker
 *
 * @param W  A worker
 * @return The LCN, or zero if the worker is not connected
 */
lcn_t worker_get_lcn(const worker_t *W);

/**
 * @brief Return the role of the worker
 *
 * Indicates if it is unconnected, a caller, or a callee
 *
 * @param W  A worker
 * @return The role
 */
role_t worker_get_role(const worker_t *W) G_GNUC_CONST;

/**
 * @brief Return a CZMQ frame containing a ZeroMQ Router identity
 *
 * @param W  A worker
 * @return The ZeroMQ address frame
 */
zframe_t *worker_get_zaddr(const worker_t *W);

/**
 * @brief Return TRUE if the worker allows incoming calls
 *
 * @param W  A worker
 * @return TRUE if incoming calls are allowed.  FALSE otherwise.
 */
gboolean worker_is_allowed_incoming_call(const worker_t *W);

/**
 * @brief Return TRUE if the worker allows outgoing calls
 *
 * @param W  A worker
 * @return TRUE if outgoing calls are allowed.  FALSE otherwise.
 */
gboolean worker_is_allowed_outgoing_call(const worker_t *W);

/**
 * @brief Return TRUE if the worker isn't currently on a call
 *
 * In other words, return TRUE if its role is READY, not X_CALLER
 * or Y_CALLEE.
 * @param W  A worker
 * @return TRUE if not currently connected.  FALSE otherwise.
 */
gboolean worker_is_available_for_call(const worker_t *W);

/**
 * @brief Return TRUE is currently on a call it initiated
 *
 * In other words, return TRUE if its role is X_CALLER,
 * and not READY or Y_CALLEE.
 * @param W  A worker
 * @return TRUE if on a call it initiated.  FALSE otherwise.
 */
gboolean worker_is_x_caller(const worker_t *W);

/**
 * @brief Set the role of the worker to READY.
 *
 * This indicates that the worker is not currently on a call.
 * @param W  A worker
 */
void worker_set_role_to_ready(worker_t *W);

/**
 * @brief Set the role of the worker to X_CALLER.
 *
 * This indicates that the worker is now on a call it initiated.
 * @param W  A worker
 * @param lcn  The logical channel number of the new call.
 */
void worker_set_role_to_x_caller(worker_t *W, lcn_t lcn);

/**
 * @brief Set the role of the worker to Y_CALLEE.
 *
 * This indicates that the worker is now on a call it received
 * from another worker.
 *
 * @param W  A worker
 * @param lcn  The logical channel number of the new call.
 */
void worker_set_role_to_y_callee(worker_t *W, lcn_t lcn);

/**
 * @brief Update the access time of the channel to now.
 *
 * @param W  A worker
 */
void worker_update_atime(worker_t *W);

/**
 * @brief Update the mtime of the channel to now.
 *
 * @param W  A worker
 */
void worker_update_mtime(worker_t *W);

#endif
