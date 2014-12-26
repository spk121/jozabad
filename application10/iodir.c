/*
    iodir.c - channel directionality

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

#include <assert.h>
#include "iodir.h"

const char iodir_names[4][22] = {
    "bidirectional",
    "incoming calls barred",
    "outgoing calls barred",
    "calls barred"
};

const char iodir_short_names[4][3] = {
    "IO",
    "XO",
    "IX",
    "XX"
};

gboolean iodir_incoming_calls_allowed(iodir_t I)
{
    return (I == io_bidirectional || I == io_outgoing_calls_barred);
}

gboolean iodir_outgoing_calls_allowed(iodir_t I)
{
    return (I == io_bidirectional || I == io_incoming_calls_barred);
}

gboolean iodir_validate(iodir_t x)
{
    if (x == io_bidirectional
            || x == io_incoming_calls_barred
            || x == io_outgoing_calls_barred
            || x == io_calls_barred)
        return TRUE;
    return FALSE;
}

const char *iodir_name(iodir_t x)
{
    assert(iodir_validate(x));
    return iodir_names[x];
}

const char *iodir_short_name(iodir_t x)
{
    assert(iodir_validate(x));
    return iodir_short_names[x];
}
