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

#ifndef JOZA_POLL_H
#define JOZA_POLL_H

#include <glib.h>
#include <czmq.h>

typedef struct {
    zctx_t *ctx;
    void *sock;
    zloop_t *loop;
} joza_poll_t;

joza_poll_t *poll_create(gboolean verbose, const char *endpoint);
void poll_start(zloop_t *loop);
void poll_destroy(joza_poll_t *poll);

#endif

