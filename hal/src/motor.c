#include "hal/motor.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static bool initialized = false;
static bool enabled = false;

// We are using gpio pin 15 for the pwm output which is attached to uart pin
// on the right side when the uart pin group is top left when looking at the
// board from the top.
#define MOTOR_ENABLE_FILE "/dev/hat/pwm/GPIO15/enable"
#define MOTOR_DUTY_CYCLE_FILE "/dev/hat/pwm/GPIO15/duty_cycle"
#define MOTOR_PERIOD_FILE "/dev/hat/pwm/GPIO15/period"

#define NEUTRAL_US 1549 // 1549us
#define FORWARD_US 1789 // 1789us
#define BACKWARD_US 1296 // 1296us

static int current_speed = 0;
static bool current_reverse = false;

// #region Helper functions
static void sleepForMs(long long delayInMs) {
    // Sleep for the specified number of milliseconds; From assignment 1
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

static long long us_to_ns(int us) { return us * 1000; }

static int hz_to_period(int hz) {
    assert(hz >= 3);                      // 3 hz is the smallest hz value the pwm can handle
    return (1 * 1000 * 1000 * 1000) / hz; // 1 second in nanoseconds
}

static void set_duty_cycle(long long duty_cycle) {
    FILE *duty_file = fopen(MOTOR_DUTY_CYCLE_FILE, "r");
    if (duty_file == NULL) {
        perror("Error opening duty file for reading");
        exit(EXIT_FAILURE);
    }

    long long current_duty_cycle;
    int char_read = fscanf(duty_file, "%lld", &current_duty_cycle);
    if (char_read <= 0) {
        perror("Error reading from reading duty file");
        exit(EXIT_FAILURE);
    }
    int fileclose = fclose(duty_file);
    if (fileclose != 0) {
        perror("Error closing reading duty file");
        exit(EXIT_FAILURE);
    }

    // Early return because it will fail if we try to set the duty cycle to the
    // same value
    if (current_duty_cycle == duty_cycle) {
        return;
    }

    duty_file = fopen(MOTOR_DUTY_CYCLE_FILE, "w");
    if (duty_file == NULL) {
        perror("Error opening duty file");
        exit(EXIT_FAILURE);
    }
    int char_printed = fprintf(duty_file, "%lld", duty_cycle);
    if (char_printed <= 0) {
        perror("Error writing to duty file");
        exit(EXIT_FAILURE);
    }
    fileclose = fclose(duty_file);
    if (fileclose != 0) {
        perror("Error closing duty file");
        exit(EXIT_FAILURE);
    }
}

static void set_period(long long period) {
    // A call to this function requires a call to set_duty_cycle afterwards

    set_duty_cycle(0); // Since the duty cycle needs to be less than the period
    FILE *period_file = fopen(MOTOR_PERIOD_FILE, "w");
    if (period_file == NULL) {
        perror("Error opening period file");
        exit(EXIT_FAILURE);
    }
    int char_printed = fprintf(period_file, "%lld", period);
    if (char_printed <= 0) {
        perror("Error writing to period file");
        exit(EXIT_FAILURE);
    }
    int fileclose = fclose(period_file);
    if (fileclose != 0) {
        perror("Error closing period file");
        exit(EXIT_FAILURE);
    }
}

static void enable_motor() {
    assert(initialized);
    enabled = true;

    // Check the value in the enable file
    FILE *enable_file = fopen(MOTOR_ENABLE_FILE, "r");
    if (enable_file == NULL) {
        perror("Error opening enable file");
        exit(EXIT_FAILURE);
    }
    int on;
    int char_read = fscanf(enable_file, "%d", &on);
    if (char_read <= 0) {
        perror("Error readding from enable file");
        exit(EXIT_FAILURE);
    }
    int fileclose = fclose(enable_file);
    if (fileclose != 0) {
        perror("Error closing enable file");
        exit(EXIT_FAILURE);
    }

    if (on == 1) {
        // The Motor is already on
        return;
    }

    // Start the Motor
    enable_file = fopen(MOTOR_ENABLE_FILE, "w");
    if (enable_file == NULL) {
        perror("Error opening enable file");
        exit(EXIT_FAILURE);
    }
    int char_printed = fprintf(enable_file, "1");
    if (char_printed <= 0) {
        perror("Error writing to enable file");
        exit(EXIT_FAILURE);
    }
    fileclose = fclose(enable_file);
    if (fileclose != 0) {
        perror("Error closing enable file");
        exit(EXIT_FAILURE);
    }
}

static void disable_motor() {
    // This function is called when the motor turns off
    assert(initialized);
    enabled = false;

    // Check the vaalue in the enable file
    FILE *enable_file = fopen(MOTOR_ENABLE_FILE, "r");
    if (enable_file == NULL) {
        perror("Error opening enable file");
        exit(EXIT_FAILURE);
    }
    int on = 0;
    int char_read = fscanf(enable_file, "%d", &on);
    if (char_read <= 0) {
        perror("Error readding from enable file");
        exit(EXIT_FAILURE);
    }
    int fileclose = fclose(enable_file);
    if (fileclose != 0) {
        perror("Error closing enable file");
        exit(EXIT_FAILURE);
    }

    if (on == 0) {
        // The motor is already off
        return;
    }

    // Turn off the LED emitter
    enable_file = fopen(MOTOR_ENABLE_FILE, "w");
    if (enable_file == NULL) {
        perror("Error opening enable file");
        exit(EXIT_FAILURE);
    }
    int char_printed = fprintf(enable_file, "0");
    if (char_printed <= 0) {
        perror("Error writing to enable file");
        exit(EXIT_FAILURE);
    }
    fileclose = fclose(enable_file);
    if (fileclose != 0) {
        perror("Error closing enable file");
        exit(EXIT_FAILURE);
    }
}

static void start_motor() {
    assert(initialized);
    assert(!enabled); // The motor should be off

    // Set the correct duty and period cycle
    // From the radio module it seems the motor expects a 71hz signal
    set_period(hz_to_period(71));         // Set the period to 71hz
    set_duty_cycle(us_to_ns(NEUTRAL_US)); // Set the duty cycle to neutral

    // Enable the motor
    enable_motor();
}

// #endregion

void motor_init(void) {
    assert(!initialized);
    initialized = true;
    start_motor();
}
void motor_cleanup(void) {
    assert(initialized);
    disable_motor();
    initialized = false;
}

void motor_set_speed(int speed, bool reverse) {
    assert(initialized);
    assert(speed >= 0 && speed <= 100);

    assert(enabled); // The motor should be on
    if (current_speed == speed) {
        return; // No need to change the speed
    }

    // Calculate the duty cycle for the speed
    if (reverse != current_reverse) {
        // Need to change direction of the motor
        // Go to neutral first
        set_duty_cycle(us_to_ns(NEUTRAL_US));
        sleepForMs(15); // 71hz has a period of ~14ms so we wait atleast 1 period
        current_reverse = reverse;
    }

    long long duty_cycle = 0;
    if (reverse) {
        // Move in reverse
        printf("reverse\n");
        duty_cycle = us_to_ns(NEUTRAL_US - speed * (NEUTRAL_US - BACKWARD_US) / 100);
    } else {
        // Move forward
        printf("forward\n");
        duty_cycle = us_to_ns(NEUTRAL_US + speed * (FORWARD_US - NEUTRAL_US) / 100);
    }

    assert(duty_cycle != 0);
    set_duty_cycle(duty_cycle);
    current_speed = speed;
}

void motor_add_speed(int speed) {
    assert(initialized);

    // Compute speed
    int new_speed = current_speed + speed;
    if (new_speed > 100) {
        new_speed = 100;
    } else if (new_speed < 0) {
        new_speed = 0;
    }

    // Validate the new speed
    assert(new_speed >= 0 && new_speed <= 100);

    // Set the new speed
    motor_set_speed(new_speed, current_reverse);
}

int motor_get_speed(void) { return current_speed; }
bool motor_get_reverse(void) { return current_reverse; }
