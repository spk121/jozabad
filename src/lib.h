/*
    lib.h - miscellaneous library functions

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
 * @file lib.h
 * @author Mike Gran
 * @brief Miscellaneous library functions
 */

#ifndef JOZA_LIB_H
#define JOZA_LIB_H

#include <glib.h>
#include <stdint.h>
#include <stdlib.h>
#include <czmq.h>

//----------------------------------------------------------------------------
// BASICS
//----------------------------------------------------------------------------

size_t intlen(int x);
char *cstrdup (const char *s);


/**
 * @brief A safer string length function
 *
 * @param s  A null-terminated string
 * @param maxsize  A the maximum length of the string
 * @return the lesser of the string length or @p maxsize
 */
size_t strnlen_s (const char *s, size_t maxsize);

//----------------------------------------------------------------------------
// ASCII-STYLE NAMES
//----------------------------------------------------------------------------
/**
 * @brief TRUE if a string contains the safe ASCII subset of characters
 *
 * The initial and terminal non-NULL character must be ASCII
 * alphanumeric or "%&+,.:=_" Middle character can also be SPACE.
 *
 * @param mem  A buffer of 8-bit characters
 * @param n  The number of bytes in the buffer
 * @return TRUE if the buffer contains the allowed ASCII characters
 */
gboolean safeascii(const char *mem, size_t n);

//----------------------------------------------------------------------------
// PHONE-STYLE NAMES
//----------------------------------------------------------------------------
gboolean safe121 (const char *str, size_t n);
uint32_t pack121(const char *str, size_t n);
const char *unpack121(uint32_t x);

//----------------------------------------------------------------------------
// INDEX SORTING OF STRINGS
//----------------------------------------------------------------------------

// void qisort(char *arr[], size_t n, size_t indx[]);
// size_t namefind(const char *arr[], size_t n, size_t nidx[], const char *str);

//----------------------------------------------------------------------------
// SEARCHING ORDERED UINT32_T ARRAYS
//----------------------------------------------------------------------------

// size_t ifind(uint32_t arr[], size_t n, uint32_t X);

//----------------------------------------------------------------------------
// STRICT C11 TIME
//----------------------------------------------------------------------------

double now(void);

//----------------------------------------------------------------------------
// GLIB TIME
//----------------------------------------------------------------------------

/**
 * @brief Returns a string representation of time.
 *
 * Given @p T, the number of microseconds since 1970, it returns a
 * string representation in HH:MM:SS.  It must be freed by the caller.
 *
 * @param T microseconds since 1970
 * @return A null-terminated string, to be freed by caller.
 */
G_GNUC_INTERNAL
char *monotonic_time_to_string(gint64 T) G_GNUC_WARN_UNUSED_RESULT;

// ZeroMQ Helpers

/**
 * @brief Return a new ZeroMQ context, or die in the attempt.
 *
 * Return a new ZeroMQ context, or die in the attempt.  If it can't
 * be done, log a fatal error and quit.
 * @return a ZeroMQ context, to be freed by caller.
 */
G_GNUC_INTERNAL
zctx_t *zctx_new_or_die (void) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Return a new ZeroMQ socket, or die in the attempt.
 *
 * Return a new ZeroMQ socket, or die in the attempt.  If it can't
 * be done, log a fatal error and quit.
 *
 * @param ctx  a ZeroMQ context
 * @param type a ZeroMQ socket type, e.g. ZMQ_ROUTER
 * @return a ZeroMQ socket, to be freed by caller
 */
G_GNUC_INTERNAL
void *zsocket_new_or_die(zctx_t *ctx, int type) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Return a new CZMQ loop, or die in the attempt.
 *
 * Return a new CZMQ loop, or die in the attempt.  If it can't
 * be done, log a fatal error and quit.
 *
 * @return a CZMQ socket, to be freed by caller
 */
G_GNUC_INTERNAL
zloop_t *zloop_new_or_die(void) G_GNUC_WARN_UNUSED_RESULT;

#endif  /* LIB_H */

