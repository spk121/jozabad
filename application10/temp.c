#include <glib.h>
#include <gio/gio.h>
#include "msg.h"

int
main (int argc, char *argv[])
{
   /* initialize glib */
  g_type_init ();

  GError * error = NULL;

  /* create a new connection */
  GSocketConnection *X = NULL, *Y = NULL;
  GSocketClient *clientX = g_socket_client_new();
  GSocketClient *clientY = g_socket_client_new();

  /* connect to the host */
  X = g_socket_client_connect_to_host (clientX,
                                       (gchar*)"localhost",
                                       8516, /* your port goes here */
                                       NULL,
                                       &error);
  Y = g_socket_client_connect_to_host (clientY,
                                       (gchar*)"localhost",
                                       8516, /* your port goes here */
                                       NULL,
                                       &error);

  /* use the connection */
  GInputStream * istreamX = g_io_stream_get_input_stream (G_IO_STREAM (X));
  GOutputStream * ostreamX = g_io_stream_get_output_stream (G_IO_STREAM (X));
  GInputStream * istreamY = g_io_stream_get_input_stream (G_IO_STREAM (Y));
  GOutputStream * ostreamY = g_io_stream_get_output_stream (G_IO_STREAM (Y));

  // A CONNECT message that registers this client under the name XXX
  JzMsg *msg1 = jz_msg_new_connect ("XXX", 0);
  GByteArray *arr1 = jz_msg_to_byte_array (msg1);
  g_output_stream_write  (ostreamX,
                          arr1->data,
                          arr1->len, /* length of your message */
                          NULL,
                          &error);

  JzMsg *msg2 = jz_msg_new_connect ("YYY", 0);
  GByteArray *arr2 = jz_msg_to_byte_array (msg2);
  g_output_stream_write  (ostreamY,
                          arr2->data, arr2->len,
                          NULL,
                          &error);

  g_usleep(100000);

  // A CALL request
  GByteArray *empty_data = g_byte_array_new();
  JzMsg *msg3 = jz_msg_new_call_request (1 /* LCN */,
                                         "XXX" /* Calling address */,
                                         "YYY" /* called adress */,
                                         3, // packet facility
                                         3, // window facility
                                         3, // throughput facility
                                         empty_data);

  GByteArray *arr3 = jz_msg_to_byte_array (msg3);
  g_output_stream_write (ostreamX,
                         arr3->data, arr3->len,
                         NULL,
                         &error);

  
  g_usleep(2 * 1000000);
  
  return 0;
}
