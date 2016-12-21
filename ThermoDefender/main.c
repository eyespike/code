#include <gtk/gtk.h>
#include <stdio.h>
#include <stdbool.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "monitor.h"
#include "tdio.h"
#include "qdbmp.h"

// --- UI Global Variables
GMainContext *mainc;
GtkApplication *app_ui;
GtkWindow *main_window;
GtkLabel *statusLabel;
GtkButton *monitorButton;
GtkImage *captureImage;
GtkDrawingArea *videoArea;


 typedef struct {
	  GtkEntry *waterDiff;
	  GtkEntry *bodyDiff;
	  GtkEntry *fireDiff;
	  GtkEntry *minWaterPC;
	  GtkEntry *minBodyPC;
	  GtkEntry *minFirePC;
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
	while(transfer(fd)!=59){}
	close(fd);
	set_capture_image_from_current_array(captureImage);
}


//gboolean video_area_expose (GtkWidget *da, GdkEvent *event, gpointer data)
void video_area_expose (GtkWidget *da, BMP *bmp)
{
	
	//(void)event; void(data);
	GdkPixbuf *pix;
	GError *err = NULL;
		
/*
	if(data == NULL){
		pix = gdk_pixbuf_new_from_file ("no-video.gif", &err);
	}
*/
	//else
	//	pix = gdk_pixbuf_new_from_file ("capture.bmp", &err);
		
	//else
	//{
		//
		//UCHAR *pixData = (UCHAR*)data;
		
		//BMP *bmp = (BMP*)data;
		//BMP *bmp = BMP_ReadFile( "capture.bmp" );
		
		//UCHAR *pixels = (UCHAR*)data;
		
		pix = gdk_pixbuf_new_from_data ((guchar*)BMP_GetBytes(bmp),
			GDK_COLORSPACE_RGB,
			FALSE,
			8,80,60,80,
			pix_destroy,
			bmp);
	//}

    cairo_t *cr;
    cr = gdk_cairo_create (gtk_widget_get_window(da));
    //    cr = gdk_cairo_create (da->window);
    gdk_cairo_set_source_pixbuf(cr, pix, 0, 10);
    cairo_paint(cr);
    //    cairo_fill (cr);
    cairo_destroy (cr);
	
	BMP_Free(bmp);
	//return TRUE;
}


static void set_detect_variables (GtkWidget *widget, VariableEntries *data)
{
	
	water_tc_differential = (int)strtol(gtk_entry_get_text (data->waterDiff), (char **) NULL, 10);
	body_tc_differential = (int)strtol(gtk_entry_get_text (data->bodyDiff), (char **) NULL, 10);
	fire_tc_differential = (int)strtol(gtk_entry_get_text (data->fireDiff), (char **) NULL, 10);
	water_min_detected_pc = (int)strtol(gtk_entry_get_text (data->minWaterPC), (char **) NULL, 10);
	body_min_detected_pc = (int)strtol(gtk_entry_get_text (data->minBodyPC), (char **) NULL, 10);
	fire_min_detected_pc = (int)strtol(gtk_entry_get_text (data->minFirePC), (char **) NULL, 10);
	
	printf("Water Diff: %d\nBody Diff: %d\nFire Diff: %d\n", water_tc_differential, body_tc_differential, fire_tc_differential);
	printf("Water PC: %d\nBody PC: %d\nFire PC: %d\n", water_min_detected_pc, body_min_detected_pc, fire_min_detected_pc);
}

static void calibrate_water (GtkWidget *widget, VariableEntries *data)
{
	set_reference_frame();
	
	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new (main_window,
									 flags,
									 GTK_MESSAGE_INFO,
									 GTK_BUTTONS_OK,
									 "Reference Image Set.\nPlace the water in the camera frame click 'OK'");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	// Get the thermal count difference
	int tc_diff = get_tc_difference(40, water_min_detected_pc); // Water
	
	if(tc_diff != 0){
		gchar varStr[8];
		
		if(tc_diff < 0)
			sprintf(varStr, "%d", tc_diff + 10);
		else
			sprintf(varStr, "%d", tc_diff - 5);
		
		gtk_entry_set_text(data->waterDiff, varStr);
	} else {
		printf("Could not detect difference.");	
	}	
}


static void calibrate_body (GtkWidget *widget, VariableEntries *data)
{
	set_reference_frame();
	
	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new (main_window,
									 flags,
									 GTK_MESSAGE_INFO,
									 GTK_BUTTONS_OK,
									 "Reference Image Set.\nPlace your hand in the camera frame click 'OK'");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	// Get the thermal count difference
	int tc_diff = get_tc_difference(100, body_min_detected_pc);
		
	if(tc_diff != 0){
		gchar varStr[8];
		sprintf(varStr, "%d", tc_diff - 5); // Reducing the difference since the average is returned.
		gtk_entry_set_text(data->bodyDiff, varStr);
	} else {
		printf("Could not detect difference.");	
	}	
}


static void calibrate_fire (GtkWidget *widget, VariableEntries *data)
{
	set_reference_frame();
	
	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new (main_window,
									 flags,
									 GTK_MESSAGE_INFO,
									 GTK_BUTTONS_OK,
									 "Reference Image Set.\nPlace some fire in the camera frame click 'OK'");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	// Get the thermal count difference
	int tc_diff = get_tc_difference(300, fire_min_detected_pc); 
			
	if(tc_diff != 0){
		gchar varStr[8];
		sprintf(varStr, "%d", tc_diff - 20); // Reducing the difference since the average is returned.
		gtk_entry_set_text(data->fireDiff, varStr);
	} else {
		printf("Could not detect difference.");	
	}	
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
  GObject *grid;
  
  
  // Initialize the IO
  initializeGpio();
  
  //--- Construct a GtkBuilder instance and load our UI description
  gtk_init (&argc, &argv);
  mainc = g_main_context_default();
  app_ui = gtk_application_new ("org.gnome.example", G_APPLICATION_FLAGS_NONE);
  builder = gtk_builder_new ();
  gtk_builder_set_application(builder, app_ui);
  gtk_builder_add_from_file (builder, "builder.xml", NULL);
    
  //--- Connect signal handlers to the constructed widgets.
  window = gtk_builder_get_object (builder, "mainWindow");
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  main_window = (GtkWindow*)window;
  //gtk_window_fullscreen(window);
  
  grid = gtk_builder_get_object (builder, "grid");
  gtk_widget_set_halign((GtkWidget*)grid, GTK_ALIGN_CENTER);
  gtk_widget_set_valign((GtkWidget*)grid, GTK_ALIGN_CENTER);
  
  //--- Detection Variables
  VariableEntries detectionEntries;
  
  gchar varStr[8];
  sprintf(varStr, "%d", water_tc_differential);
  detectionEntries.waterDiff = (GtkEntry*)gtk_builder_get_object (builder, "eWaterTcDiff");
  gtk_entry_set_text(detectionEntries.waterDiff, varStr);
  
  sprintf(varStr, "%d", body_tc_differential);
  detectionEntries.bodyDiff = (GtkEntry*)gtk_builder_get_object (builder, "eBodyTcDiff");
  gtk_entry_set_text(detectionEntries.bodyDiff, varStr);
  
  sprintf(varStr, "%d", fire_tc_differential);
  detectionEntries.fireDiff = (GtkEntry*)gtk_builder_get_object (builder, "eFireTcDiff");
  gtk_entry_set_text(detectionEntries.fireDiff, varStr);
    
  sprintf(varStr, "%d", water_min_detected_pc);
  detectionEntries.minWaterPC = (GtkEntry*)gtk_builder_get_object (builder, "eWaterPC");
  gtk_entry_set_text(detectionEntries.minWaterPC, varStr);
  
  sprintf(varStr, "%d", body_min_detected_pc);
  detectionEntries.minBodyPC = (GtkEntry*)gtk_builder_get_object (builder, "eBodyPC");
  gtk_entry_set_text(detectionEntries.minBodyPC, varStr);
  
  sprintf(varStr, "%d", fire_min_detected_pc);
  detectionEntries.minFirePC = (GtkEntry*)gtk_builder_get_object (builder, "eFirePC");
  gtk_entry_set_text(detectionEntries.minFirePC, varStr);
  
  button = gtk_builder_get_object (builder, "btnSetVariables");
  g_signal_connect (button, "clicked", G_CALLBACK (set_detect_variables), (gpointer)&detectionEntries);
  
  //--- Calibrate Variables
  button = gtk_builder_get_object (builder, "btnCalibrateWater");
  g_signal_connect (button, "clicked", G_CALLBACK (calibrate_water), (gpointer)&detectionEntries);
  
  button = gtk_builder_get_object (builder, "btnCalibrateBody");
  g_signal_connect (button, "clicked", G_CALLBACK (calibrate_body), (gpointer)&detectionEntries);
  
  button = gtk_builder_get_object (builder, "btnCalibrateFire");
  g_signal_connect (button, "clicked", G_CALLBACK (calibrate_fire), (gpointer)&detectionEntries);
  
  
  //--- I/O buttons
  button = gtk_builder_get_object (builder, "btnGpio12");
  g_signal_connect (button, "clicked", G_CALLBACK (toggle_gpio_12), (gpointer)true);
  
  button = gtk_builder_get_object (builder, "btnGpio16");
  g_signal_connect (button, "clicked", G_CALLBACK (toggle_gpio_16), (gpointer)true);
  
  
  //--- Capture Image
  captureImage = (GtkImage*)gtk_builder_get_object (builder, "captureImage");
  gtk_image_set_from_file (captureImage, "no-image.gif");
  gtk_widget_set_halign ((GtkWidget*)captureImage, GTK_ALIGN_CENTER);
  button = gtk_builder_get_object (builder, "btnCaptureFrame");
  g_signal_connect (button, "clicked", G_CALLBACK (capture_image), captureImage);
  
  
  // --- Video Area
  videoArea = (GtkDrawingArea*)gtk_builder_get_object (builder, "videoArea");
  gtk_widget_set_size_request ((GtkWidget*)videoArea, 500, 380);
  //g_signal_connect (videoArea, "draw", (GCallback) video_area_expose, NULL);
  
  //--- Monitor toggle & status
  statusLabel = (GtkLabel*)gtk_builder_get_object (builder, "statuslabel");
  monitorButton = (GtkButton*)gtk_builder_get_object (builder, "btnStartMonitor");
  g_signal_connect (monitorButton, "clicked", G_CALLBACK (toggle_monitor), NULL);
  
  //--- Quit
  button = gtk_builder_get_object (builder, "quit");
  g_signal_connect (button, "clicked", G_CALLBACK (gtk_main_quit), NULL);

  gtk_main ();
 
  return 0;
}
