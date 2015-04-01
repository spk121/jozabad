// #include <gtk/gtk.h>
#include <glib.h>
#include <glib-unix.h>
// #include "exampleapp.h"
#include <stdbool.h>
#include "service.h"

GMainLoop *loop = NULL;

int
signal_handler (void *user_data)
{
	service_fini ();
	g_main_loop_quit (loop);
	return G_SOURCE_CONTINUE;
}

int
main (int argc, char *argv[])
{
#if 0	
  /* Since this example is running uninstalled,
   * we have to help it find its schema. This
   * is *not* necessary in properly installed
   * application.
   */
  g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);

  return g_application_run (G_APPLICATION (example_app_new ()), argc, argv);
#endif

  
  loop = g_main_loop_new (NULL, FALSE);
  g_unix_signal_add (SIGINT, signal_handler, NULL);
  service_init();
  service_start_accepting_new_connections();
  g_main_loop_run (loop);
  service_fini();
  g_main_loop_unref (loop);
  
  return 0;
}
