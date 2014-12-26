/*
    tput.h

    Copyright 2013, 2014 Michael L. Gran <spk121@yahoo.com>

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

#include <glib.h>

/**
 * @file tput.h
 * @brief List of throughput rates for channels
 *
 */

#ifndef JOZA_TPUT_H
#define JOZA_TPUT_H

#include <stdint.h>

#define TPUT_NAME_LEN (12)

/**
 * @brief The list of possible maximum channel throughput values
 *
 * When a channel is established, the peers agree on a maximum throughput
 * for the channel.
 */
typedef enum {
    t_unspecified,  /**< UNUSED */
    t_reserved_1,   /**< UNUSED */
    t_reserved_2,   /**< UNUSED */
    t_75bps,
    t_150bps,
    t_300bps,
    t_600bps,
    t_1200bps,
    t_2400bps,
    t_4800bps,
    t_9600bps,
    t_19200bps,
    t_48kpbs,
    t_64kbps,
    t_128kbps,
    t_192kbps,
    t_256kbps,
    t_320kbps,
    t_384kbps,
    t_448kbps,
    t_512kbps,
    t_578kbps,
    t_640kbps,
    t_704kbps,
    t_768kbps,
    t_832kbps,
    t_896kbps,
    t_960kbps,
    t_1024kbps,
    t_1088kbps,
    t_1152kbps,
    t_1216kbps,
    t_1280kbps,
    t_1344kbps,
    t_1408kbps,
    t_1472kbps,
    t_1536kbps,
    t_1600kbps,
    t_1664kbps,
    t_1728kbps,
    t_1792kbps,
    t_1856kbps,
    t_1920kbps,
    t_1984kbps,
    t_2048kbps,

    t_negotiate = t_9600bps,  /**< in negotiating throughput, one chooses a value between this and initial */
    t_default = t_64kbps,  /**< starting point for channel throughput negotiation */
    t_last = t_2048kbps
} tput_t;

/**
 * @brief convert a #tput_t enum to a bits-per-second integer
 *
 * @param p A throughput enum
 * @return bits-per-second throughput value
 */
uint32_t    tput_bps(tput_t p);

/**
 * @brief convert a #tput_t enum to a string representation of the throughput rate
 *
 * @param p A throughput enum
 * @return A string representation of the data rate.  Do not free.
 */
const char *tput_name(tput_t c);

/**
 * @brief Return TRUE if @p r and @p c are a valid negotiation
 *
 * Checks to see if, in the context of an CALL_REQUEST negotiation, it
 * is valid to set the throughput from @p c (CURRENT) to @p r (REQUEST).
 * In a throughput negotiation, the request has to be between the
 * current value and the 9600 bps (t_negotiate).
 * @param r Requested throughput
 * @param c Current throughput
 * @return TRUE if @p r is a valid request.  FALSE otherwise.
 */
gboolean   tput_negotiate(tput_t r, tput_t c);

/**
 * @brief Check the range of the throughput value
 *
 * The throughput range needs to be between t_75bps
 * and the maximum which is the smaller of t_2048kbps
 * and the TPUT_MAX compile-time constant.
 *
 * @param p Throughput
 * @return -1 if the tput value is too small, 0 if it is valid, or 1 if it is too large
 */
int         tput_rngchk(tput_t p);

/**
 * @brief Return the lesser of @p request and @p limit
 *
 * @param request A thoughput rate
 * @param limit A max throughput
 * @return The minimum of the two
 */
tput_t      tput_throttle(tput_t request, tput_t limit);

#endif  /* JOZA_TPUT_H */

