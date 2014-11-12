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

#ifndef JOZA_IODIR_H
#define JOZA_IODIR_H

#include <glib.h>

typedef enum {
    io_bidirectional,
    io_incoming_calls_barred,
    io_outgoing_calls_barred,
    io_calls_barred
} iodir_t;

int iodir_validate(iodir_t x);
const char *iodir_name(iodir_t x);
gboolean iodir_incoming_calls_allowed(iodir_t I);
gboolean iodir_outgoing_calls_allowed(iodir_t I);
#endif
