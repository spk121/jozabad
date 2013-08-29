#include <string>
#include <memory>
#include "../include/connections.h"
#include "../include/msg.h"
#include "../include/log.h"
#include "../include/poll.h"

using namespace std;

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

char const *
Connection::dname() {
    return dnames[id % DNAME_COUNT];
}

Connection::~Connection () {
    zframe_destroy(&address);
}

ConnectionStore::ConnectionStore () {
}

Connection*
ConnectionStore::find_worker(const char *key) {
    // assert (store.count(key) > 0);
    string s;

    return store[key];
}

void
ConnectionStore::connect(const char *key, msg_t *msg) {
    int c = store.count(key);
    if (c == 0) {
        zframe_t *address = msg_address(msg);
        char *name = msg_service(msg);
        throughput_t throughput = (throughput_t) msg_throughput(msg);
        direction_t direction = (direction_t) msg_directionality(msg);

        store.insert (pair<string, Connection *>(key, new Connection(address, name, throughput, direction)));
        NOTE("adding %s as connection %s", key, store[key]->dname());
        NOTE("%d connections exist", store.size());

        msg_t *reply = msg_new(MSG_CONNECT_INDICATION);
        msg_set_throughput(reply, throughput);
        msg_set_directionality(reply, direction);
        msg_set_service(reply, name);
        connection_msg_send(key, &reply);
    }
}

void
ConnectionStore::connection_msg_send(const char *key, msg_t **msg) {
    // Note that this message may not have an address frame yet

    Connection *w = find_worker(key);
    msg_set_address(*msg, w->address);
    INFO("sending '%s' to %s", msg_command(*msg), w->dname());
    msg_send(msg, sock);
}

#if 0
uint16_t
connection_store_find_worker(connection_store_t *c, const char *key) {
    bool found = false;
    int i = c->min;
    while (i < c->count) {
        if (strcmp(c->connections[i].key, key) == 0) {
            found = true;
            break;
        }
        i++;
    }
    if (found)
        return i;
    return 0;
}

uint16_t
connection_store_find_worker_by_name(connection_store_t *c, const char *name, const char *my_key) {
    bool found = false;
    int i = c->min;

    // searching a worker with the given name that isn't me and isn't busy.
    while (i < c->count) {
        if ((strcmp(c->connections[i].name, name) == 0)
                && (strcmp(c->connections[i].key, my_key) != 0)
                && c->connections[i].busy == false) {
            found = true;
            break;
        }
        i++;
    }
    if (found)
        return i;
    return 0;
}


const char *
dname(const char *key) {
    int i = connection_store_find_worker(&connection_store, key);
    if (i == 0)
        return key;
    return connection_store.connections[i].dname;
}

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
