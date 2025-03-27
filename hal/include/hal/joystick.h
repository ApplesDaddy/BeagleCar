/**
 * joystick.h
 *
 * Part of the Hardware Abstraction Layer (HAL)
 *
 * Gives methods to get input from the joystick
 */
#ifndef _JOY_H_
#define _JOY_H_

#include <stdbool.h>
#include <stdint.h>


enum joystick_x_dir
{
    JOY_X_DIR_LEFT,
    JOY_X_DIR_RIGHT,
    JOY_X_DIR_NONE
};
enum joystick_y_dir
{
    JOY_Y_DIR_UP,
    JOY_Y_DIR_DOWN,
    JOY_Y_DIR_NONE
};


void joystick_init(void);
void joystick_cleanup(void);

double joystick_get_x_val(void);             // @return double 1.0 for 100% right, -1.0 for 100% left
double joystick_get_y_val(void);             // @return double 1.0 for 100% up, -1.0 for 100% down
void joystick_get_dir(enum joystick_x_dir *x_dir, enum joystick_y_dir *y_dir);
bool joystick_is_pressed(void);

int joystick_get_angle(void);           // @return [0, 360) where x=1, y=0 is 0
float joystick_get_radius(void);        // @return [0.0, 1.0] where 1 means fully pushed in any direction,
                                        // 0 means not pushed at all
#endif