/**
 * i2c.h
 *
 * Part of the Hardware Abstraction Layer (HAL)
 *
 * Gives methods to communicate with hardware that use I2C
 */

#ifndef _I2C_H_
#define _I2C_H_
#include <stdint.h>


/**
 * @brief initializes device
 *
 * @param bus path like /dev/i2c-1
 * @param addr slave address
 * @return int file descriptor
 */
int i2c_init_bus(char* bus, int addr);

void i2c_write_reg16(int file_desc, uint8_t addr, uint16_t val);
void i2c_write_reg8(int file_desc, uint8_t addr, uint8_t val);

uint16_t i2c_read_reg16(int file_desc, uint8_t addr);
uint8_t i2c_read_reg8(int file_desc, uint8_t addr);

#endif