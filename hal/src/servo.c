#include "hal/servo.h"
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
#define SERVO_ENABLE_FILE "/dev/hat/pwm/GPIO14/enable"
#define SERVO_DUTY_CYCLE_FILE "/dev/hat/pwm/GPIO14/duty_cycle"
#define SERVO_PERIOD_FILE "/dev/hat/pwm/GPIO14/period"

#define LEFT 1000 // 1000us
#define RIGHT 2000 // 2000us

static double current_angle = 90.0; // 0 is left and 180 is right

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
  assert(hz >= 3); // 3 hz is the smallest hz value the pwm can handle
  return (1 * 1000 * 1000 * 1000) / hz; // 1 second in nanoseconds
}

static void set_duty_cycle(long long duty_cycle) {
  FILE *duty_file = fopen(SERVO_DUTY_CYCLE_FILE, "r");
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

  duty_file = fopen(SERVO_DUTY_CYCLE_FILE, "w");
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
  FILE *period_file = fopen(SERVO_PERIOD_FILE, "w");
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

static void enable_servo() {
  assert(initialized);
  enabled = true;

  // Check the value in the enable file
  FILE *enable_file = fopen(SERVO_ENABLE_FILE, "r");
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
    // The servo is already on
    return;
  }

  // Start the Servo
  enable_file = fopen(SERVO_ENABLE_FILE, "w");
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

static void disable_servo() {
  // This function is called when the servo turns off
  assert(initialized);
  enabled = false;

  // Check the value in the enable file
  FILE *enable_file = fopen(SERVO_ENABLE_FILE, "r");
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
    // The servo is already off
    return;
  }

  // Turn off the servo
  enable_file = fopen(SERVO_ENABLE_FILE, "w");
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

static long long angle_to_duty(int angle){
  // Convert the angle to a duty cycle
  // The servo expects a duty cycle between 1000us and 2000us
  // The angle is between 0 and 180 degrees
  assert(angle >= 0 && angle <= 180);
  return (long long) LEFT + (RIGHT - LEFT) * angle / 180;
}



static void start_servo() {
  assert(initialized);
  assert(!enabled); // The servo should be off

  // Set the correct duty and period cycle
  // From the radio module it seems the servo expects a 71hz signal
  set_period(hz_to_period(50));         // Set the period to 71hz
  set_duty_cycle(us_to_ns(angle_to_duty(90))); // Set the duty cycle to neutral

  // Enable the servo
  enable_servo();
}

// #endregion

void servo_init(void) {
  assert(!initialized);
  initialized = true;
  start_servo();
}
void servo_cleanup(void) {
  assert(initialized);
  disable_servo();
  initialized = false;
}

void servo_set_angle(double angle) {
  assert(initialized);
  assert(enabled); // The servo should be on
  assert(angle >= 0 && angle <= 180);

  // Set the duty cycle to the new angle
  set_duty_cycle(us_to_ns(angle_to_duty(angle)));
  current_angle = angle;
}

double servo_get_angle(void) {
  assert(initialized);
  assert(enabled); // The servo should be on
  return current_angle;
}