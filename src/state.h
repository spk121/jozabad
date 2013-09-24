/*
    state.h - state machine

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

#ifndef PARCH_STATE_H_INCLUDE
#define PARCH_STATE_H_INCLUDE

#define STATE_NAME_MAX_LEN (24)

typedef enum _state_t {
    state_ready,
    state_x_call_request,
    state_y_call_request,
    state_data_transfer,
    state_call_collision,
    state_x_clear_request,
    state_y_clear_request,
    state_x_reset_request,
    state_y_reset_request,

    state_last = state_y_reset_request
} state_t;

#define STATE_CLEAR_REQUEST(is_y) ((is_y)?state_y_clear_request:state_x_clear_request)

char const *state_name(state_t a);

#endif

