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

#ifdef __cplusplus
extern "C" {
#endif
extern GMainContext *mainc;	
extern bool _active;
extern GtkLabel *statusLabel;
extern GtkButton *monitorButton;

void update_monitor_status_labels();

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */

