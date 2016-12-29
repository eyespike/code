/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tdio.h
 * Author: Jeff
 *
 * Created on December 14, 2016, 4:01 PM
 */

#ifndef TDIO_H
#define TDIO_H

#ifdef __cplusplus
extern "C" {
#endif

	
void initializeGpio();	
int set_gpio_12(int state);
int set_gpio_16(int state);




#ifdef __cplusplus
}
#endif

#endif /* TDIO_H */