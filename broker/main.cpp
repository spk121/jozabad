
#include <cstdio>
#include <cstdlib>
#include <string>
#include <czmq.h>
#include "../libjoza/joza_lib.h"
extern "C" {
#include "pgetopt.h"
}
#include "log.h"
#include "poll.h"

using namespace std;

int          opt_loglevel = 0;
throughput_t opt_throughput = t_default;
int          opt_port = 5555;
int          opt_max_connections = 200;

/* Note the ':' shown below in the pattern string, to specify additional arg */
char opt_pattern[] = "p:t:v:V?";
const char PROGNAME[] = "Jozabad Broker";
const char VERSION[] = "0.0";

static void
usage (void) {
    printf("%s : %s\n", PROGNAME, VERSION);
    printf("\n");
    printf("A server for the Jozabad chat protocol.\n");
    printf("\n");
    printf("\t-p(n)\tport number from 1 to 65535\n");
    printf("\t-v(n)\tverbosity level from zero (quiet) to five (verbose)\n");
    printf("\t-t(n)\tper-channel throughput level 3 (slow) to %d (fast)\n", t_last);
    printf("\t-V\tshow version\n");
    printf("\t-?\tshow help\n");
    exit (0);
}

int main (int argc, char *argv[]) {
    NOTE("entering main()");
    NOTE("argc = %d", argc);
    for (int i = 0; i < argc; i ++) {
        NOTE("argv[%d] = '%s'", i, argv[i]);
    }

    int c;
    int val;
    while ((c = pgetopt(argc, argv, opt_pattern)) != -1) {
	switch(c) {
            case 'p':
                val = atoi(poptarg);
                if (val >= 1 && val <= 65535) {
                    opt_port = val;
                }
                else {
                    ERR("invalid port number %d", val);
                    return (1);
                }
                break;
            case 't':
                val = atoi(poptarg);
                if (throughput_validate((throughput_t) val)) {
                    opt_throughput = (throughput_t) val;
                }
                else {
                    ERR("invalid per-channel throughput level %d", val);
                    return (1);
                }
                break;
            case 'v':
        	opt_loglevel = atoi(poptarg);   /* should use strtol() in new code */
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

    INFO("port = %d", opt_port);
    INFO("verbosity = %d", opt_loglevel);
    INFO("per-channel throughput = %s", throughput_name(opt_throughput));
    string s = "tcp://*:" + to_string(opt_port);

    poll_init(true, s.c_str());;
    poll_start();
    // finalize_poll();
    return 0;
}
