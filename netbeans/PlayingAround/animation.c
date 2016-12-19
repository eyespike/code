
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include "qdbmp.h"



BMP* negative()
{

	UCHAR	r, g, b;
	UINT	width, height;
	UINT	x, y;
	BMP*	bmp;

	
	/* Read an image file */
	bmp = BMP_ReadFile( "howdy.bmp" );
	BMP_CHECK_ERROR( stdout, -1 );

	/* Get image's dimensions */
	width = BMP_GetWidth( bmp );
	height = BMP_GetHeight( bmp );

	/* Iterate through all the image's pixels */
	for ( x = 0 ; x < width ; ++x )
	{
		for ( y = 0 ; y < height ; ++y )
		{
			/* Get pixel's RGB values */
			BMP_GetPixelRGB( bmp, x, y, &r, &g, &b );

			/* Invert RGB values */
			BMP_SetPixelRGB( bmp, x, y, 255 - r, 255 - g, 255 - b );
		}
	}

	/* Save result */
	//BMP_WriteFile( bmp, "negative.bmp" );
	//BMP_CHECK_ERROR( stdout, -2 );


	/* Free all memory allocated for the image */
	//BMP_Free( bmp );

	return bmp;
}


void pix_destroy(guchar *pixels, gpointer data)
{

	BMP_Free((BMP*)pixels);
	
}


static gboolean
da_expose (GtkWidget *da, GdkEvent *event, gpointer data)
{
    (void)event; (void)data;
    GdkPixbuf *pix;
    GError *err = NULL;
    /* Create pixbuf */
    pix = gdk_pixbuf_new_from_file("no-image.gif", &err);
	BMP* bmp = negative();
	printf("Depth: %d\n", BMP_GetDepth(bmp));
	//printf("BMP array length: %d\n", sizeof((UCHAR*)bmp->Data));
	
	UCHAR* bmp_data = bmp->Data;
	//BMP_Free(bmp);
/*
	pix = gdk_pixbuf_new_from_bytes (bmp->Data,
			GDK_COLORSPACE_RGB,
			FALSE,
			BMP_GetDepth(bmp),
			480,
			360,
			1440);
*/
	
/*
	pix = gdk_pixbuf_new_from_data((guchar)bmp,
			GDK_COLORSPACE_RGB,
			FALSE,
			BMP_GetDepth(bmp),
			480,
			360,
			1440,
			pix_destroy,
			NULL
			);
*/
	if(err)
	{
		printf("Error : %s\n", err->message);
		g_error_free(err);
		return FALSE;
	}
    cairo_t *cr;
    cr = gdk_cairo_create (gtk_widget_get_window(da));
    //    cr = gdk_cairo_create (da->window);
    gdk_cairo_set_source_pixbuf(cr, pix, 0, 0);
    cairo_paint(cr);
    //    cairo_fill (cr);
    cairo_destroy (cr);
    //    return FALSE;
}

int main ( int argc, char **argv) {
    GtkWidget *window;
    GtkWidget *canvas;
    gtk_init (&argc , &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window,
        600, 600);

    g_signal_connect (window, "destroy",
        G_CALLBACK (gtk_main_quit) , NULL);
    canvas = gtk_drawing_area_new ();
    gtk_container_add (GTK_CONTAINER (window), canvas);
    g_signal_connect (canvas, "draw", (GCallback) da_expose, NULL);

    gtk_widget_set_app_paintable(canvas, TRUE);
    gtk_widget_show_all (window);
    gtk_main ();
    return 0;
}




/*
#include<stdio.h>
   unsigned char bitmap[1300];


void BMPmake()
{
    int i,noColor=256,end_color=54+4*noColor;
    static unsigned char temp=0;
    // -- FILE HEADER -- //

    // bitmap signature
    bitmap[0] = 'B';
    bitmap[1] = 'M';

    // file size
    bitmap[2] = 0xc6; // 40 + 14 + 256*4+400
    bitmap[3] = 0x05;
    bitmap[4] = 0;
    bitmap[5] = 0;

    // reserved field (in hex. 00 00 00 00)
    for( i = 6; i < 10; i++) bitmap[i] = 0;

    // offset of pixel data inside the image
    //The offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found.
    //here 1078
    bitmap[10]=0x36;
    bitmap[11]=0x04;
    for( i = 12; i < 14; i++) bitmap[i] = 0;


    // -- BITMAP HEADER -- //

    // header size
    bitmap[14] = 40;
    for( i = 15; i < 18; i++) bitmap[i] = 0;

    // width of the image
    bitmap[18] = 20;
    for( i = 19; i < 22; i++) bitmap[i] = 0;

    // height of the image
    bitmap[22] = 20;
    for( i = 23; i < 26; i++) bitmap[i] = 0;

    // no of color planes, must be 1
    bitmap[26] = 1;
    bitmap[27] = 0;

    // number of bits per pixel
    bitmap[28] = 8; // 1 byte
    bitmap[29] = 0;

    // compression method (no compression here)
    for( i = 30; i < 34; i++) bitmap[i] = 0;

    // size of pixel data
    bitmap[34] = 0x90; // 400 bytes => 400 pixels ,,,, 20x20x1
    bitmap[35] = 0x01;//0x190
    bitmap[36] = 0;
    bitmap[37] = 0;

    // horizontal resolution of the image - pixels per meter (2835)
    bitmap[38] = 0;
    bitmap[39] = 0;
    bitmap[40] = 0;
    bitmap[41] = 0;

    // vertical resolution of the image - pixels per meter (2835)
    bitmap[42] = 0;
    bitmap[43] = 0;
    bitmap[44] = 0;
    bitmap[45] = 0;

    // color palette information here 256
    bitmap[46]=0;
    bitmap[47]=1;
    for( i = 48; i < 50; i++) bitmap[i] = 0;

    // number of important colors
    // if 0 then all colors are important
    for( i = 50; i < 54; i++) bitmap[i] = 0;

    //Color Palette
    //for less then or equal to 8 bit BMP Image we have to create a 4*noofcolor size color palette which is nothing but
    //[BLUE][GREEN][RED][ZERO] values
    //for 8 bit we have the following code
    for (i=54;i<end_color;i+=4)
    {
        bitmap[i]=temp;
        bitmap[i+1]=temp;
        bitmap[i+2]=temp;
        bitmap[i+3]=0;
        temp++;
    }

    // -- PIXEL DATA -- //
    for( i = end_color; i < end_color+400; i++) bitmap[i] = 0xff;
}

void BMPwrite()
{
    FILE *file;
    int i;

    //use wb+ when writing to binary file .i.e. in binary form whereas w+ for txt file.
    file = fopen("b.bmp", "wb+");
    for( i = 0; i < 1478; i++)
    {
        fputc(bitmap[i], file);
    }
    fclose(file);
}
void main()
{

    BMPmake();
    BMPwrite();
    printf("Done!!");
}
*/


/*

#include <cairo.h>
#include <gtk/gtk.h>

static void do_drawing(cairo_t *);

struct {
  int count;
  double coordx[100];
  double coordy[100];
} glob;

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{
  do_drawing(cr);

  return FALSE;
}

static void do_drawing(cairo_t *cr)
{
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_set_line_width(cr, 0.5);

  int i, j;
  for (i = 0; i <= glob.count - 1; i++ ) {
      for (j = 0; j <= glob.count - 1; j++ ) {
          cairo_move_to(cr, glob.coordx[i], glob.coordy[i]);
          cairo_line_to(cr, glob.coordx[j], glob.coordy[j]);
      }
  }

  glob.count = 0;
  cairo_stroke(cr);    
}

static gboolean clicked(GtkWidget *widget, GdkEventButton *event,
    gpointer user_data)
{
    if (event->button == 1) {
        glob.coordx[glob.count] = event->x;
        glob.coordy[glob.count++] = event->y;
    }

    if (event->button == 3) {
        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}


int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;
  
  glob.count = 0;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), darea);
 
  gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_draw_event), NULL); 
  g_signal_connect(window, "destroy",
      G_CALLBACK(gtk_main_quit), NULL);  
    
  g_signal_connect(window, "button-press-event", 
      G_CALLBACK(clicked), NULL);
 
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300); 
  gtk_window_set_title(GTK_WINDOW(window), "Lines");

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
*/
