// source: gpio_statemachine_demo

#include "hal/gpio.h"
#include <stdbool.h>
#include <stdlib.h>

#define INIT_CHECK()                                                                                                   \
    {                                                                                                                  \
        if (!initialized) {                                                                                            \
            gpio_init();                                                                                               \
        }                                                                                                              \
    }

static bool initialized = false;
static char *chip_names[] = {
    "gpiochip0",
    "gpiochip1",
    "gpiochip2",
};
static struct gpiod_chip *open_chips[GPIO_CHIP_COUNT];

void gpio_init(void) {
    // open all chips
    for (int i = 0; i < GPIO_CHIP_COUNT; i++) {
        open_chips[i] = gpiod_chip_open_by_name(chip_names[i]);
        if (!open_chips[i]) {
            perror("GPIO Initializing: Unable to open GPIO chip");
            exit(EXIT_FAILURE);
        }
    }

    initialized = true;
}
void gpio_cleanup(void) {
    // close all chips
    for (int i = 0; i < GPIO_CHIP_COUNT; i++) {
        gpiod_chip_close(open_chips[i]);
    }

    initialized = false;
}

struct GpioLine *gpio_open(enum eGpioChip chip, int pin) {
    INIT_CHECK()

    struct gpiod_chip *chip_obj = open_chips[chip];
    struct gpiod_line *line = gpiod_chip_get_line(chip_obj, pin);
    if (!line) {
        perror("Unable to get GPIO line");
        exit(EXIT_FAILURE);
    }

    return (struct GpioLine *)line;
}

// Source:
// https://people.eng.unimelb.edu.au/pbeuchat/asclinic/software/building_block_gpio_encoder_counting.html
int gpio_wait_line_change(struct GpioLine *line1, struct GpioLine *line2, struct gpiod_line_bulk *bulk_events) {
    INIT_CHECK()

    struct gpiod_line_bulk bulk_wait;
    gpiod_line_bulk_init(&bulk_wait);

    gpiod_line_bulk_add(&bulk_wait, (struct gpiod_line *)line1);
    gpiod_line_bulk_add(&bulk_wait, (struct gpiod_line *)line2);
    gpiod_line_request_bulk_both_edges_events(&bulk_wait, "Event Waiting");

    struct timespec timeout;
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;
    int ret = gpiod_line_event_wait_bulk(&bulk_wait, &timeout, bulk_events);
    if (ret == -1) {
        perror("Error waiting on lines for event waiting");
        exit(EXIT_FAILURE);
    }
    if (ret == 0) {
        return 0;
    }

    int num_events = gpiod_line_bulk_num_lines(bulk_events);
    return num_events;
}

// Source:
// https://people.eng.unimelb.edu.au/pbeuchat/asclinic/software/building_block_gpio_encoder_counting.html
int gpio_wait_line_change_singular(struct GpioLine *line1, struct gpiod_line_bulk *bulk_events) {
    INIT_CHECK()

    struct gpiod_line_bulk bulk_wait;
    gpiod_line_bulk_init(&bulk_wait);

    gpiod_line_bulk_add(&bulk_wait, (struct gpiod_line *)line1);
    gpiod_line_request_bulk_both_edges_events(&bulk_wait, "Event Waiting");

    struct timespec timeout;
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;
    int ret = gpiod_line_event_wait_bulk(&bulk_wait, &timeout, bulk_events);
    if (ret == -1) {
        perror("Error waiting on lines for event waiting");
        exit(EXIT_FAILURE);
    }
    if (ret == 0) {
        return 0;
    }

    int num_events = gpiod_line_bulk_num_lines(bulk_events);
    return num_events;
}

void gpio_close(struct GpioLine *line) {
    INIT_CHECK()
    gpiod_line_release((struct gpiod_line *)line);
}

void gpio_set_input(struct GpioLine *line) {
    INIT_CHECK()
    int ret = gpiod_line_request_input((struct gpiod_line *)line, "");
    if (ret != 0) {
        perror("Failed to request input GPIO line\n");
        exit(EXIT_FAILURE);
    }
}

int gpio_get_val(struct GpioLine *line) {
    INIT_CHECK()

    int val = gpiod_line_get_value((struct gpiod_line *)line);
    return val;
}