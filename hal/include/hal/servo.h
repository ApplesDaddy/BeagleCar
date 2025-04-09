#ifndef _SERVO_H_
#define _SERVO_H_

#include <stdbool.h>

void servo_init(void);
void servo_cleanup(void);


void servo_set_angle(double angle);
double servo_get_angle(void);

#endif 