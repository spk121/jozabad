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

#ifndef JOZA_MYLIMITS_H
#define JOZA_MYLIMITS_H

#include <assert.h>
#include <limits.h>
#include <stdint.h>

// Just to keep things simple, I use a lot of hard limits and
// capacities in this code.  I figure that it is fair, since it is intended
// for very low speed lengths and very low memory.

////////////////////////////////////////////////////////////////
// YOU MAY CHANGE THESE NUMBERS

#define _WORKER_COUNT 1000
#define _CHAN_COUNT 200

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
#define NAME_LEN 16
static_assert(NAME_LEN <= INT8_MAX, "NAME_LEN too large");

// Q NUMBERS
// These 8-bit integers are used in DATA messages as an enumerated
// type that would describe the contents of the DATA frame.  Their
// definition is application specific.
#define Q_C(x) UINT8_C(x)
#define Q_MIN Q_C(0)
#define Q_MAX UINT8_MAX
typedef uint8_t q_t;

// SEQUENCE NUMBERS  These 16-bit integers  are part of  DATA messages
// and  are used  to check  if the  messages are  in order.   They are
// 16-bit  because we  assume  that there  won't be  more  than a  few
// hundred packets  in transit  "on the wire"  between one  worker and
// another
#define SEQ_C(x) UINT16_C(x)
#define SEQ_MIN UINT16_C(0)
#define SEQ_MAX (UINT16_MAX/2)
typedef uint16_t seq_t;

// WINDOWS - The delta between two non-identical SEQUENCE numbers
#define WINDOW_MIN (SEQ_C(1))
#define WINDOW_MAX (SEQ_C(UINT16_MAX/2 - 1))
// The default WINDOW value
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

#define WORKER_IDX_C(x) UINT16_C(x)
#define WORKER_IDX_MIN (WORKER_IDX_C(0))
#define WORKER_IDX_MAX (WORKER_IDX_C(_WORKER_COUNT-1))
#define WORKER_COUNT WORKER_IDX_C(_WORKER_COUNT)
typedef uint16_t worker_idx_t;

////////////////////////////////////////////////////////////////
// ZMQ Address keys

// ZeroMQ 3.x uses a 32-bit unsigned integer to denote each connection, so
// our hash-table key is that 32-bit integer.

#define WKEY_C(x) UINT32_C(x)
#define WKEY_MIN WKEY_C(0)
#define WKEY_MAX UINT32_MAX
typedef uint32_t wkey_t;

////////////////////////////////////////////////////////////////
// The CHANNEL array indices and size

// Since it takes two workers for each channel, we can
// limit these capacities to half those of the WORKER array

static_assert(_CHAN_COUNT >= 1, "_CHAN_COUNT too small");
static_assert(_CHAN_COUNT <= UINT16_MAX, "_CHAN_COUNT too large for data type");
static_assert(_CHAN_COUNT <= _WORKER_COUNT / 2, "_CHAN_COUNT too large for _WORKER_COUNT");

#define CHAN_IDX_C(x) UINT16_C(x)
#define CHAN_IDX_MIN (CHAN_IDX_C(0))
#define CHAN_COUNT (CHAN_IDX_C(_WORKER_COUNT/2))
#define CHAN_IDX_MAX (CHAN_IDX_C(C_COUNT-1))
typedef uint16_t chan_idx_t;

////////////////////////////////////////////////////////////////
// Logical Channel Numbers

// A Logical Channel Number is an integer that identifies a channel.
// But unlike the chan_idx_t, which is an index into a location of
// the channel array, a LCN is a number assigned to a connection that
// is constant for the lifetime of the connection.  It needs to be
// at least twice the size of the CHANNEL array, so that finding
// an unused LCN is not difficult.

// Also, LCN #0 is reserved for possible future stuff.

#define LCN_C(C) UINT16_C(C)
#define LCN_MIN LCN_C(1)
#define LCN_MAX (UINT16_MAX-1)
#define LCN_COUNT UINT16_MAX
typedef uint16_t lcn_t;

static_assert(CHAN_COUNT <= LCN_COUNT / 2,
              "LCN_COUNT is too low for this CHAN_COUNT");

#endif
