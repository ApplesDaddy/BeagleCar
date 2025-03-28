// Low-level GPIO access using gpiod: From class example.
#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdbool.h>
#include <gpiod.h>

// Opaque structure
struct GpioLine;

enum eGpioChips {
    GPIO_CHIP_0,
    GPIO_CHIP_1,
    GPIO_CHIP_2,
    GPIO_NUM_CHIPS      // Count of chips
};

// Must initialize before calling any other functions.
void Gpio_initialize(void);
void Gpio_cleanup(void);


// Opening a pin gives us a "line" that we later work with.
//  chip: such as GPIO_CHIP_0
//  pinNumber: such as 15
struct GpioLine* Gpio_openForEvents(enum eGpioChips chip, int pinNumber);


int Gpio_waitForLineChange_2(
    struct GpioLine* line1, 
    struct GpioLine* line2, 
    struct gpiod_line_bulk *bulkEvents
);

int Gpio_waitForLineChange_1(
    struct GpioLine* line1, 
    struct gpiod_line_bulk *bulkEvents
);

void Gpio_close(struct GpioLine* line);

#endif