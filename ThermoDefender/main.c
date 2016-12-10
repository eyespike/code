#include <gtk/gtk.h>
#include <stdio.h>
#include <stdbool.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "monitor.h"

// --- UI Global Variables
GMainContext *mainc;
GtkApplication *app_ui;
GtkLabel *statusLabel;
GtkButton *monitorButton;
GtkImage *captureImage;


 typedef struct {
	  GtkEntry *waterDiff;
	  GtkEntry *fireDiff;
	  GtkEntry *minPixelCount;
  } VariableEntries;

pthread_t tid[2];
bool _active = false;


// --- Forward Declarations
//static void toggle_status (GtkWidget *widget, gpointer *data);

void set_capture_image_from_current_array(GtkImage *image)
{

	char* imageName = save_pgm_file();
	
	// Scale the image up and display it (using ImageMagick commands)
	char largeImageName[34];
	snprintf(largeImageName, sizeof largeImageName, "l_%s", imageName);
	
	char scaleCommand[100];
	snprintf(scaleCommand, sizeof scaleCommand, "convert %s -resize 600%% %s", imageName, largeImageName);
	system(scaleCommand);
	
	gtk_image_set_from_file (image, largeImageName);	
}


static void capture_image (GtkWidget *widget, gpointer *data)
{
	int fd = connect_to_lepton();
	//isFirstTransferFrame = true;
	while(transfer(fd)!=59){}
	close(fd);
	set_capture_image_from_current_array(captureImage);
}


static void set_detect_variables (GtkWidget *widget, VariableEntries *data)
{
	
	water_tc_differential = (int)strtol(gtk_entry_get_text (data->waterDiff), (char **) NULL, 10);
	fire_tc_differential = (int)strtol(gtk_entry_get_text (data->fireDiff), (char **) NULL, 10);
	min_detected_pixel_count = (int)strtol(gtk_entry_get_text (data->minPixelCount), (char **) NULL, 10);
	
	printf("Water Diff: %d\nFire Diff: %d\nMin Pixel Count: %d\n", water_tc_differential, fire_tc_differential, min_detected_pixel_count);
}


void update_monitor_status_labels(char *labelValue)
{	
	if(!_active){
		if(labelValue == NULL)
			gtk_label_set_text(statusLabel, "Inactive");
		else
			gtk_label_set_text(statusLabel, labelValue);
		
		gtk_button_set_label(monitorButton, "Monitor");
	} else {
		if(labelValue == NULL)
			gtk_label_set_text(statusLabel, "Monitoring");
		else
			gtk_label_set_text(statusLabel, labelValue);
		
		gtk_button_set_label(monitorButton, "Stop");
	}
}


static void toggle_monitor (GtkWidget *widget, gpointer *data)
{	
	_active = !_active;
	update_monitor_status_labels(NULL);
	
	if(_active && !_monitorActive) {
		
		// Spin up thread to start monitoring
		int err;
		err = pthread_create(&(tid[0]), NULL, &f_monitor, NULL);
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
  GObject *entry;
  
  gtk_init (&argc, &argv);

  mainc = g_main_context_default();
  
  
  // Construct a GtkBuilder instance and load our UI description
  app_ui = gtk_application_new ("org.gnome.example", G_APPLICATION_FLAGS_NONE);
  builder = gtk_builder_new ();
  gtk_builder_set_application(builder, app_ui);
  gtk_builder_add_from_file (builder, "builder.xml", NULL);
    
  // Connect signal handlers to the constructed widgets.
  window = gtk_builder_get_object (builder, "window");
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  

  // Detection Variables
  VariableEntries detectionEntries;
  
  gchar varStr[8];
  sprintf(varStr, "%d", water_tc_differential);
  detectionEntries.waterDiff = (GtkEntry*)gtk_builder_get_object (builder, "eWaterTcDiff");
  gtk_entry_set_text(detectionEntries.waterDiff, varStr);
   
  sprintf(varStr, "%d", fire_tc_differential);
  detectionEntries.fireDiff = (GtkEntry*)gtk_builder_get_object (builder, "eFireTcDiff");
  gtk_entry_set_text(detectionEntries.fireDiff, varStr);
    
  sprintf(varStr, "%d", min_detected_pixel_count);
  detectionEntries.minPixelCount = (GtkEntry*)gtk_builder_get_object (builder, "eMinPixelCount");
  gtk_entry_set_text(detectionEntries.minPixelCount, varStr);
  
  button = gtk_builder_get_object (builder, "btnSetVariables");
  g_signal_connect (button, "clicked", G_CALLBACK (set_detect_variables), (gpointer)&detectionEntries);
  
  // Capture Image
  captureImage = (GtkImage*)gtk_builder_get_object (builder, "captureImage");
  gtk_image_set_from_file (captureImage, "no-image.gif");
  gtk_widget_set_halign ((GtkWidget*)captureImage, GTK_ALIGN_CENTER);
  button = gtk_builder_get_object (builder, "btnCaptureFrame");
  g_signal_connect (button, "clicked", G_CALLBACK (capture_image), captureImage);
  
  // Monitor toggle & status
  statusLabel = (GtkLabel*)gtk_builder_get_object (builder, "statuslabel");
  monitorButton = (GtkButton*)gtk_builder_get_object (builder, "btnStartMonitor");
  g_signal_connect (monitorButton, "clicked", G_CALLBACK (toggle_monitor), NULL);
  
  // Quit
  button = gtk_builder_get_object (builder, "quit");
  g_signal_connect (button, "clicked", G_CALLBACK (gtk_main_quit), NULL);

  gtk_main ();
 
  return 0;
}
