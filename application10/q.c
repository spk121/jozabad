/*
    q.c - q numbers

    Copyright 2014 Michael L. Gran <spk121@yahoo.com>

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
#include "q.h"

#define Q_NAME_MAX_LEN (9)

static const char q_names[Q_MAX + 1][Q_NAME_MAX_LEN] = {
  "ALPHA",
  "BRAVO",
  "CHARLIE",
  "DELTA",
  "ECHO",
  "FOXTROT",
  "GOLF",
  "HOTEL",
  "INDIA",
  "JULIET",
  "KILO",
  "LIMA",
  "MIKE",
  "NOVEMBER",
  "OSCAR",
  "PAPA",
  "QUEBEC",
  "ROMEO",
  "SIERRA",
  "TANGO",
  "UNIFORM",
  "VICTOR",
  "XRAY",
  "YANKEE",
  "ZULU"
};

const char *Q_name(q_t q)
{
    g_assert_cmpint(q, <=, Q_MAX);

    return &(q_names[q][0]);
}

gboolean q_validate(q_t q)
{
  if (q < q_alpha || q > q_zulu)
    return FALSE;
  return TRUE;
}
