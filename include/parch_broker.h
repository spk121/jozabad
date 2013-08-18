/*  =========================================================================
    parch_broker.h - broker API

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

#ifndef __PARCH_BROKER_H_INCLUDE__
#define __PARCH_BROKER_H_INCLUDE__

typedef struct _broker_t broker_t;
typedef struct _node_t node_t;

int
parch_broker_get_verbose (broker_t *self);

int
parch_node_call_request (node_t *self, char const * const service);

void *
parch_broker_get_socket (broker_t *self);

zframe_t *
parch_node_get_node_address (node_t *self);

node_t *
parch_node_get_peer (node_t * const self);

parch_state_engine_t *
parch_node_get_state_engine (node_t *self);

char *
parch_node_get_service_name (node_t *node);

void
parch_node_set_service_name (node_t *self, char *sname);

void
parch_node_update_service_name (node_t *node, char *service_name);

void
parch_node_disconnect_from_service (node_t *self);

void
parch_node_disconnect_from_peer (node_t *self);

#endif
