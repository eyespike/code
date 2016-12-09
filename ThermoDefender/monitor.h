/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   monitor.h
 * Author: Jeff
 *
 * Created on December 5, 2016, 8:31 AM
 */

#ifndef MONITOR_H
#define MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

extern bool _monitorActive;	
char* save_pgm_file(void);
int connect_to_lepton();
int transfer(int fd);
void* f_monitor(void *arg);


#ifdef __cplusplus
}
#endif

#endif /* MONITOR_H */

