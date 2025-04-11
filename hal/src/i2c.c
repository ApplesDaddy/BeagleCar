#include "hal/i2c.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int i2c_init_bus(char *bus, int addr) {
    int file_desc = open(bus, O_RDWR);
    if (file_desc == -1) {
        printf("Unable to open bus %s for r/w\n", bus);
        exit(EXIT_FAILURE);
    }

    int ret = ioctl(file_desc, I2C_SLAVE, addr);
    if (ret == -1) {
        perror("Unable to set I2C device to slave address");
        exit(EXIT_FAILURE);
    }

    return file_desc;
}

void i2c_write_reg16(int file_desc, uint8_t addr, uint16_t val) {
    int tx_size = sizeof(val) + 1;
    uint8_t buf[tx_size];
    buf[0] = addr;
    buf[1] = (val & 0xFF);
    buf[2] = (val & 0xFF00) >> 8;

    // write val
    int bytes_written = write(file_desc, buf, tx_size);
    if (bytes_written != tx_size) {
        perror("Unable to write i2c register");
        exit(EXIT_FAILURE);
    }
}
void i2c_write_reg8(int file_desc, uint8_t addr, uint8_t val) {
    int tx_size = sizeof(val) + 1;
    uint8_t buf[tx_size];
    buf[0] = addr;
    buf[1] = (val & 0xFF);
    buf[2] = (val & 0xFF00) >> 8;

    // write val
    int bytes_written = write(file_desc, buf, tx_size);
    if (bytes_written != tx_size) {
        perror("Unable to write i2c register");
        exit(EXIT_FAILURE);
    }
}

uint16_t i2c_read_reg16(int file_desc, uint8_t addr) {
    // have to write address before reading
    int tx_size = sizeof(addr);
    int bytes_written = write(file_desc, &addr, tx_size);
    if (bytes_written != tx_size) {
        perror("Unable to write i2c register");
        exit(EXIT_FAILURE);
    }

    // read val
    uint16_t val = 0;
    tx_size = sizeof(val);
    int bytes_read = read(file_desc, &val, tx_size);
    if (bytes_read != tx_size) {
        perror("Unable to read i2c register");
        exit(EXIT_FAILURE);
    }

    // swap bytes to correct byte order
    uint16_t swapped = (val & 0xFF00) >> 8;
    swapped += (val & 0x00FF) << 8;
    swapped = swapped >> 4; // trim

    return swapped;
}

uint8_t i2c_read_reg8(int file_desc, uint8_t addr) {
    // have to write address before reading
    int tx_size = sizeof(addr);
    int bytes_written = write(file_desc, &addr, tx_size);
    if (bytes_written != tx_size) {
        perror("Unable to write i2c register");
        exit(EXIT_FAILURE);
    }

    // read val
    uint8_t val = 0;
    tx_size = sizeof(val);
    int bytes_read = read(file_desc, &val, tx_size);
    if (bytes_read != tx_size) {
        perror("Unable to read i2c register");
        exit(EXIT_FAILURE);
    }

    return val;
}
