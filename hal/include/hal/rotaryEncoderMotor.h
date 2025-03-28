// Runs the rotary encoder thread which changes the led_emitters 
// frequency based on the direction of the rotary encoder.
#ifndef _ROTARY_ENCODER_MOTOR_H_
#define _ROTARY_ENCODER_MOTOR_H_

#include <stdbool.h>


extern bool done;


void rotaryEncoder_init();
void rotaryEncoder_cleanup();

#endif