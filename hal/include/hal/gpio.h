/**
 * gpio.h
 * source: gpio_statemachine_demo
 *
 * Part of the Hardware Abstraction Layer (HAL)
 *
 * Gives methods to communicate with hardware that use GPIO pins
 */

#ifndef _GPIO_H_
#define _GPIO_H_

// #include "gpio.h"
#include <assert.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>

struct GpioLine;
enum eGpioChip { GPIO_CHIP_0 = 0, GPIO_CHIP_1, GPIO_CHIP_2, GPIO_CHIP_COUNT };

void gpio_init(void);
void gpio_cleanup(void);

struct GpioLine *gpio_open(enum eGpioChip chip, int pin);
void gpio_close(struct GpioLine *line);

// @return number of events for 2 lines
int gpio_wait_line_change(struct GpioLine *line1, struct GpioLine *line2, struct gpiod_line_bulk *bulk_events);
int gpio_wait_line_change_singular(struct GpioLine *line1, struct gpiod_line_bulk *bulk_events);

void gpio_set_input(struct GpioLine *line);
int gpio_get_val(struct GpioLine *line);

#endif