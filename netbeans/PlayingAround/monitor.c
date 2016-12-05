/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdbool.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "main.h"


// --- Constants
static const int LOOPTIMES = 25;

bool _monitorActive = false;


gboolean set_status_monitor_ended(gpointer data){
	
	update_monitor_status_labels();
	return FALSE;
}


void* f_monitor(void *arg)
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
	// Call main thread to update the GUI
	g_main_context_invoke(mainc, set_status_monitor_ended, NULL);
	pthread_exit(NULL);	
	
}
