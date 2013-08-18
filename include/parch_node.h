/*  =========================================================================
    parch_node.h - node API

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

#ifndef __PARCH_NODE_H_INCLUDED__
#define __PARCH_NODE_H_INCLUDED__

#include <czmq.h>
#if CZMQ_VERSION < 10402
#   error "petulant-archer needs CZMQ/1.4.2 or later"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _parch_node_t parch_node_t;

//  @interface
CZMQ_EXPORT parch_node_t *
parch_node_new (char *broker, char *service, int verbose);
CZMQ_EXPORT void
    parch_node_destroy (parch_node_t **self_p);
CZMQ_EXPORT int
    parch_node_setsockopt (parch_node_t *self, int option, const void *optval,
    size_t optvallen);
CZMQ_EXPORT int
    parch_node_getsockopt (parch_node_t *self, int option, void *optval,
    size_t *optvallen);
CZMQ_EXPORT const char *
    parch_node_service (parch_node_t *self);
CZMQ_EXPORT void *
    parch_node_client (parch_node_t *self);

CZMQ_EXPORT void
    parch_node_send (parch_node_t *self, zmsg_t **request_p);
CZMQ_EXPORT int
    parch_node_poll (parch_node_t *self, int timeout_ms);
CZMQ_EXPORT zmsg_t *
    parch_node_recv (parch_node_t *self, char **service_p);
//  @end

#ifdef __cplusplus
}
#endif

#endif
