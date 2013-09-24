/*
    ukey.h - unique key integers

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
#ifndef JOZA_UKEY_H
#define JOZA_UKEY_H

// UKEYs are integer members of a hash key list.
// The list should have the following properties
// - every member is unique
// - the list is strictly monotonically increasing

#include <stdint.h>
#include "mylimits.h"

typedef struct {
    chan_idx_t index;
    lcn_t key;
} chan_idx_lcn_t;

lcn_t lcn_find(lcn_t arr[], chan_idx_t n, lcn_t key);
chan_idx_lcn_t lcn_next(lcn_t arr[], chan_idx_t n, lcn_t key);

void indexx(lcn_t arr[], uint32_t n, lcn_t indx[]);
#endif
