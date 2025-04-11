#include "hal/motor.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"

#include <stdbool.h>

// static void sleepForMs(long long delayInMs) {
//   // Sleep for the specified number of milliseconds; From assignment 1
//   const long long NS_PER_MS = 1000 * 1000;
//   const long long NS_PER_SECOND = 1000000000;
//   long long delayNs = delayInMs * NS_PER_MS;
//   int seconds = delayNs / NS_PER_SECOND;
//   int nanoseconds = delayNs % NS_PER_SECOND;
//   struct timespec reqDelay = {seconds, nanoseconds};
//   nanosleep(&reqDelay, (struct timespec *)NULL);
// }

int main(void) {
    gpio_init();
    motor_init();
    rot_encoder_init();
    rot_encoder_set_min_max(-100, 100);
    rot_encoder_set_step(10);
    rot_encoder_set_counter(0);

    while (true) {
        int val = rot_encoder_get_val();
        if (val < 100) {
            val = abs(val);
            motor_set_speed(val, true);
        } else {
            motor_set_speed(val, false);
        }

        printf("Rotary Encoder Value: %d\n", val);
    }

    rot_encoder_cleanup();
    motor_cleanup();
    gpio_cleanup();

    return 0;
}