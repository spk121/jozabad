/*
    iodir.h - channel directionality

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
 * @file iodir.h
 * @brief List of possibilities of whether incoming or outgoing calls are allowed
 *
 * Workers and channels can be prevented from making or receiving
 * calls.  The iodir type indicates this behavior.
 */

#ifndef JOZA_IODIR_H
#define JOZA_IODIR_H

#include <glib.h>

/**
 * @brief The directionality, e.g. whether incoming or outgoing calls are allowed
 *
 */
typedef enum {
    io_bidirectional = 0,          /**< worker can place or receive calls */
    io_incoming_calls_barred = 1,  /**< worker won't receive calls */
    io_outgoing_calls_barred = 2,  /**< worker won't place calls */
    io_calls_barred = 3            /**< worker won't place or receive calls */
} iodir_t;

/**
 * @brief Return TRUE if the iodir_t is valid
 *
 * @param x An iodir_t directionality
 * @return TRUE if the directionality is valid.  FALSE otherwise
 */
gboolean iodir_validate(iodir_t x);

/**
 * @brief Returns the string representation of the directionality.
 *
 * @param x  An iodir_t directionality
 * @return A string.  Caller must not free.
 */
const char *iodir_name(iodir_t x);

/**
 * @brief Return TRUE if the iodir_t indicates that a worker can receive calls
 *
 * @param x An iodir_t directionality
 * @return TRUE if incoming calls are allowed.  FALSE otherwise.
 */
gboolean iodir_incoming_calls_allowed(iodir_t I);

/**
 * @brief Return TRUE if the iodir_t indicates that a worker can place calls
 *
 * @param x An iodir_t directionality
 * @return TRUE if outgoing calls are allowed.  FALSE otherwise.
 */
gboolean iodir_outgoing_calls_allowed(iodir_t I);

#endif
