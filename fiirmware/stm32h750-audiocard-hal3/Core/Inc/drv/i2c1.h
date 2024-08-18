// This code sucks.
// I have NO idea how to properly handle I2C errors.
// And literally hours of googling did not get me anywhere.
// Just use HAL. Even stable LL examples are unavailable.
// Please don't use this I2C library ever, just use HAL
// Author: sx107

#include "main.h"

#include "i2c_common.h"

#ifndef I2C1_H_
#define I2C1_H_

extern volatile uint32_t _i2c1_error;

#define I2C1_TMO_VAL 600000

uint8_t i2c1_nack();
void i2c1_nack_clear();


void i2c1_init();
void i2c1_deinit();
void i2c1_reset();

// Error handling
void i2c1_ER_IRQHandler(void);
void i2c1_reset_error();
uint32_t i2c1_get_error();
uint32_t i2c1_handle_error();
uint32_t i2c1_waitBit(uint32_t bit);

// Manual functions
uint32_t i2c1_transmission_start(uint8_t addr, _i2c_wrmode mode, uint16_t busy_timeout, uint8_t nbytes);
uint32_t i2c1_transmission_end(_i2c_wrmode mode);
uint32_t i2c1_transmission_write8(uint8_t byte);
uint32_t i2c1_transmission_read8(volatile uint8_t* byte);
uint8_t i2c1_transmission_busy();
void i2c1_transmission_ack_disable();
uint8_t i2c1_dr();


// Main write functions
uint32_t i2c1_write_blocking(uint8_t addr, uint16_t sz, const volatile uint8_t* buf, uint16_t busy_timeout);

// Main read functions
uint32_t i2c1_read_blocking(uint8_t addr, uint16_t sz, volatile uint8_t* buf, uint16_t busy_timeout);

// Other read-write functions
uint32_t i2c1_write8(uint8_t addr, uint8_t byte);
uint32_t i2c1_write16(uint8_t addr, uint16_t byte);
uint32_t i2c1_write32(uint8_t addr, uint32_t byte);

uint32_t i2c1_read8(uint8_t addr);
uint32_t i2c1_read16(uint8_t addr);
uint32_t i2c1_read32(uint8_t addr);

uint32_t i2c1_readRegister(uint8_t addr, uint8_t reg, uint16_t sz, uint8_t* buf);
uint32_t i2c1_readRegister8(uint8_t addr, uint8_t reg);
uint32_t i2c1_writeRegister8(uint8_t addr, uint8_t reg, uint8_t val);

#endif

