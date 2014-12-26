#pragma once
#include <glib.h>
#include <gio/gio.h>
#include "iodir.h"
#include "msg.h"

// To avoid circular dependency
typedef struct _JzChannel JzChannel;

typedef enum {
	C1_UNITIALIZED = 0,
	C2_CLIENT_RESTART_REQUEST,
	C3_SERVER_RESTART_REQUEST,
    C4_INITIALIZED
} client_state_t;

#define CLIENT_RECV_BUFFER_SIZE (1024)

struct _JzClient {
  GSocketConnection *connection;
  gboolean has_connection;

  // Text representation of the client's IP address
  char *inet_address_str;
  guint16 port;

  // Registration of the socket's input stream as a main loop source
  GSource *input_stream_source;

  // Connection state
  client_state_t state;
  gchar *address;
  gchar *hostname;
  iodir_t iodir;
  
  // Timers IDs, like from g_timeout_add
  // guint restart_timer_id;
  // guint heartbeat_timer_id;

  // The input buffer
  guint8 recv_buffer[1024];
  gsize recv_buffer_size;

  // The input message queue of unprocessed messages
  GQueue *incoming;

  // The list of 
  GHashTable *channels;
};

typedef struct _JzClient JzClient;

// Creates a new JzClient, freshly allocated.
JzClient *jz_client_new (void);

// Frees the memory allocated for the JzClient. Only call this
// function if queue was created with jz_client_new().
void jz_client_free (JzClient *x);

// Attaches a socket connection to a JzClient so as to be managed as
// part of a connection to a client.
void jz_client_take_socket_connection (JzClient *C, GSocketConnection *S);

JzClient *jz_client_list_find_client (gchar *address);

JzChannel *jz_client_reserve_low_channel (JzClient *C);

JzChannel *jz_channel_new (JzClient *client);

void
jz_client_send_msg (JzClient *C, JzMsg *M);

