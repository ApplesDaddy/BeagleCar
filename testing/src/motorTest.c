#include "hal/motor.h"
#include "hal/gpioMotor.h"
#include "hal/rotaryEncoderMotor.h"

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
  Gpio_initialize();
  motor_init();
  rotaryEncoder_init();

  while (!done) {
    // Wait for the rotary encoder to be turned
    sleepForMs(100); // Sleep for 100ms to avoid busy waiting
  }

  rotaryEncoder_cleanup();
  motor_cleanup();
  Gpio_cleanup();

  return 0;
}