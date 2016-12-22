/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.h
 * Author: Jeff
 *
 * Created on December 5, 2016, 8:34 AM
 */

#ifndef MAIN_H
#define MAIN_H
#include <gtk/gtk.h>
#include "qdbmp.h"

#ifdef __cplusplus
extern "C" {
#endif
extern GMainContext *mainc;	
extern bool _active;
extern GtkLabel *statusLabel;
extern GtkButton *monitorButton;
extern GtkImage *captureImage;
extern GtkDrawingArea *videoArea;

void update_monitor_status_labels();
void set_capture_image_from_current_array(GtkImage *image);
//gboolean video_area_expose (GtkWidget *da, GdkEvent *event, gpointer data);
//void video_area_expose (GtkWidget *da, BMP *bmp);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */

