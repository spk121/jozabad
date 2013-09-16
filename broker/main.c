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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <czmq.h>
#include "log.h"
#include "pgetopt.h"
#include "poll.h"
#include "tput.h"
// #include "lib.h"
//#include "../libjoza/joza_lib.h"

int main_port = 5555;

/* Note the ':' shown below in the pattern string, to specify additional arg */
static char options[] = "p:t:v:V?";
static char PROGNAME[] = "Jozabad Broker";
static char VERSION[] = "0.0";
static char PROTOCOL[] = "tcp://*:";

static void
usage (void)
{
    printf("%s : %s\n", PROGNAME, VERSION);
    printf("\n");
    printf("A server for the Jozabad chat protocol.\n");
    printf("\n");
    printf("\t-p(n)\tport number from 1 to 65535\n");
    printf("\t-v(n)\tverbosity level from zero (quiet) to five (verbose)\n");
    printf("\t-t(n)\tper-channel tput level 3 (slow) to %d (fast)\n", t_last);
    printf("\t-V\tshow version\n");
    printf("\t-?\tshow help\n");
    exit (0);
}

int main (int argc, char *argv[])
{
    NOTE("entering main()");
    NOTE("argc = %d", argc);
    for (int i = 0; i < argc; i ++) {
        NOTE("argv[%d] = '%s'", i, argv[i]);
    }

    int c;
    int val;
    while ((c = pgetopt(argc, argv, options)) != -1) {
        switch(c) {
        case 'p':
            val = atoi(poptarg);
            if (val >= 1 && val <= 65535) {
                main_port = val;
            } else {
                ERR("invalid port number %d", val);
                return (1);
            }
            break;
        case 't':
            val = atoi(poptarg);
            if (tput_rngchk((tput_t) val) == 0) {
                g_tput_threshold = (tput_t) val;
            } else {
                ERR("invalid per-channel tput level %d", val);
                return (1);
            }
            break;
        case 'v':
            g_log_level = atoi(poptarg);   /* should use strtol() in new code */
            break;
        case 'V':
            printf("%s : %s\n", PROGNAME, VERSION);
            break;
        case '?':
        default:
            usage();
            break;
        }
    }

    INFO("port = %d", main_port);
    INFO("verbosity = %d", g_log_level);
    INFO("per-channel tput = %s", tput_name(g_tput_threshold));

    size_t len = intlen(main_port) + strlen(PROTOCOL) + 1;
    char *s = calloc(len, sizeof(char));
    snprintf(s, "%s%d", PROTOCOL, main_port);

    poll_init(TRUE, s);
    poll_start();
    // finalize_poll();

    free(s);
    return 0;
}
