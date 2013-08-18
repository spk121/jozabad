/*  =========================================================================
    parch_node.c - petulant-archer common definitions

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

#ifndef __PARCH_COMMON_H_INCLUDED__
#define __PARCH_COMMON_H_INCLUDED__

#include <stdlib.h>

//  These are the address fields from Section 2.4.2/X.25
#define PARCH_BROKER_TO_NODE         "\300" /* 192 */
#define PARCH_NODE_TO_BROKER         "\200" /* 128 */

//  These are the common command IDs as strings.  They come from Table 5-2/X.25
#define PARCH_DATA                   "\000" /* 0 */
#define PARCH_RR                     "\001" /* 1 */
#define PARCH_RNR                    "\005" /* 5 */
#define PARCH_REJ                    "\011" /* 9 */
#define PARCH_CALL_REQUEST           "\013" /* 11 */
#define PARCH_CALL_ACCEPTED          "\017" /* 15 */
#define PARCH_CLEAR_REQUEST          "\023" /* 19 */
#define PARCH_CLEAR_CONFIRMATION     "\027" /* 23 */
#define PARCH_RESET_REQUEST          "\033" /* 27 */
#define PARCH_RESET_CONFIRMATION     "\037" /* 31 */
#define PARCH_INTERRUPT              "\043" /* 35 */
#define PARCH_INTERRUPT_CONFIRMATION "\047" /* 39 */
#define PARCH_DIAGNOSTIC             "\361" /* 241 */
#define PARCH_RESTART_REQUEST        "\373" /* 251 */
#define PARCH_RESTART_CONFIRMATION   "\377" /* 255 */

static char const * const parch_commands [] = {
    /*   0 */ "DATA", "RR", NULL, NULL, NULL, "RNR", NULL, NULL,
    /*   8 */ NULL, "REJ", NULL, "CALL_REQUEST", NULL, NULL, NULL, "CALL_ACCEPTED",
    /*  16 */ NULL, NULL, NULL, "CLEAR_REQUEST", NULL, NULL, NULL, "CLEAR_CONFIRMATION",
    /*  24 */ NULL, NULL, NULL, "RESET_REQUEST", NULL, NULL, NULL,"RESET_CONFIRMATION",
    /*  32 */ NULL, NULL, NULL, "INTERRUPT", NULL, NULL, NULL, "INTERRUPT_CONFIRMATION",
    /*  40 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /*  48 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /*  56 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /*  64 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /*  72 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /*  80 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /*  88 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /*  96 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 104 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 112 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 120 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 128 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 136 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 144 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 152 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 168 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 176 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 184 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 192 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 200 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 208 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 216 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 224 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 232 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 240 */ NULL, "DIAGNOSTIC", NULL, NULL, NULL, NULL, NULL, NULL,
    /* 248 */ NULL, NULL, NULL, "RESTART_REQUEST", NULL, NULL, NULL, "RESTART_CONFIRMATION",
    /* 256 */ 
};

//  This is the version of the messaging that is implemented
#define PARCH_HEADER        "PARCH01"

#endif
