/*
    channels_table.h - a collection of channels

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

#pragma once

#include <glib.h>
#include <czmq.h>
#include "channels_table.h"
#include "channel.h"

typedef GHashTable channels_table_t;

channels_table_t *
  channels_table_create(void);
void
  channels_table_destroy(channels_table_t **p_channels_table);
lcn_t
  channels_table_find_free_lcn(channels_table_t *channels_table, lcn_t last_lcn);
gboolean
  channels_table_is_full(channels_table_t *channels_table);
channel_t *
  channels_table_add_new_channel(channels_table_t *channel_table, gint lcn, zframe_t *xzaddr, const char *xaddress,
                                 zframe_t *yzaddr, const char *yaddress, packet_t pkt, lcn_t window, tput_t tput);
channel_t *
  channels_table_lookup_by_lcn(channels_table_t *channels_table, gint lcn);
void
  channels_table_remove_by_lcn(channels_table_t *channels_table, gint lcn);
void
  channels_table_foreach(channels_table_t *channels_table, void func(channel_t *worker, gpointer user_data), gpointer user_data);
void
channels_table_dump(channels_table_t *channels_table);


