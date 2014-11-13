/*
    seq.h - message sequence numbers and flow control

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
 * @file seq.h
 * @brief helper functions for handling sequence number checks
 *
 * Sequence number are 16-bit integers that are part of DATA messages
 * and are used to check if the messages are in order.  As a worker
 * sends out DATA packets, it will give each packet a sequence number.
 * The sequence number increments from SEQ_MIN to SEQ_MAX, and then
 * wraps back around to SEQ_MIN.
 *
 * Windows are used in flow control.  A worker will declare that it is
 * ready to accept a set of DATA packets with sequence numbers between
 * LO and HI.  The difference between LO and HI, which is the window
 * size, is a constant for a given channel and it is agreed upon when
 * a channel is created.
 */
#ifndef JOZA_SEQ_H
#define JOZA_SEQ_H

#include <glib.h>
#include <stdint.h>
#include "mylimits.h"

/**
 * @brief Check that the sequence number is in the valid range
 *
 * Sequence numbers need to be between SEQ_MIN and SEQ_MAX.
 * @param x A sequence number
 * @return -1 if the sequence number is too small, 1 if too large, 0 otherwise
 */
int seq_rngchk(seq_t x);

/**
 * @brief Check that a sequence value is between lo and hi
 *
 * Since sequence numbers increment and then wrap from
 * SEQ_MAX back to SEQ_MIN, the data window also can wrap.
 * If @p lo is less than @p hi, then @p x needs to be between them.
 * If @p lo is greather than @p hi, then @x must not be between them.
 * @param x The sequence number to test
 * @param lo The beginning of the allowed range
 * @param hi The end of the allowed range
 * @return TRUE if in the allowed range.  FALSE otherwise.
 */
gboolean seq_in_range(seq_t x, seq_t lo, seq_t hi);

/**
 * @brief Return TRUE if @p request and @p current are a valid negotiation
 *
 * Checks to see if, in the context of an CALL_REQUEST negotiation, it
 * is valid to set the sequence window size from @p currend) to @p request.
 * In a throughput negotiation, the request has to be between the
 * current value and the WINDOW_NEGOTIATE value.
 * @param request Requested window size
 * @param current Current window size
 * @return TRUE if @p request is a valid request.  FALSE otherwise.
 */
int window_negotiate(seq_t request, seq_t current);

/**
 * @brief Check that a window size is in the valid range
 *
 * Window sizes need to be between WINDOW_MIN and WINDOW_MAX.
 * @param x A window size
 * @return -1 if the window is too small, 1 if too large, 0 otherwise
 */
int window_rngchk(seq_t x);

#endif
