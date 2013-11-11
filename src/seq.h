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

#ifndef JOZA_SEQ_H
#define JOZA_SEQ_H

#include <stdint.h>
#include <stdbool.h>
#include "mylimits.h"


// SEQUENCE NUMBERS
// These 16-bit integers are part of DATA messages and are used to check
// if the messages are in order.  They are 16-bit because we assume that there won't
// be more than SEQ_MAX packets "on the wire" between the client and server.

int seq_rngchk(seq_t x);
gboolean seq_in_range(seq_t x, seq_t lo, seq_t hi);
int window_negotiate(seq_t request, seq_t current);
int window_rngchk(seq_t x);

#endif
