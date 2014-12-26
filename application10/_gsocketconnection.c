#include "_gsocketconnection.h"

// Returns a freshly allocated string containing the IP address
// of a socket connection.
gchar *
_g_socket_connection_get_remote_address_str (GSocketConnection *S)
{
  GSocketAddress *addr;
  gchar *str;

  addr = g_socket_connection_get_remote_address (S, NULL);
  str = g_inet_address_to_string (g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (addr)));
  
  return str;
}

// Returns the port number of the remote side of a socket connection
guint16 _g_socket_connection_get_remote_port (GSocketConnection *S)
{
  GSocketAddress *addr;
  guint16 port;

  addr = g_socket_connection_get_remote_address (S, NULL);
  port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (addr));
  
  return port;
}

// Returns a GSource that can be used to make a main loop trigger for
// when the input socket has data waiting
GSource *_g_socket_connection_input_stream_create_source (GSocketConnection *S)
{
  GInputStream *in;
  GPollableInputStream *pollable;
  GSource *source;
  
  in = g_io_stream_get_input_stream (G_IO_STREAM (S));
  pollable = G_POLLABLE_INPUT_STREAM (in);
  source = g_pollable_input_stream_create_source (pollable, NULL);
  return source;
}
