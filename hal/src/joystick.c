#include "hal/joystick.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "util/common_funcs.h"


#include <linux/i2c-dev.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


// I2C constants
#define I2C_DEVICE_ADDR 0x48
#define I2C_BUS "/dev/i2c-1"
#define REG_ON_CONFIG 0x01
#define I2C_Y_CHANNEL 0x83C2
#define I2C_X_CHANNEL 0x83D2

// GPIO constants
#define GPIO_CHIP GPIO_CHIP_2
#define GPIO_LINE_NUMBER 15 // GPIO5

// other constants
#define THRESHOLD 200
#define DIAG_ADJUST_SCALE 1.2

// global vars
static uint16_t neutral_y = 828;
static uint16_t up_max = 5;
static uint16_t down_max = 1620;

static uint16_t neutral_x = 836;
static uint16_t right_max = 1614;
static uint16_t left_max = 17;

static int file_desc = -1;
static struct GpioLine *line = NULL;

enum channel { CHANNEL_X, CHANNEL_Y, CHANNEL_NONE };
static enum channel enabled_channel = CHANNEL_NONE;

static bool initialized = false;

// ==================================== private helper functions ===================================
static inline void enable_channel(enum channel new_channel) {
    if (new_channel == enabled_channel) {
        return;
    }

    enabled_channel = new_channel;
    if (new_channel == CHANNEL_NONE) {
        return;
    }

    i2c_write_reg16(file_desc, REG_ON_CONFIG, new_channel == CHANNEL_X ? I2C_X_CHANNEL : I2C_Y_CHANNEL);
    sleep_ms(50); // need delay before reading
}

// negative if between min and mid, else positive
static inline double val_to_percent(uint16_t val, uint16_t mid, uint16_t max, uint16_t min) {
    double ret_val = 0; // neutral
    // -- mid ----------- val ----------- max
    if (val > mid + THRESHOLD) {
        uint16_t range_start = mid + THRESHOLD;
        ret_val = (double)(val - range_start) / (double)(max - range_start);
    }
    // -- min ----------- val ----------- mid --
    else if (val < mid - THRESHOLD) {
        uint16_t range_start = min;
        uint16_t range_end = mid - THRESHOLD;
        ret_val = -1.0 * (1.0 - (double)(val - range_start) / (double)(range_end - range_start));
    }
    return ret_val;
}

// ======================================= public functions ========================================
void joystick_init(void) {
    file_desc = i2c_init_bus(I2C_BUS, I2C_DEVICE_ADDR);
    line = gpio_open(GPIO_CHIP, GPIO_LINE_NUMBER);
    gpio_set_input(line);
    initialized = true;
}

void joystick_cleanup(void) {
    close(file_desc);
    file_desc = -1;
    gpio_close(line);
    initialized = false;
}

double joystick_get_x_val(void) {
    if (!initialized) {
        joystick_init();
    }

    enable_channel(CHANNEL_X);
    uint16_t val = i2c_read_reg16(file_desc, 0x00);

    // adjust threshold if needed
    if (val > right_max) {
        right_max = val;
    }
    if (val < left_max) {
        left_max = val;
    }

    // --> greater values
    // -- left_max ----------- val ----------- neutral_x ----------- val ----------- right_max
    return val_to_percent(val, neutral_x, right_max, left_max);
}

double joystick_get_y_val(void) {
    if (!initialized) {
        joystick_init();
    }

    enable_channel(CHANNEL_Y);
    uint16_t val = i2c_read_reg16(file_desc, 0x00);

    // adjust threshold if needed
    if (val > down_max) {
        down_max = val;
    }
    if (val < up_max) {
        up_max = val;
    }

    // --> greater values
    // -- up_max ----------- val ----------- neutral_y ----------- val ----------- down_max
    return -1.0 * val_to_percent(val, neutral_y, down_max, up_max);
}

void joystick_get_dir(enum joystick_x_dir *x_dir, enum joystick_y_dir *y_dir) {
    if (!initialized) {
        joystick_init();
    }

    double x_percent = joystick_get_x_val();
    double y_percent = joystick_get_y_val();

    if (!x_percent) {
        *x_dir = JOY_X_DIR_NONE;
    } else {
        *x_dir = x_percent > 0 ? JOY_X_DIR_RIGHT : JOY_X_DIR_LEFT;
    }

    if (!y_percent) {
        *y_dir = JOY_Y_DIR_NONE;
    } else {
        *y_dir = y_percent > 0 ? JOY_Y_DIR_UP : JOY_Y_DIR_DOWN;
    }
}

bool joystick_is_pressed() {
    return !gpio_get_val(line); // 0 when pressed, else 1
}

int joystick_get_angle(void) {
    if (!initialized) {
        joystick_init();
    }

    double x_percent = joystick_get_x_val();
    double y_percent = joystick_get_y_val();

    if (x_percent == 0) {
        return y_percent >= 0 ? 90 : 270;
    }
    if (y_percent == 0) {
        return x_percent >= 0 ? 0 : 180;
    }

    int angle = atan2(y_percent, x_percent) * (180 / M_PI);

    // convert to [0, 360)
    if (angle < 0) {
        angle = 360 + angle;
    }

    return angle;
}
float joystick_get_radius(void) {
    if (!initialized) {
        joystick_init();
    }

    double x_percent = joystick_get_x_val();
    double y_percent = joystick_get_y_val();

    float radius = sqrt((pow(x_percent, 2)) + (pow(y_percent, 2)));

    // to account for diagonals, add ~0.2 (since maximum for 45 degrees is ~0.7)
    radius *= DIAG_ADJUST_SCALE;
    radius = radius > 1 ? 1.0 : radius;
    return radius;
}
