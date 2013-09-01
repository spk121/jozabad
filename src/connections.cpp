#include <string>
#include <memory>
#include "../include/connections.h"
#include "../include/msg.h"
#include "../include/log.h"
#include "../include/poll.h"
#include "../include/channel.h"
#include "../include/throughput.h"

using namespace std;

connection_store_t connection_store;
extern throughput_t opt_throughput;

// The DNAMES are used in log messages, because hex addresses are hard
// to keep straight.
static int iter = 0;
static char dnames[DNAME_COUNT][DNAME_LENGTH_MAX] = {

    "Anthony", "Brock", "Casey", "Dorotha", "Estelle", "Flora",
    "Garry", "Hector", "Iago", "Jasmin", "Kenji", "Laura", "Margot",
    "Nanette", "Orpheus", "Paul", "Quentin", "Rasheeda", "Sage",
    "Tuan", "Ursula", "Vonnie", "William", "Xander", "Yukiko",
    "Zachariah",

    "Arlene", "Bethann", "Camellia", "Danuta", "Emmaline", "Floretta",
    "Gayle", "Hollie", "Isaac", "Jeane", "Kathrine", "Laurie",
    "Marianela", "Nichole", "Oleta", "Priscilla", "Quartus", "Rana",
    "Shara", "Toni", "Ulysses", "Vallie", "Walt", "Xerxes",
    "Yehoyada", "Zephaniah",

    "Ava", "Beula", "Carol", "Daren", "Enriqueta", "Francoise",
    "Georgann", "Haggai", "Ira", "Jeanetta", "Keenan", "Leia",
    "Marie", "Noah", "Obadiah", "Peter", "Quirinius", "Rebecca",
    "Sharyn", "Trish", "Uzziah", "Velvet", "Wayland", "Xerces",
    "Yessica", "Zero",

    "Adam", "Bessie", "Carlos", "Donte", "Errol", "Frederic",
    "Giuseppe", "Hai", "Irene", "Jeannette", "Keesha", "Lelah",
    "Matt", "Nestor", "Oleta", "Pamela", "Queenie", "Reynalda",
    "Sherill", "Thomas", "Uriel", "Victor", "Weston", "Xibiah",
    "Yoav", "Zara"

};

Connection::Connection (zframe_t *a, const char *n, throughput_t t, direction_t d) {
    assert (validate (n));
    address = zframe_dup (a);
    id = iter ++;
    strcpy(name, n);
    throughput = t;
    direction = d;
}

Connection::~Connection () {
    zframe_destroy(&address);
}

char const *
dname(uint16_t id) {
    return dnames[id % DNAME_COUNT];
}

void
add_connection(connection_store_t *store, const char *key, msg_t *msg) {
    TRACE("add_connection(store = %p, key = %p '%s', msg = %p)", store, key, key, msg);
    int c = store->count(key);
    if (c == 0) {
        zframe_t *address = msg_address(msg);
        char *serv = msg_service(msg);
        throughput_t throughput = (throughput_t) msg_throughput(msg);
        direction_t direction = (direction_t) msg_directionality(msg);

        // CONNECTION NEGOTIATION -- the broker may the throughput requested by the worker.
        if (throughput != throttle(throughput, opt_throughput))
            NOTE("connection negotiation throttles requested throughput - %s -> %s", name(throughput), name(opt_throughput));
        throughput = throttle(throughput, opt_throughput);

        store->insert (pair<string, Connection *>(key, new Connection(address, serv, throughput, direction)));
        INFO("adding %s (%s, %s) as connection %s", key, name(throughput), name(direction), dname(store->at(key)->id));
        NOTE("%d connections exist", store->size());

        msg_t *reply = msg_new(MSG_CONNECT_INDICATION);
        msg_set_throughput(reply, throughput);
        msg_set_directionality(reply, direction);
        msg_set_service(reply, serv);
        connection_msg_send(store, key, &reply);
    }
    else {
        ERR("can't add %s as new connection. Connection already exists as %d.", key, dname(store->at(key)->id));
        abort ();
    }
}

Connection*
find_worker(connection_store_t *store, const char *key) {
    TRACE("find_worker(store = %p, key = %p '%s')", store, key, key);
    if (store->count(key) == 0) {
        TRACE("connection store has no workers");
        return NULL;
    }
    Connection* c = store->at(key);
    if (c == NULL) {
        TRACE("worker %s not found in connection store", key);
    }
    else {
        TRACE("worker %s found in connection store as %s", key, dname(c->id));
    }
    return c;
}

Connection*
find_worker_by_name(connection_store_t* store, const char* name) {
    TRACE("find_worker_by_name(store = %p, name = %p '%s')", store, name, name);
    Connection* connection = NULL;
    for (connection_store_t::iterator it = store->begin();
         it != store->end();
         ++ it) {
        if (strcmp(it->second->name, name) == 0)
            connection = it->second;
    }
    return connection;
}

void
connection_msg_send(connection_store_t *store, const char *key, msg_t **msg) {
    Connection *w = find_worker(store, key);
    msg_set_address(*msg, w->address);
    INFO("sending '%s' to %s", msg_const_command(*msg), dname(w->id));
    msg_send(msg, sock);
}

void
connection_disconnect(connection_store_t *store, const char *key) {
    TRACE("connection_disconnect(store = %p, key = %p '%s'", store, key, key);
    Connection* c = find_worker(store, key);
    if (c != NULL) {
        INFO("removing connection %s", dname(c->id));
        store->erase(key);
        NOTE("%d connections remain", store->size());
    }
    else {
        WARN("failed to disconnection non-existent connection %s", key);
    }
}

bool
connection_dispatch (channel_store_t* s, connection_store_t *c, const char *key, msg_t *msg) {
    // This dispatcher is for messages from already-registered workers
    // that aren't connected to peers.  Thus the only valid messages
    // are DISCONNECT and CALL_REQUEST
    bool more = false;
    int id = msg_id(msg);
    if (id == MSG_DISCONNECT) {
        connection_disconnect(c, key);
    }
    else if (id == MSG_CALL_REQUEST) {
        // Look for a peer key
        Connection* x = find_worker(c, key);
        Connection* y = find_worker_by_name(c, msg_service(msg));
        if (y == 0) {
            // No available worker found.  Send a clear request.
        } else {
            INFO("adding channel %s/%s", dname(x->id), dname(y->id));
            char *y_key = zframe_strhex(y->address);
            add_channel(s, key, dname(x->id), y_key, dname(y->id));
            free (y_key);
            more = true;
        }
    }
    return more;
}

#if 0

void
connection_disconnect(connection_store_t *c, const char *key) {
    int i = connection_store_find_worker(c, key);
    if (i > 0) {
        INFO("removing connection %s", c->connections[i].dname);
        free(c->connections[i].key);
        if (c->count - i > 1)
            memmove(&c->connections[i], &c->connections[i + 1], sizeof (connection_t) * (c->count - i));
        c->count--;
        NOTE("%d connections remain", c->count);
    }
    else {
        WARN("failed to disconnection non-existent connection %s", key);
    }
}

void
connection_msg_send(connection_store_t *c, const char *key, msg_t **msg) {
    // Note that this message may not have an address frame yet

    int i = connection_store_find_worker(c, key);
    if (i > 0) {
        msg_set_address(*msg, c->connections[i].address);
        INFO("sending '%s' to %s", msg_command(*msg), dname(key));
        msg_send(msg, sock);
    }
    else {
        WARN("failed to send %s to %s - worker not found", msg_command(*msg), key);
    }
}

void
connection_dispatch(connection_store_t *c, const char *key, msg_t *msg) {
    // This dispatcher is for messages from already-registered workers
    // that aren't connected to peers.  Thus the only valid messages
    // are DISCONNECT and CALL_REQUEST
    int msg_id = msg_id(msg);
    if (msg_id == MSG_DISCONNECT) {
        connection_disconnect(c, key);
    } else if (msg_id == MSG_CALL_REQUEST) {
        // Look for a peer key
        uint16_t y = connection_store_find_worker_by_name(c, msg_service(msg), key);
        if (y == 0) {
            // No available worker found.  Send a clear request.
        } else {
            channel_store_add_channel(&channel_store, key, c->connections[y].key);
            int channel_id = channel_store_find_worker (&channel_store, key);
            channel_store.channels[channel_id] = channel_dispatch (channel_store.channels[channel_id], msg);
        }
    }
}

#if 0

struct _connection_search_t {
    bool idle_found; // true if it found a free connection
    bool found; // true if it found a connection but it was busy
    int index; // index of a found connection, or -1 if not found.
};

struct _connection_search_t
find_free_connection_index_by_name(const char *name) {
    int i = 0;
    struct _connection_search_t ret = {false, false, -1};
    do {
        if (streq(name, connections[i].name)) {
            if (connections[i].busy == 0) {
                ret.idle_found = true;
                ret.found = true;
                ret.index = i;
                break;
            } else {
                ret.found = true;
                ret.index = i;
            }
        }
        i++;
    } while (i < connection_count);
    return ret;
}

struct _connection_search_t
find_free_connection_index_by_key(const char *key) {
    int i = 0;
    struct _connection_search_t ret = {false, false, -1};
    do {
        if (streq(key, connections[i].key)) {
            if (connections[i].busy == 0) {
                ret.idle_found = true;
                ret.found = true;
                ret.index = i;
                break;
            } else {
                ret.found = true;
                ret.index = i;
            }
        }
        i++;
    } while (i < connection_count);
    return ret;
}

void add_to_worker_directory(char *key, zframe_t *address, const char *service, byte throughput,
        byte incoming, byte outgoing) {
    // Check to make sure this worker isn't already in the directory.  If it is, send a diagnostic.
    // Check to make sure the directory isn't full.  If it is, send a diagnostic.
    // Otherwise, add it to the directory
    connections[connection_count].key = strdup(key);
    connections[connection_count].address = zframe_dup(address);
    strncpy(connections[connection_count].name, service, 40);
    connections[connection_count].throughput_index = throughput;
    connections[connection_count].incoming_calls_barred = incoming;
    connections[connection_count].outgoing_calls_barred = outgoing;
    connection_count++;
}
#endif

#endif
