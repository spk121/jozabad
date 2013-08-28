#include "../include/svc.h"

connection_store_t connection_store = {CONNECTIONS_MIN, CONNECTIONS_MAX, 0};
static int dindex = 0;
static char dnames[][16] = {
    "Anthony",
    "Brock",
    "Casey",
    "Dorotha",
    "Estelle",
    "Flora",
    "Garry",
    "Hector",
    "Iago",
    "Jasmin",
    "Katelin",
    "Laura",
    "Margot",
    "Nanette",
    "Orpheus",
    "Paul",
    "Quentin",
    "Rasheeda",
    "Sage",
    "Terry",
    "Ursula",
    "Vonnie",
    "William",
    "Xander",
    "Yukiko",
    "Zachariah",
    "Arlene",
    "Bethann",
    "Camellia",
    "Danuta",
    "Emmaline",
    "Floretta",
    "Gayle",
    "Hollie",
    "Jeane",
    "Kathrin",
    "Laurie",
    "Marianela",
    "Nichole",
    "Rana",
    "Shara",
    "Toni",
    "Vallie",
    "Yanira",
    "Ava",
    "Beula",
    "Buddy",
    "Carol",
    "Daren",
    "Enriqueta",
    "Francoise",
    "Georgann",
    "Jeanetta",
    "Keenan",
    "Leia",
    "Marie",
    "Rebecca",
    "Sharyn",
    "Trish",
    "Velvet",
    "Versie",
    "Vickey",
    "Carolynn",
    "Dick",
    "Donte",
    "Errol",
    "Frederic",
    "Giuseppe",
    "Jeannette",
    "Keesha",
    "Lelah",
    "Matt",
    "Reynalda",
    "Sherill",
    "Chanelle",
    "Frederica",
    "Joye",
    "Kimbra",
    "Lena",
    "Myrl",
    "Rich",
    "Steven",
    "Cherish",
    "Cinderella",
    "Cindie",
    "Clyde",
    "Contessa",
    "Cordelia",
    "Cyndi",
    "Julienne",
    "Kera",
    "Leonila",
    "Lourdes",
    "Lida",
    "Merlyn",
    "Milford",
    "Mindy",
    "Rick",
    "Rod",
    "Romona",
    "Ronald",
    "Roselee",
    "Shonda",
    "Solange",
};

void
connection_store_init(connection_store_t *c) {
    memset(c->connections, 0, sizeof (c->connections));
    c->count = 1;
}

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

void
connection_connect(connection_store_t *c, const char *key, parch_msg_t *msg) {
    int i = connection_store_find_worker(c, key);
    if (i == 0) {
        char *name = parch_msg_service(msg);
        uint8_t throughput = parch_msg_throughput(msg);
        direction_t direction = parch_msg_directionality(msg);
        throughput = parch_throughput_index_throttle(throughput, THROUGHPUT_INDEX_MAX);
        direction = parch_direction_throttle(direction, direction_bidirectional);
        c->connections[c->count].key = strdup(key);
        c->connections[c->count].address = zframe_dup(parch_msg_address(msg));
        strncpy(c->connections[c->count].dname, dnames[dindex++], 16);
        strncpy(c->connections[c->count].name, name, 40);
        c->connections[c->count].throughput_index = throughput;
        c->connections[c->count].direction = direction;
        NOTE("adding %s as connection %s", key, c->connections[c->count].dname);
        c->count++;
        NOTE("%d connections exist", c->count);

        parch_msg_t *reply = parch_msg_new(PARCH_MSG_CONNECT_INDICATION);
        parch_msg_set_throughput(reply, throughput);
        parch_msg_set_directionality(reply, direction);
        parch_msg_set_service(reply, name);
        connection_msg_send(c, key, &reply);
    }
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
connection_msg_send(connection_store_t *c, const char *key, parch_msg_t **msg) {
    // Note that this message may not have an address frame yet

    int i = connection_store_find_worker(c, key);
    if (i > 0) {
        parch_msg_set_address(*msg, c->connections[i].address);
        INFO("sending '%s' to %s", parch_msg_command(*msg), dname(key));
        parch_msg_send(msg, sock);
    }
    else {
        WARN("failed to send %s to %s - worker not found", parch_msg_command(*msg), key);
    }
}

void
connection_dispatch(connection_store_t *c, const char *key, parch_msg_t *msg) {
    // This dispatcher is for messages from already-registered workers
    // that aren't connected to peers.  Thus the only valid messages
    // are DISCONNECT and CALL_REQUEST
    int msg_id = parch_msg_id(msg);
    if (msg_id == PARCH_MSG_DISCONNECT) {
        connection_disconnect(c, key);
    } else if (msg_id == PARCH_MSG_CALL_REQUEST) {
        // Look for a peer key
        uint16_t y = connection_store_find_worker_by_name(c, parch_msg_service(msg), key);
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

