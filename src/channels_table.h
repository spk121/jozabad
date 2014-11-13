/*
    channels_table.h - a collection of channels

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
 * @file channels_table.h
 * @brief A hash table of channels
 *
 * A channels table is a hash table of channels whose key is the
 * logical channel number.  It holds all the open channels for a given
 * server.
 */

#ifndef JOZA_CHANNELS_TABLE_H
#define JOZA_CHANNELS_TABLE_H

#include <glib.h>
#include <czmq.h>
#include "channel.h"

/**
 * @brief The channels_table_t is just a GHashTable.
 *
 * The key of the has table is the Logical Channel Number
 * and the value is a channel_t *.
 */
typedef GHashTable channels_table_t;

/**
 * @brief Create a new empty channels table
 *
 * Allocates a new empty hash table whose key is an integer and whose
 * value is a pointer (to a channel_t).
 *
 * @return A new, empty channels table
 */
G_GNUC_INTERNAL
channels_table_t *channels_table_create(void) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Destroy a channels_table_t
 *
 * Deallocates the hash table and sets its pointer to NULL.
 *
 * @param p_channels_table
 */
G_GNUC_INTERNAL
void channels_table_destroy(channels_table_t **p_channels_table);

/**
 * @brief Returns the next unused logical channel number
 *
 * Beginning with the logical channel number @p last_lcn, it searches
 * for a LCN that isn't currently assigned to a channel.
 *
 * @param channels_table
 * @param lcn the first LCN to test.
 * @return an unused LCN
 * @warning If there are no free LCN's this routine will repeate infinitely.
 */
G_GNUC_INTERNAL
lcn_t channels_table_find_free_lcn(channels_table_t *channels_table, lcn_t lcn) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Returns TRUE if the channels table is full
 *
 * The CHANNEL_COUNT_MAX constant is the maximum number of channels
 * that the server will allow.
 *
 * @param channels_table
 * @return TRUE if there are CHANNEL_COUNT_MAX channels already in use.
 */
G_GNUC_INTERNAL
gboolean channels_table_is_full(channels_table_t *channels_table);

/**
 * @brief Allocates and stores a new channel in the channels_table
 *
 * @param channel_table  A channel table
 * @param lcn The unique Logical Channel Number for this channel
 * @param xzaddr  a CZMQ frame containing a ZMQ router identity for X caller
 * @param xname  a plain-text identifier for X caller
 * @param yzaddr  a CZMQ frame containing a ZMQ router identity for Y callee
 * @param yname  a plain-text identifier for Y callee
 * @param pkt   the negotiated maximum packet size category for this channel
 * @param window  the delta between the smallest and largest acceptable sequence number
 * @param tput  the maximum allowed throughput category for this channel
 * @return An new channel.  Must be freed by the caller.
 */
G_GNUC_INTERNAL
channel_t * channels_table_add_new_channel(channels_table_t *channel_table, lcn_t lcn, zframe_t *xzaddr,
        const char *xname, zframe_t *yzaddr, const char *yname,
        packet_t pkt, lcn_t window, tput_t tput) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Returns a channel with the given LCN
 *
 * It searches through the channels_table for a channel with a matching LCN.
 *
 * @param channels_table
 * @param lcn
 * @return The matching channel, or NULL if not found.
 */
G_GNUC_INTERNAL
channel_t *channels_table_lookup_by_lcn(channels_table_t *channels_table, lcn_t lcn) G_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Removes the channel with the given LCN from the channels_table
 *
 * It searches through the channels_table for a channel with a matching LCN.
 *
 * @param channels_table
 * @param lcn The logical channel number
 */
G_GNUC_INTERNAL
void channels_table_remove_by_lcn(channels_table_t *channels_table, lcn_t lcn);

/**
 * @brief Prints out the contents of the channels table
 *
 * It prints it to the current output port.
 *
 * @param channels_table
 */
G_GNUC_INTERNAL
void channels_table_dump(const channels_table_t *channels_table);


#endif
