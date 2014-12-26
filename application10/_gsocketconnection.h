#pragma once

#include <glib.h>
#include <gio/gio.h>

// Returns a freshly allocated string containing the IP address
// of the remote side of a socket connection.
gchar *_g_socket_connection_get_remote_address_str (GSocketConnection *S);

// Returns the port number of the remote side of a socket connection
guint16 _g_socket_connection_get_remote_port (GSocketConnection *S);

// Returns a GSource that can be used to make a main loop trigger for
// when the input socket has data waiting
GSource *_g_socket_connection_input_stream_create_source (GSocketConnection *S);
