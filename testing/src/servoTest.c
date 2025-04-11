#include "hal/servo.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"

#include <stdbool.h>

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

int main(void) {
    gpio_init();
    servo_init();
    rot_encoder_init();
    rot_encoder_set_min_max(0, 180);
    rot_encoder_set_step(1);
    rot_encoder_set_counter(0);

    while (true) {
        int val = rot_encoder_get_val();
        servo_set_angle(val);
        printf("Rotary Encoder Value: %d\n", val);
        sleepForMs(100); // Sleep for 100ms to avoid flooding the console
    }

    rot_encoder_cleanup();
    motor_cleanup();
    gpio_cleanup();

    return 0;
}