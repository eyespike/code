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
extern bool _demoFinished;
extern int _currentIteration;

extern int water_tc_differential;
extern int body_tc_differential;
extern int fire_tc_differential;
extern int water_min_detected_pc;
extern int body_min_detected_pc;
extern int fire_min_detected_pc;

extern bool floodIconUpdated;
extern bool floodConfirmedIconUpdated;
extern bool waterShutoffIconUpdated;
extern bool noticeSentIconUpdated;

extern uint8_t PUMP_START_DELAY_SECS;
extern uint8_t WATER_DETECTED_SECS;
extern uint8_t WATER_DETECTED_CONFIRMED_SECS_TO_SHUTDOWN;
extern uint8_t NOTIFCATION_SENT_DELAY_SECS;

char* save_pgm_file(void);
int connect_to_lepton();
int transfer(int fd);
void* iterate_lepton(void *arg);

bool set_reference_frame();
int get_tc_difference(int min_diff, int min_pixel_count);

#ifdef __cplusplus
}
#endif

#endif /* MONITOR_H */

