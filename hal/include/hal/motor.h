#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <stdbool.h>

#define MAX_SPEED 100
#define MAX_GEAR 10
#define SPEED_STEP (MAX_SPEED)/(MAX_GEAR)

void motor_init(void);
void motor_cleanup(void);


void motor_set_speed(int speed, bool reverse);
int motor_get_speed(void);
bool motor_get_reverse(void);
void motor_add_speed(int speed);

#endif