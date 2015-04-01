#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include "seq.h"
#include "msg.h"
#include "diag.h"
#include "iodir.h"
#include "crc.h"

char recv_buffer[1024];
size_t recv_buffer_size = 0;

void client_init (GSocketClient **client, GSocketConnection **connection)
{
  /* create the network service */
  *client = g_socket_client_new ();

  GInetAddress *inet_address = g_inet_address_new_loopback (G_SOCKET_FAMILY_IPV4);
  GSocketAddress *socket_address = g_inet_socket_address_new (inet_address, 8516);
  GError *error = NULL;

  *connection = 
    g_socket_client_connect (*client,
                             G_SOCKET_CONNECTABLE(socket_address),
                             NULL,
                             &error); 
  g_object_unref (inet_address);
  g_object_unref (socket_address);
  if (error)
    g_error ("%s", error->message);
}

void client_fini ()
{
  // There is no way to deallocate or finalize a GSocketService
}

void
client_send_msg (GSocketConnection *sock_con, JzMsg *M)
{
  GOutputStream *out = g_io_stream_get_output_stream (G_IO_STREAM (sock_con));
  gssize bytes_written;
  GByteArray *b_array = jz_msg_to_byte_array (M);
  bytes_written = g_output_stream_write (out,
                                         b_array->data,
                                         b_array->len,
                                         NULL,
                                         NULL);
  gchar *intro;
  if (bytes_written >= 0)
    intro = "Sent message";
  else
    intro = "Failed to send message";

  if (M->id == JZ_MSG_DIAGNOSTIC)
    g_message ("%s: %s %s %s", "me", intro, id_name(M->id), diag_name (M->diagnostic));
  else
    g_message ("%s: %s %s", "addr", intro, id_name(M->id));
}

void
client_send_then_free_msg (GSocketConnection *sock_con, JzMsg *M)
{
  client_send_msg (sock_con, M);
  jz_msg_free (M);
}

static gboolean
read_into_recv_buffer (GInputStream *in)
{
  gssize res;
  GError *error;

  g_assert (recv_buffer_size < sizeof (recv_buffer));

  error = NULL;
  res = g_input_stream_read (in, &recv_buffer[recv_buffer_size],
                             sizeof (recv_buffer) - recv_buffer_size,
                             NULL, &error);

  if (res > 0)
    {
      g_message ("received %d bytes", res);
      recv_buffer_size += res;
    }
  else if (res < 0 && g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
    {
      g_error_free (error);
      res = 0;
    }
  else if (res <= 0)
    {
      if (error)
        g_message ("%s", error ? error->message : "eof");

      // Since the server has disappeared, we give up
      return FALSE;
    }

  return TRUE;
}

#define GET(___IN)                                                     \
  do {                                                                  \
    recv_buffer_size = 0;                                               \
    read_into_recv_buffer(___IN);                                        \
    jz_buffer_parse_into_message (recv_buffer, recv_buffer_size, &siz, &msg_recv, \
                                  &is_error, &is_fatal);                \
    if (is_fatal)                                                       \
      printf("received fatally corrupt message\n");                     \
    else if (is_error)                                                  \
      printf("received erroneous message\n");                           \
    else                                                                \
      printf("received %s message\n", id_name(msg_recv->id));           \
  } while (0)

int main(int argc, char **argv)
{
  GSocketClient *client_X, *client_Y;
  GSocketConnection *connection_X = NULL, *connection_Y;
  gsize siz;
  JzMsg *msg_recv;
  gboolean is_error, is_fatal;

  client_init(&client_X, &connection_X);
  client_init(&client_Y, &connection_Y);

  GInputStream *in_X = g_io_stream_get_input_stream (G_IO_STREAM (connection_X));
  GInputStream *in_Y = g_io_stream_get_input_stream (G_IO_STREAM (connection_Y));

  // Connect
  client_send_then_free_msg (connection_X, jz_msg_new_connect ("ADAM", io_bidirectional));
  client_send_then_free_msg (connection_Y, jz_msg_new_connect ("EVE", io_bidirectional));

  GET(in_X);
  GET(in_Y);

  // A call request should use the logical channel with the highest number in the range.
  #define LCN_X1 103
  #define LCN_X2 102
  #define LCN_X3 101
  #define LCN_Y1 1
  #define LCN_Y2 2
  #define LCN_Y3 3
  
  client_send_then_free_msg (connection_X,
                             jz_msg_new_call_request(LCN_X1, "ADAM", "EVE", p_default, WINDOW_MAX, t_default, NULL, 0));
  GET(in_Y);

  client_send_then_free_msg (connection_Y,
                             jz_msg_new_call_accepted(LCN_Y1, "EVE", "ADAM", p_default, WINDOW_MAX, t_default, NULL, 0));
  GET(in_X);

    client_send_then_free_msg (connection_X,
                             jz_msg_new_call_request(LCN_X2, "ADAM", "EVE", p_default, WINDOW_MAX, t_default, NULL, 0));
  GET(in_Y);

  client_send_then_free_msg (connection_Y,
                             jz_msg_new_call_accepted(LCN_Y2, "EVE", "ADAM", p_default, WINDOW_MAX, t_default, NULL, 0));
  GET(in_X);

    client_send_then_free_msg (connection_X,
                             jz_msg_new_call_request(LCN_X3, "ADAM", "EVE", p_default, WINDOW_MAX, t_default, NULL, 0));
  GET(in_Y);

  client_send_then_free_msg (connection_Y,
                             jz_msg_new_call_accepted(LCN_Y3, "EVE", "ADAM", p_default, WINDOW_MAX, t_default, NULL, 0));
  GET(in_X);

  
  client_send_then_free_msg (connection_X,
                             jz_msg_new_data(LCN_X1, 0, 0, 0, g_strdup ("MESSAGE1"), 8));
  GET(in_Y);
  client_send_then_free_msg (connection_X,
                             jz_msg_new_data(LCN_X2, 0, 0, 1, g_strdup ("MESSAGE2"), 8));
  GET(in_Y);
  client_send_then_free_msg (connection_X,
                             jz_msg_new_data(LCN_X3, 0, 0, 2, g_strdup ("MESSAGE3"), 8));
  GET(in_Y);

  client_send_then_free_msg (connection_Y,
							jz_msg_new_restart_request (c_reset__client_originated, 0));
  GET(in_Y);

  client_fini();
  g_usleep (G_USEC_PER_SEC);
  
  return 0;
}
