/*
 * i2s_clock.h
 *
 *  Created on: Aug 8, 2024
 *      Author: Sx107
 */

#ifndef DRV_I2S_CLOCK_H_
#define DRV_I2S_CLOCK_H_

#include <main.h>

void i2sclock_init();
void i2sclock_select(uint32_t freq);

#endif /* DRV_I2S_CLOCK_H_ */
