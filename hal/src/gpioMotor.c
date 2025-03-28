#include "hal/gpioMotor.h"
#include <stdlib.h>
#include <stdio.h>
#include <gpiod.h>
#include <assert.h>
#include <errno.h>

// Relies on the gpiod library.
// Insallation for cross compiling:
//      (host)$ sudo dpkg --add-architecture arm64
//      (host)$ sudo apt update
//      (host)$ sudo apt install libgpdiod-dev:arm64
// GPIO: https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library
// Example: https://github.com/starnight/libgpiod-example/blob/master/libgpiod-input/main.c

// TYPE NOTE:
// Internally cast the 
//    struct GpioLine* 
// to 
//    (struct gpiod_line*)
// so we hide the dependency on gpiod



// static const struct timespec time_out = {0, 100}; 



static bool s_isInitialized = false;

static char* s_chipNames[] = {
    "gpiochip0",
    "gpiochip1",
    "gpiochip2",
};

// Hold open chips
static struct gpiod_chip* s_openGpiodChips[GPIO_NUM_CHIPS];

void Gpio_initialize(void)
{
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
         // Open GPIO chip
        s_openGpiodChips[i] = gpiod_chip_open_by_name(s_chipNames[i]);
        if (!s_openGpiodChips[i]) {
            perror("GPIO Initializing: Unable to open GPIO chip");
            exit(EXIT_FAILURE);
        }
    }
    s_isInitialized = true;
}

void Gpio_cleanup(void)
{
    assert(s_isInitialized);
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
        // Close GPIO chip
        gpiod_chip_close(s_openGpiodChips[i]);
        if (!s_openGpiodChips[i]) {
            perror("GPIO Initializing: Unable to open GPIO chip");
            exit(EXIT_FAILURE);
        }
    }
    s_isInitialized = false;
}

// Opening a pin gives us a "line" that we later work with.
//  chip: such as GPIO_CHIP_0
//  pinNumber: such as 15
struct GpioLine* Gpio_openForEvents(enum eGpioChips chip, int pinNumber)
{
    assert(s_isInitialized);
    struct gpiod_chip* gpiodChip = s_openGpiodChips[chip];
    struct gpiod_line* line = gpiod_chip_get_line(gpiodChip, pinNumber);
    if (!line) {
        perror("Unable to get GPIO line");
        exit(EXIT_FAILURE);
    }

    return (struct GpioLine*) line;  
}

void Gpio_close(struct GpioLine* line)
{
    assert(s_isInitialized);
    gpiod_line_release((struct gpiod_line*) line);
}



// Returns the number of events
int Gpio_waitForLineChange_2(
    struct GpioLine* line1, 
    struct GpioLine* line2, 
    struct gpiod_line_bulk *bulkEvents
) {
    assert(s_isInitialized);

    // Source: https://people.eng.unimelb.edu.au/pbeuchat/asclinic/software/building_block_gpio_encoder_counting.html   
    struct gpiod_line_bulk bulkWait;
    gpiod_line_bulk_init(&bulkWait);
    
    // TODO: Add more lines if needed
    gpiod_line_bulk_add(&bulkWait, (struct gpiod_line*)line1);
    gpiod_line_bulk_add(&bulkWait, (struct gpiod_line*)line2);
    
    gpiod_line_request_bulk_both_edges_events(&bulkWait, "Event Waiting");

    int result = gpiod_line_event_wait_bulk(&bulkWait, NULL, bulkEvents);
    if ( result == -1) {
        perror("Error waiting on lines for event waiting");
        printf("Last error code: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    int numEvents = gpiod_line_bulk_num_lines(bulkEvents);
    return numEvents;
}

int Gpio_waitForLineChange_1(
    struct GpioLine* line1, 
    struct gpiod_line_bulk *bulkEvents
) {
    assert(s_isInitialized);

    // Source: https://people.eng.unimelb.edu.au/pbeuchat/asclinic/software/building_block_gpio_encoder_counting.html   
    struct gpiod_line_bulk bulkWait;
    gpiod_line_bulk_init(&bulkWait);
    
    // TODO: Add more lines if needed
    gpiod_line_bulk_add(&bulkWait, (struct gpiod_line*)line1);

    gpiod_line_request_bulk_both_edges_events(&bulkWait, "Event Waiting");

    int result = gpiod_line_event_wait_bulk(&bulkWait, NULL, bulkEvents);
    if ( result == -1) {
        perror("Error waiting on lines for event waiting");
        printf("Last error code: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    int numEvents = gpiod_line_bulk_num_lines(bulkEvents);
    return numEvents;
}
