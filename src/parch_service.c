/*  =========================================================================
    parch_service.c - a group of nodes with the same address

    -------------------------------------------------------------------------
    Copyright (c) 2013 - Michael L. Gran - http://lonelycactus.com
    Copyright other contributors as noted in the AUTHORS file.

    This file is part of petulant-archer, A ZeroMQ-based networking
    library implementing the Switched Virtual Circuit pattern.

    http://github.com/spk121/petulant-archer

    This is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANT-
    ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
    Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not see http://www.gnu.org/licenses.
    =========================================================================
*/

//  The service class keeps a list of nodes that have the same service name


struct _parch_service_t {
    broker_t *broker;           //  Broker instance
    char *name;                 //  Service name
    zlist_t *requests;          //  List of client requests
    zlist_t *waiting;           //  List of waiting workers
    size_t workers;             //  How many workers we have
    zlist_t *blacklist;
};

//  Here is the implementation of the methods that work on a service.
//  Lazy constructor that locates a service by name, or creates a new
//  service if there is no service already with that name.

parch_service_t *
parch_service_new(broker_t *broker, char *service)
{
    parch_service_t self = (parch_service_t *) zmalloc (sizeof (parch_service_t));
    assert (service_frame);
    char *name = strdup (service);

    self->broker = broker;
    self->name = name;
    self->requests = zlist_new ();
    self->waiting = zlist_new ();
    self->blacklist = zlist_new ();

    return self;
}

void
parch_service_destroy (parch_service_t **self_p)
{
    assert (self_p);
    if (*self_p) {
    while (zlist_size (self->requests)) {
        zmsg_t *msg = (zmsg_t*)zlist_pop (self->requests);
        zmsg_destroy (&msg);
    }
    //  Free memory keeping  blacklisted commands.
    char *command = (char *) zlist_first (self->blacklist);
    while (command) {
        zlist_remove (self->blacklist, command);
        free (command);
    }
    zlist_destroy (&self->requests);
    zlist_destroy (&self->waiting);
    zlist_destroy (&self->blacklist);
    free (self->name);
    free (self);
    *self_p = NULL;
}

//  The dispatch method sends request to the worker.
void
parch_service_dispatch (parch_service_t *self)
{
    assert (self);

    s_broker_purge (self->broker);
    if (zlist_size (self->waiting) == 0)
        return;

    while (zlist_size (self->requests) > 0) {
        worker_t *worker = (worker_t*)zlist_pop (self->waiting);
        zlist_remove (self->waiting, worker);
        zmsg_t *msg = (zmsg_t*)zlist_pop (self->requests);
        s_worker_send (worker, MDPW_REQUEST, NULL, msg);
        //  Workers are scheduled in the round-robin fashion
        zlist_append (self->waiting, worker);
        zmsg_destroy (&msg);
    }
}

void
parch_service_enable_command (parch_service_t *self, const char *command)
{
    char *item = (char *) zlist_first (self->blacklist);
    while (item && !streq (item, command))
        item = (char *) zlist_next (self->blacklist);
    if (item) {
        zlist_remove (self->blacklist, item);
        free (item);
    }
}

void
parch_service_disable_command (parch_service_t *self, const char *command)
{
    char *item = (char *) zlist_first (self->blacklist);
    while (item && !streq (item, command))
        item = (char *) zlist_next (self->blacklist);
    if (!item)
        zlist_push (self->blacklist, strdup (command));
}

int
parch_service_is_command_enabled (parch_service_t *self, const char *command)
{
    char *item = (char *) zlist_first (self->blacklist);
    while (item && !streq (item, command))
        item = (char *) zlist_next (self->blacklist);
    return item? 0: 1;
}
