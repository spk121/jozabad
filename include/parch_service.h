/*  =========================================================================
    parch_service.c - a group of nodes with the same address, API

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

#ifndef __PARCH_SERVICE_H_INCLUDED__
#define __PARCH_SERVICE_H_INCLUDED__

#include <czmq.h>
#if CZMQ_VERSION < 10402
#   error "petulant-archer needs CZMQ/1.4.2 or later"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef_struct _parch_service_t parch_service_t;

parch_service_t *
    parch_service_new (zframe_t *service_frame);
void
    parch_service_destroy (void *argument);
void
    parch_service_dispatch (parch_service_t *service);
void
    parch_service_enable_command (parch_service_t *self, const char *command);
void
    parch_service_disable_command (parch_service_t *self, const char *command);
int
    parch_service_is_command_enabled (parch_service_t *self, const char *command);

#ifdef __cplusplus
}
#endif

#endif
