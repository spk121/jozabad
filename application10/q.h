/*
    q.h - q numbers

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

/**
 * @file q.h
 * @brief Q Numbers
 *
 * Q numbers are used to distinguish between different types
 * of data packets.
 */

#ifndef JOZA_Q_H
#define JOZA_Q_H

/**
 * @brief The list of q numbers
 *
 * Q numbers distinguish between different types of messages
 */
typedef enum {
  q_alpha,
  q_bravo,
  q_charlie,
  q_delta,
  q_echo,
  q_foxtrot,
  q_golf,
  q_hotel,
  q_india,
  q_juliet,
  q_kilo,
  q_lima,
  q_mike,
  q_november,
  q_oscar,
  q_papa,
  q_quebec,
  q_romeo,
  q_sierra,
  q_tango,
  q_uniform,
  q_victor,
  q_whiskey,
  q_xray,
  q_yankee,
  q_zulu,
} q_t;

#define Q_MAX (q_zulu)

/**
 * @brief Returns a string reprentation of a cause.
 *
 * @param C an cause
 * @return A null-terminated string. Do not free.
 */
const char *q_name(q_t c);

gboolean q_validate(q_t c);

#endif
