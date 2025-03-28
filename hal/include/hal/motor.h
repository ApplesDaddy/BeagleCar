#ifndef _MOTOR_H_
#define _MOTOR_H_



void motor_init(void);
void motor_cleanup(void);


void motor_set_speed(int speed);
int motor_get_speed(void);


#endif 