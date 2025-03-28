#include "hal/rotaryEncoderMotor.h"
#include "hal/motor.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "hal/gpioMotor.h"


bool done = false;

// Pin config info: GPIO 16 (Rotary Encoder A)
#define GPIO_CHIP_A GPIO_CHIP_2
#define GPIO_LINE_NUMBER_A 7

// Pin config info: GPIO 17 (Rotary Encoder B)
#define GPIO_CHIP_B GPIO_CHIP_2
#define GPIO_LINE_NUMBER_B 8

// Pin config info: GPIO 24 (Rotary Encoder PUSH)
#define GPIO_CHIP_PUSH GPIO_CHIP_0
#define GPIO_LINE_NUMBER_PUSH 10

#define DEBOUNCE_TIME_MS 50

// #region private variables
static bool rotaryEncoder_initialized = false;
struct GpioLine *s_lineA = NULL;
struct GpioLine *s_lineB = NULL;
struct GpioLine *s_lineBtn = NULL;

pthread_t rotaryEncoder_thread_turning;
pthread_t rotaryEncoder_thread_button;

static bool A_risen = false;
static bool B_risen = false;
static bool btn_risen = true;
static long long lastBtnPress = 0;

static long long getTimeInMs(void) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}

static void left(void) { motor_add_speed(5); }

static void right(void) { motor_add_speed(-5); }

static void btn_press(void) { printf("Button was pressed.\n"); }

static void *rotaryEncoder_turn(void *arg) {
    (void)arg;  // Gets rid of the unused variable warning
    while (!done) {
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitForLineChange_2(s_lineA, s_lineB, &bulkEvents);

        // Iterate over the event
        for (int i = 0; i < numEvents; i++) {
            // Get the line handle for this event
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            // Get the number of this line
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Get the line event
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle, &event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }

            // Run the state machine
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

            // Can check with line it is, if you have more than one...
            bool isA = this_line_number == GPIO_LINE_NUMBER_A;

            // Change the value of the one that changed
            if (isRising) {
                if (isA) {
                    A_risen = true;
                    if (B_risen) {
                        right();
                        A_risen = false;
                        B_risen = false;
                    }
                } else {
                    B_risen = true;
                    if (A_risen) {
                        left();
                        A_risen = false;
                        B_risen = false;
                    }
                }
            } else {
                if (isA) {
                    A_risen = false;
                } else {
                    B_risen = false;
                }
            }

// DEBUG INFO ABOUT STATEMACHINE
#if 0
            double time = event.ts.tv_sec + event.ts.tv_nsec / 1000000000.0;
            printf("State machine Debug: i=%d/%d  line num/dir = %d %8s ->  [%f]\n", i, numEvents, this_line_number,
                   isRising ? "RISING" : "falling", time);
#endif
        }
    }
    return NULL;
}

static void *rotaryEncoder_button(void *arg) {
    (void)arg;  // Gets rid of the unused variable warning
    int numEvents;
    while (!done) {
        struct gpiod_line_bulk bulkEvents;
        numEvents = Gpio_waitForLineChange_1(s_lineBtn, &bulkEvents);

        // Iterate over the event
        for (int i = 0; i < numEvents; i++) {
            // Get the line handle for this event
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            // Get the number of this line
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Get the line event
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle, &event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }

            // Run the state machine
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

            // Can check with line it is, if you have more than one...
            bool isBtn = this_line_number == GPIO_LINE_NUMBER_PUSH;

            // Change the value of the one that changed
            if (isBtn) {
                if (isRising) {
                    btn_risen = true;
                    lastBtnPress = getTimeInMs();
                } else {
                    if (btn_risen && getTimeInMs() - lastBtnPress > DEBOUNCE_TIME_MS) {
                        btn_press();
                    }
                    btn_risen = false;
                }
            }

// DEBUG INFO ABOUT STATEMACHINE
#if 0
            double time = event.ts.tv_sec + event.ts.tv_nsec / 1000000000.0;
            printf("State machine Debug: i=%d/%d  line num/dir = %d %8s ->  [%f]\n", i, numEvents, this_line_number,
                   isRising ? "RISING" : "falling", time);
#endif
        }
    }
    return NULL;
}

// #endregion

void rotaryEncoder_init() {
    assert(!rotaryEncoder_initialized);

    s_lineA = Gpio_openForEvents(GPIO_CHIP_A, GPIO_LINE_NUMBER_A);
    s_lineB = Gpio_openForEvents(GPIO_CHIP_B, GPIO_LINE_NUMBER_B);
    s_lineBtn = Gpio_openForEvents(GPIO_CHIP_PUSH, GPIO_LINE_NUMBER_PUSH);

    rotaryEncoder_initialized = true;

    int pthread_create_result = pthread_create(&rotaryEncoder_thread_turning, NULL, rotaryEncoder_turn, NULL);
    if (pthread_create_result != 0) {
        perror("Unable to create thread, in rotaryEncoder_init");
        exit(EXIT_FAILURE);
    }

    pthread_create_result = pthread_create(&rotaryEncoder_thread_button, NULL, rotaryEncoder_button, NULL);
    if (pthread_create_result != 0) {
        perror("Unable to create thread, in rotaryEncoder_init");
        exit(EXIT_FAILURE);
    }
}

void rotaryEncoder_cleanup() {
    assert(rotaryEncoder_initialized);

    int pthread_cancel_result = pthread_cancel(rotaryEncoder_thread_turning);
    if (pthread_cancel_result != 0) {
        perror("Unable to cancel thread, in rotaryEncoder_cleanup");
        exit(EXIT_FAILURE);
    }
    pthread_cancel_result = pthread_cancel(rotaryEncoder_thread_button);
    if (pthread_cancel_result != 0) {
        perror("Unable to cancel thread, in rotaryEncoder_cleanup");
        exit(EXIT_FAILURE);
    }

    int pthread_join_result = pthread_join(rotaryEncoder_thread_turning, NULL);
    if (pthread_join_result != 0) {
        perror("Unable to join thread, in rotaryEncoder_cleanup");
        exit(EXIT_FAILURE);
    }
    pthread_join_result = pthread_join(rotaryEncoder_thread_button, NULL);
    if (pthread_join_result != 0) {
        perror("Unable to join thread, in rotaryEncoder_cleanup");
        exit(EXIT_FAILURE);
    }

    Gpio_close(s_lineA);
    Gpio_close(s_lineB);
    Gpio_close(s_lineBtn);

    rotaryEncoder_initialized = false;
}
