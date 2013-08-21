/*  =========================================================================
    parch.h - public header file

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


#ifndef __PARCH_H_INCLUDED__
#define __PARCH_H_INCLUDED__

#define _GNU_SOURCE

// PARCH version macros for compile-time API detection
#define PARCH_VERSION_MAJOR 0
#define PARCH_VERSION_MINOR 1
#define PARCH_VERSION_PATCH 0

#define PARCH_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define PARCH_VERSION \
    PARCH_MAKE_VERSION(MYPROJ_VERSION_MAJOR, \
                        MYPROJ_VERSION_MINOR, \
                        MYPROJ_VERSION_PATCH)



#include <czmq.h>
#if CZMQ_VERSION < 10402
#   error "petulant-archer needs CZMQ/1.4.2 or later"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>
#include <stdbool.h>

//  Opaque class structures
typedef struct _parch_state_engine_t parch_state_engine_t;

#include "lib.h"

//  Classes in the API
#include "parch_msg.h"
#include "parch_msg2.h"
// #include "parch_common.h"
#include "parch_node.h"
#include "parch_state_engine.h"
#include "parch_broker.h"

#ifdef __cplusplus
}
#endif

#endif
