/*
    channels_table.c - a collection of channels

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

#include <glib.h>
#include "channels_table.h"

#define CHANNEL_COUNT_MAX (100);

static void (*foreach_method)(channel_t *channel, gpointer user_data);

static void
s_foreach_func_(gpointer key G_GNUC_UNUSED, gpointer value, gpointer user_data)
{
    channel_t *channel = value;
    foreach_method(channel, user_data);
}

channels_table_t *
channels_table_create()
{
    return g_hash_table_new_full (g_int_hash, g_int_equal, NULL, g_free);
}

void
channels_table_destroy(channels_table_t **p_channels_table)
{
    g_hash_table_destroy(*p_channels_table);
    p_channels_table = NULL;
}

lcn_t
channels_table_find_free_lcn(channels_table_t *channels_table, lcn_t lcn)
{
    while (g_hash_table_lookup(channels_table, &lcn) != NULL) {
        lcn ++;
        if (lcn > LCN_MAX)
            lcn = LCN_MIN;
   }
   return lcn;
}

gboolean
channels_table_is_full(channels_table_t *channels_table)
{
    return g_hash_table_size(channels_table) >= CHANNEL_COUNT_MAX;
}

channel_t *
  channels_table_add_new_channel(channels_table_t *channels_table, gint lcn, zframe_t *xzaddr, const char *xaddress,
                                 zframe_t *yzaddr, const char *yaddress, packet_t pkt, lcn_t window, tput_t tput)
{
    // g_print("Before adding new channel\n");
    // channels_table_dump(channels_table);
    channel_t *new_channel = channel_create(lcn, xzaddr, xaddress, yzaddr, yaddress, pkt, window, tput);
    new_channel->state = state_x_call_request;
    g_hash_table_insert(channels_table, &lcn, new_channel);
    // g_print("After adding new channel\n");
    // channels_table_dump(channels_table);
    return new_channel;
}

static gboolean
s_lookup_by_lcn_(gpointer key, gpointer value, gpointer data)
{
    channel_t *C = value;
    gint L = *((gint *)data);
    return C->lcn == L;
}

channel_t *
  channels_table_lookup_by_lcn(channels_table_t *channels_table, gint lcn)
{
#if 0
    return g_hash_table_lookup(channels_table, &lcn);
#else
   return g_hash_table_find(channels_table, s_lookup_by_lcn_, &lcn);   
#endif
}

void
channels_table_remove_by_lcn(channels_table_t *channels_table, gint lcn)
{
#if 0
    g_hash_table_remove(channels_table, &lcn);
#else
   g_hash_table_foreach_remove(channels_table, s_lookup_by_lcn_, &lcn);   
#endif
}

void
channels_table_foreach(channels_table_t *channels_table, void func(channel_t *worker, gpointer user_data), gpointer user_data)
{
   foreach_method = func;
   g_hash_table_foreach(channels_table, s_foreach_func_, user_data);   
   foreach_method = NULL;
}

static void
s_dump_func_(gpointer key G_GNUC_UNUSED, gpointer value, gpointer user_data)
{
    gint K = *(gint *)key;
    channel_t *C = value;
    gchar *ctime_str = monotonic_time_to_string(C->ctime);
    gchar *mtime_str = monotonic_time_to_string(C->mtime);
    g_print("lcn %d, %s/%s, %s, created %s, modified %s\n",
            C->lcn, C->xname, C->yname, state_name(C->state), ctime_str, mtime_str);
    g_free(ctime_str);
    g_free(mtime_str);
}

void
channels_table_dump(channels_table_t *channels_table)
{
    g_print("====CHANNELS====\n");
    g_hash_table_foreach(channels_table, s_dump_func_, NULL);
    g_print("================\n");
}
