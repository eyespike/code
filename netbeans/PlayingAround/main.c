 /*#include <gtk/gtk.h>

static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
  g_print ("Hello World\n");
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *button_box;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 200);

  button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (window), button_box);

  button = gtk_button_new_with_label ("Hello World - Destroy Me!!!");
  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add (GTK_CONTAINER (button_box), button);

  gtk_widget_show_all (window);
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
*/

/*

#include <gtk/gtk.h>

static void print_hello (GtkWidget *widget, gpointer data)
{
  g_print ("Hello World\n");
}

void changeImage(GtkWidget *image)
{
	gtk_image_set_from_file (image, "/home/pi/PlayingAround/dist/Debug/GNU-Linux/l_IMG_0002.pgm");


}

int main( int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *layout;
    GtkWidget *image;
    GtkWidget *button;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 380);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER (window), layout);
    gtk_widget_show(layout);

    //image = gtk_image_new_from_file("/home/pi/Pictures/puppy.jpg");
	image = gtk_image_new_from_file("/home/pi/PlayingAround/dist/Debug/GNU-Linux/l_IMG_0003.pgm");
    gtk_layout_put(GTK_LAYOUT(layout), image, 0, 0);

    button = gtk_button_new_with_label("Close Me");
	//g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
	//g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (changeImage), image);
	
    gtk_layout_put(GTK_LAYOUT(layout), button, 150, 50);
    gtk_widget_set_size_request(button, 80, 35);

    g_signal_connect_swapped(G_OBJECT(window), "destroy",
    G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
*/



#include <gtk/gtk.h>
#include <stdio.h>
#include <stdbool.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

// --- Constants
static const int LOOPTIMES = 100;

// --- Global Variables
GtkApplication *app_ui;
pthread_t tid[2];
bool _active = false;
bool _monitorActive = false;


void* monitor(void *arg)
{
	_monitorActive = true;
	
	int currentIteration = 0;
	
	while(_active){
	
		usleep(100000);
		currentIteration ++;
		if(currentIteration > LOOPTIMES){
			_active = !_active;
		} else {
			printf("Iteration: %d\n", currentIteration);
		}
	}
	
	_monitorActive = false;
	pthread_exit(NULL);	
}


GtkWidget* find_child(GtkWidget* parent, const gchar* name)
{
	const gchar* parentName = gtk_widget_get_name(parent);
	if (strcmp(g_utf8_casefold(parentName, sizeof(parentName)), g_utf8_casefold(name, sizeof(name))) == 0) { 
			return parent;
	}

	if (GTK_IS_BIN(parent)) {
			GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
			return find_child(child, name);
	}

	if (GTK_IS_CONTAINER(parent)) {
			GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
			do {
					GtkWidget* widget = find_child(children->data, name);
					if (widget != NULL) {
							return widget;
					}
			} while ((children = g_list_next(children)) != NULL);
	}

	return NULL;
}



static void toggle_status (GtkWidget *widget, gpointer *data)
{	
	
	GtkWindow *window = gtk_application_get_active_window(app_ui);
/*
	GtkWidget* button = find_child((GtkWidget*)window, "button1");
	GtkWidget* label = find_child((GtkWidget*)window, "statuslabel");
*/
	
/*
	if(_active){
		gtk_label_set_text((GtkLabel*)label, "Inactive");
		gtk_button_set_label((GtkButton*)button, "Start");
		_active = !_active;
	} else if(!_monitorActive) {
		gtk_label_set_text((GtkLabel*)label, "Monitoring");
		gtk_button_set_label((GtkButton*)button, "Stop");
		_active = !_active;
		
		// Spin up thread to start monitoring
		int err;
		err = pthread_create(&(tid[0]), NULL, &monitor, NULL);
		if(err != 0)
			printf("\nThreading error: [%s]", strerror(err));
		else
			printf("\nMonitoring started\n");
	}	
*/

	if(_active){
		gtk_label_set_text((GtkLabel*)data, "Inactive");
		gtk_button_set_label((GtkButton*)widget, "Start");
		_active = !_active;
	} else if(!_monitorActive) {
		gtk_label_set_text((GtkLabel*)data, "Monitoring");
		gtk_button_set_label((GtkButton*)widget, "Stop");
		_active = !_active;
		
		// Spin up thread to start monitoring
		int err;
		err = pthread_create(&(tid[0]), NULL, &monitor, NULL);
		if(err != 0)
			printf("\nThreading error: [%s]", strerror(err));
		else
			printf("\nMonitoring started\n");
	}	

}
int main (int argc, char *argv[])
{
  GtkBuilder *builder;
  GObject *window;
  GObject *button;
  GObject *label;

  gtk_init (&argc, &argv);

  // Construct a GtkBuilder instance and load our UI description
  app_ui = gtk_application_new ("org.gnome.example", G_APPLICATION_FLAGS_NONE);
  builder = gtk_builder_new ();
  gtk_builder_set_application(builder, app_ui);
  gtk_builder_add_from_file (builder, "builder.xml", NULL);
    
  // Connect signal handlers to the constructed widgets.
  window = gtk_builder_get_object (builder, "window");
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  label = gtk_builder_get_object (builder, "statuslabel");
  button = gtk_builder_get_object (builder, "button1");
  g_signal_connect (button, "clicked", G_CALLBACK (toggle_status), (gpointer)label);
  
  button = gtk_builder_get_object (builder, "quit");
  g_signal_connect (button, "clicked", G_CALLBACK (gtk_main_quit), NULL);

  gtk_main ();
 
  return 0;
}
