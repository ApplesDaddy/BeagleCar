/**
 * rotary_encoder.h
 * source: gpio_statemachine_demo
 *
 * Part of the Hardware Abstraction Layer (HAL)
 *
 * Gives methods to read the rotary encoder's value.
 * (value is reset to 0 on start, so the initial "angle" has no impact)
 */

#ifndef _ROT_ENCODER_H_
#define _ROT_ENCODER_H_


#include <stdbool.h>


void rot_encoder_init(void);
void rot_encoder_cleanup(void);

int rot_encoder_get_val(void);
bool rot_encoder_is_pressed(void);

void rot_encoder_set_min_max(unsigned int min, unsigned int max);
void rot_encoder_set_counter(int count);
void rot_encoder_set_step(int count);
#endif