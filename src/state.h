/*
    state.h - states for state machine

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

/**
 * @file state.h
 * @brief List of states for channel state machine
 *
 */

#ifndef JOZA_STATE_H
#define JOZA_STATE_H

#define STATE_NAME_MAX_LEN (24)

/**
 * @brief List of states in the channel state machine
 */
typedef enum {
    state_ready,          /**< Channel is not connected to workers */
    state_x_call_request, /**< Channel is waiting for Y to accept or reject a call request */
    state_y_call_request, /**< Channel is waiting for X to accept or reject a call request */
    state_data_transfer,  /**< Channel is connected and ready to send data back and forth */
    state_call_collision, /**< Channel is handling simultaneous call requests from X and Y */
    state_x_clear_request, /**< Channel is waiting for Y to accept request to close the connection */
    state_y_clear_request, /**< Channel is waiting for X to accept request to close the connection */
    state_x_reset_request, /**< Channel is waiting for Y to accept request to reset flow control */
    state_y_reset_request, /**< Channel is waiting for X to accept request to reset flow control */

    state_last = state_y_reset_request
} state_t;

#define STATE_CLEAR_REQUEST(is_y) ((is_y)?state_y_clear_request:state_x_clear_request)

/**
 * @brief Return a text representation of the state name
 *
 * @param a A state value
 * @return A null-terminated string.  Do not free.
 */
char const *state_name(state_t a);

#endif // JOZA_STATE_H

