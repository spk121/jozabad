/*
    main.c - application entry point

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <czmq.h>
#include "lib.h"
#include "poll.h"
#include "raii.h"
#include "tput.h"
#include "channel.h"
#include "worker.h"
// #include "lib.h"
//#include "../libjoza/joza_lib.h"

gint main_port = 5555;
gboolean verbose = FALSE;

static GOptionEntry entries[] = {
    { "port", 'p', 0, G_OPTION_ARG_INT, &main_port, "Listening port N", "N" },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL},
    { NULL }
};

/* Note the ':' shown below in the pattern string, to specify additional arg */
// static char PROGNAME[] = "Jozabad Broker";
// static char VERSION[] = "0.0";
static char PROTOCOL[] = "tcp://*:";

int main (int argc, char *argv[])
{
    RAII_VARIABLE(GOptionContext *, context, NULL, raii_option_context_free);
    RAII_VARIABLE(gchar *, s, NULL, raii_gcharp_free);
    RAII_VARIABLE(joza_poll_t *, poll, NULL, raii_pollp_destroy);
    GError *error = NULL;

    context = g_option_context_new("- server options");
    g_option_context_set_summary(context, "This is an exchange server for the Jozabad chat protocol.");
    g_option_context_add_main_entries (context, entries, NULL);
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        exit (1);
    }

    if (main_port < 1 || main_port > 65535) {
        g_print("invalid port number %d", main_port);
        exit (1);
    }

    if (verbose) {
        g_print("port = %d", main_port);
        g_print("per-channel tput = %s", tput_name(TPUT_MAX));
    }

    s = g_strdup_printf("%s%d", PROTOCOL, main_port);

    poll = poll_create(verbose, s);
    poll_start(poll->loop);

    //channel_disconnect_all(poll->sock);
    //worker_remove_all();
    return 0;
}
