#include <glib.h>
#include <gio/gio.h>
#include "client.h"

GSocketService *m_socket_service = NULL;
extern GList *client_list;


/* this function will get called everytime a client attempts to connect */
gboolean
socket_service_incoming_signal_handler  (GSocketService *service,
                                         GSocketConnection *connection,
                                         GObject *source_object,
                                         gpointer user_data)
{
  g_message ("In incoming callback");
  GSocketAddress *remote = 
    g_socket_connection_get_remote_address(connection, NULL);
  GSocketAddress *local =
    g_socket_connection_get_local_address(connection, NULL);
  
  char *remote_address_str = g_inet_address_to_string (g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (remote)));
  guint16 remote_port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (remote));
  char *local_address_str = g_inet_address_to_string (g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (local)));
  guint16 local_port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (local));
  g_message ("incoming connection %s:%u to %s:%u\n", remote_address_str, remote_port, local_address_str, local_port);
  g_free (remote_address_str);
  g_free (local_address_str);

  
  // Create a new client handler object for this connection
  JzClient *C = jz_client_new ();
  jz_client_take_socket_connection (C, connection);

  // Start the CONNECT timer.

  // Add the client to the client list
  client_list = g_list_append (client_list, C);
  
  return FALSE;
}

void service_init ()
{
  gboolean ret;

  /* create the network service */
  m_socket_service = g_socket_service_new ();
  GInetAddress *inet_address = g_inet_address_new_loopback (G_SOCKET_FAMILY_IPV4);
  GSocketAddress *socket_address = g_inet_socket_address_new (inet_address, 8516);
  g_object_unref (inet_address);

  GSocketAddress *server_sockaddr = NULL;
  GError *error = NULL;
  
  g_socket_listener_add_address (G_SOCKET_LISTENER (m_socket_service),
                                 socket_address,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 NULL,
                                 &server_sockaddr,
                                 &error);
  g_object_unref (socket_address);
  if (error)
    g_error ("%s", error->message);

  char *address_str = g_inet_address_to_string (g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (server_sockaddr)));
  guint16 port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (server_sockaddr));
  g_print ("address = %s\nport = %u\n",
           address_str, port);
  g_free (address_str);
  
  g_signal_connect (m_socket_service, "incoming",
                    G_CALLBACK (socket_service_incoming_signal_handler), NULL);

}

void service_start_accepting_new_connections ()
{
  g_socket_service_start (m_socket_service);
}

void service_stop_accepting_new_connections ()
{
  g_socket_service_stop (m_socket_service);
}

void service_fini ()
{
  // There is no way to deallocate or finalize a GSocketService
}
