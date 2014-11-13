/*
    action.h - state machine action table

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
 * @file action.h
 * @brief List of actions that a broker may take
 *
 * When the broker receives a message from a worker, it
 * performs an action.  This is the list of actions.
 */
#ifndef JOZA_ACTION_H
#define JOZA_ACTION_H

#include <glib.h>
#include "state.h"

#define ACTION_NAME_MAX_LEN (21)
#define ACTION_STATE_COUNT (9)
#define ACTION_MESSAGE_COUNT (16)

/**
 * @brief The list of actions that the broker may take.
 *
 * When the broker receives a message, it checks the state of the
 * relevant channel or worker and performs an action.  This is the
 * list of possible actions.
 */
typedef enum {
    /* These are actions takes by the broker itself. */
    a_unspecified = 0,
    a_discard,             /**< broker discards this message */
    a_reset,         /**< broker resets flow control on this channel */
    a_clear,         /**< broker closes this channel gracefully */
    a_disconnect,  /**< UNUSED: broker closes both peers forcefully */

    // these are actions requested by the node
    a_x_connect,                /**< X's connection request */
    a_x_disconnect,             /**< X's disconnection request */
    a_x_call_request,           /**< X's new channel request  */
    a_x_call_accepted, /**< UNUSED: handle X's agreement to open a channel */
    a_x_clear_request, /**< X's request to close the channel gracefully */
    a_x_clear_confirmation, /**< X's agreement to Y's channel close request */
    a_x_data,               /**< X sends data to Y  */
    a_x_rr,                 /**< X is ready for more data from Y */
    a_x_rnr,           /**< X is not ready for more data from Y */
    a_x_reset,         /**< X request reset of flow control with Y  */
    a_x_reset_confirmation, /**< X confirms Y's request to reset flow control  */

    /* These are actions request by the peer */
    a_y_disconnect, /**< Y's connection request */
    a_y_call_request, /**< Y's disconnection request */
    a_y_call_accepted,        /**< UNUSED: Y's new channel request  */
    a_y_call_collision,       /**< Y's agreement to open a channel */
    a_y_clear_request, /**< Y's request to close the channel gracefully */
    a_y_clear_confirmation, /**< Y's agreement to X's channel close request */
    a_y_data,   /**< Y sends data to X */
    a_y_rr,     /**< Y is ready for more data from X */
    a_y_rnr,    /**< Y is not ready for more data from X */
    a_y_reset,  /**< Y requests reset of flow control with X  */
    a_y_reset_confirmation, /**< Y confirms X's request to reset flow control */
    a_last = a_y_reset_confirmation
} action_t;

/**
 * @brief Returns a string reprentation of an action.
 *
 * @param A an action
 * @return A null-terminated string. Do not free.
 */
char const* action_name(action_t a) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Gets the appropriate action for a given state and msg type
 *
 * @param state  The state of a channel's state machine
 * @param msg_id A joza_msg's ID.  For example JOZA_MSG_DATA.
 * @param is_y  TRUE if the message comes from peer Y, FALSE for peer X.
 * @return An action.
 */
G_GNUC_INTERNAL
action_t    action_get(state_t state, int msg_id, gboolean is_y) G_GNUC_WARN_UNUSED_RESULT;

#endif

