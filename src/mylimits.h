/*
    mylimits.h -- sizes and number types

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
 * @file mylimits.h
 * @brief The compile-time hardcoded limits for the server
 *
 */

#ifndef JOZA_MYLIMITS_H
#define JOZA_MYLIMITS_H

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include "tput.h"

// Just to keep things simple, I use a lot of hard limits and
// capacities in this code.  I figure that it is fair, since it is intended
// for very low speed lengths and very low memory.

////////////////////////////////////////////////////////////////
// YOU MAY CHANGE THESE NUMBERS

/**
 * @brief The maximum allowed number of connected workers.
 *
 * Must fit within a #worker_idx_t.
 */
#define _WORKER_COUNT 1000

/**
 * @brief The maximum allowed number of connected channels.
 *
 * Must fit within a #chan_idx_t.  Since two workers are required for one
 * connection, there is no need for _CHAN_COUNT to be greater than
 * _WORKER_COUNT / 2.
 */
#define _CHAN_COUNT 200

/**
 * @brief The maximum number of 8-bit bytes in a worker name.
 *
 * Does not include the terminating NULL. Must be 254 or less.
 */
#define NAME_LEN 16

/**
 * @brief Maximum allowed per-channel throughput
 *
 * The maximum data rate that each worker on a channel may
 * send before they get throttled or kicked off
 */
#define TPUT_MAX (t_last)

static_assert (TPUT_MAX >= t_default, "TPUT_MAX must be greater than t_default");

////////////////////////////////////////////////////////////////
// DO NOT CHANGE ANYTHING BELOW

// These limits are part of the message format, and thus cannot be changed

// TPUT
// An enumerated type that would fit in a uint8_t

// CAUSE
// An enumerated type that would fit in a uint8_t

// DIAGNOSTIC
// An enumerated type that would fit in a uint8_t

// NAME
// A vector of bytes that indicates an address

// This limitation comes from GSL.
static_assert(NAME_LEN <= UINT8_MAX-1, "NAME_LEN too large");

// Q NUMBERS
// These 8-bit integers are used in DATA messages as an enumerated
// type that would describe the contents of the DATA frame.  Their
// definition is application specific.

/**
 * @brief The type to hold a DATA packet's Q-number
 */
typedef uint8_t q_t;

/**
 * @brief A macro to cast an integer to the Q-number type
 */
#define Q_C(x) UINT8_C(x)

/**
 * @brief The smallest valid Q number
 */
#define Q_MIN Q_C(0)

/**
 * @brief The largest valid Q number
 */
#define Q_MAX UINT8_MAX

// SEQUENCE NUMBERS  These 16-bit integers  are part of  DATA messages
// and  are used  to check  if the  messages are  in order.   They are
// 16-bit  because we  assume  that there  won't be  more  than a  few
// hundred packets  in transit  "on the wire"  between one  worker and
// another

/**
 * @brief The integer type to hold sequence numbers, such as PS and PR
 */
typedef uint16_t seq_t;

/**
 * @brief A macro to cast to the #seq_t type
 */
#define SEQ_C(x) UINT16_C(x)

/**
 * @brief The smallest valid sequence number
 */
#define SEQ_MIN (SEQ_C(0))

/**
 * @brief The largest valid sequence number
 *
 * Since we do math on sequence numbers they need to be
 * less than half of UINT16_MAX so the math never overflows.
 */
#define SEQ_MAX (SEQ_C(8))

// WINDOWS - The delta between two non-identical SEQUENCE numbers

/**
 * @brief The smallest valid window size
 */
#define WINDOW_MIN (SEQ_C(1))

/**
 * @brief The largest valid window size
 */
#define WINDOW_MAX (SEQ_C(SEQ_MAX-1))

/**
 * @brief The default window size
 */
#define WINDOW_NEGOTIATE (SEQ_C(2))

// We do math on sequence numbers, so we have to check it won't
// overflow.
static_assert(UINT16_MAX - SEQ_MAX >= SEQ_MAX, "SEQ_MAX is too large for the data type");
static_assert(UINT16_MAX - WINDOW_MAX >= SEQ_MAX, "WINDOW_MAX is too large for the data type");
static_assert(WINDOW_NEGOTIATE >= WINDOW_MIN && WINDOW_NEGOTIATE <= WINDOW_MAX,
              "WINDOW_NEGOTIATE is out of range");

////////////////////////////////////////////////////////////////
// The WORKER array indices and size

// As noted above, ZeroMQ uses a 32-bit for each connection.  It
// limits the number of simultaneous worker connections to a 32-bit
// integer.

static_assert(_WORKER_COUNT >= 2, "_WORKER_COUNT too small");
static_assert(_WORKER_COUNT <= UINT16_MAX, "_WORKER_COUNT too large for data type");

/**
 * @brief The integer type to hold counts and indices of workers.
 */
typedef uint16_t worker_idx_t;

/**
 * @brief A macro to cast an integer to the worker_idx_t type
 */
#define WORKER_IDX_C(x) UINT16_C(x)

/**
 * @brief The smallest valid worker index
 */
#define WORKER_IDX_MIN (WORKER_IDX_C(0))

/**
 * @brief The largest valid worker index
 */
#define WORKER_IDX_MAX (WORKER_IDX_C(_WORKER_COUNT-1))

/**
 * @brief The maximum allowed number of connected workers
 */
#define WORKER_COUNT WORKER_IDX_C(_WORKER_COUNT)


////////////////////////////////////////////////////////////////
// ZMQ Address keys

// ZeroMQ 3.x uses a 32-bit unsigned integer to denote each connection, so
// our hash-table key is that 32-bit integer.
/**
 * @brief The type to hold a unique ID for each worker
 *
 * This is determined by the underlying ZeroMQ structure, and also
 * must not be larger than gint.
 */
typedef uint32_t wkey_t;

/**
 * A macro to cast an integer to a #wkey_t
 */
#define WKEY_C(x) UINT32_C(x)

/**
 * @brief The smallest valid #wkey_t
 */
#define WKEY_MIN (WKEY_C(0))

/**
 * @brief The largest valid #wkey_t
 */
#define WKEY_MAX (WKEY_C(UINT32_MAX))

////////////////////////////////////////////////////////////////
// The CHANNEL array indices and size

// Since it takes two workers for each channel, we can
// limit these capacities to half those of the WORKER array

static_assert(_CHAN_COUNT >= 1, "_CHAN_COUNT too small");
static_assert(_CHAN_COUNT <= UINT16_MAX, "_CHAN_COUNT too large for data type");
static_assert(_CHAN_COUNT <= _WORKER_COUNT / 2, "_CHAN_COUNT too large for _WORKER_COUNT");

/**
 * @brief The integer type to hold counts and indices of channels
 */
typedef uint16_t chan_idx_t;

/**
 * @brief A macro to cast an integer to the #chan_idx_t type
 */
#define CHAN_IDX_C(x) UINT16_C(x)

/**
 * @brief The smallest valid channel index
 */
#define CHAN_IDX_MIN (CHAN_IDX_C(0))

/**
 * @brief The largest valid channel index
 */
#define CHAN_IDX_MAX (CHAN_IDX_C(_CHAN_COUNT-1))

/**
 * @brief The maximum allowed number of simultaneous channels
 */
#define CHAN_COUNT (CHAN_IDX_C(_CHAN_COUNT))

////////////////////////////////////////////////////////////////
// Logical Channel Numbers

// A Logical Channel Number is an integer that identifies a channel.
// But unlike the chan_idx_t, which is an index into a location of
// the channel array, a LCN is a number assigned to a connection that
// is constant for the lifetime of the connection.  It needs to be
// at least twice the size of the CHANNEL array, so that finding
// an unused LCN is not difficult.

// Also, LCN #0 is reserved for possible future stuff.
/**
 * @brief The integer type to hold Logical Channel Numbers
 */
typedef uint16_t lcn_t;

/**
 * @brief A macro to cast an integer to the #lcn_t type
 */
#define LCN_C(C) UINT16_C(C)

/**
 * @brief The smallest valid logical channel number
 */
#define LCN_MIN (LCN_C(1))

/**
 * @brief The largest valid logical channel number
 */
#define LCN_MAX (LCN_C(UINT16_MAX))

/**
 * @brief The maximum number of unique logical channel numbers
 */
#define LCN_COUNT (LCN_C(UINT16_MAX - 1))

static_assert(LCN_MIN + LCN_COUNT == LCN_MAX,
              "LCN_MIN + LCN_COUNT doesn't equal LCN_MAX");
static_assert(CHAN_COUNT <= LCN_COUNT / 2,
              "LCN_COUNT is too low for this CHAN_COUNT");

#endif
