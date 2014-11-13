/*
    poll.h - event loop

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

/**
 * @file poll.h
 * @brief The event polling loop for CZMQ
 *
 */

#ifndef JOZA_POLL_H
#define JOZA_POLL_H

#include <glib.h>
#include <czmq.h>
#include "workers_table.h"
#include "channels_table.h"

/**
 * @brief An event polling loop and associated channel and worker state
 */
typedef struct {
    zctx_t *ctx;    /**< A ZeroMQ context */
    void *sock;     /**< A ZeroMQ socket */
    zloop_t *loop;  /**< A CZMQ polling loop */
    int timer;      /**< The integer ID of the CZMQ loop timer */
    zmq_pollitem_t pollitem;          /**< information about the current polled item */
    workers_table_t *workers_table;   /**< the connected workers */
    channels_table_t *channels_table; /**< the active channels */
} joza_poll_t;

/**
 * @brief Allocate and populate an event polling loop with associated channel and worker state
 *
 * @param verbose  Send DEBUG info to console when TRUE
 * @param endpoint  A URL pointing to the socket location
 * @return A newly allocated poll structure.  Caller must destroy.
 */
joza_poll_t *poll_create(gboolean verbose, const char *endpoint);

/**
 * @brief Active the polling loop, listening on its endpoint
 *
 * @param loop  A polling loop
 */
void poll_start(zloop_t *loop);

/**
 * @brief Deactivate a polling loop and destroy member
 *
 * @param poll A polling loop
 */
void poll_destroy(joza_poll_t *poll);

#endif

